/**
 * @file kdtree/kdsahbuilder.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <kdtree/kdsahbuilder.h>

#include <algorithm>
#include <iostream> // TODO

KDSAHBuilder::KDSAHBuilder(KDTree & tree, util::vector<Triangle, 16> & triangles, float k_traversal, float k_intersect)
    : KDBuilder(tree, triangles),
      k_traversal(k_traversal),
      k_intersect(k_intersect)
{
}

KDSAHBuilder::~KDSAHBuilder() {
}

bool compareEvent(const SAHEvent & e1, const SAHEvent & e2) {
    // Sort by plane location then event type
    if (e1.dist < e2.dist)
        return true;
    else if (e1.dist == e2.dist)
        return e1.flag < e2.flag;
    else
        return false;
}

bool KDSAHBuilder::shouldSplitNode(
    KDSAHBuilderThreadCtx            & ctx,
    const AABB                       & bounds,
    const util::vector<Triangle, 16> & triangles,
    int                                depth,
    float                            & split,
    int                              & dir,
    enum KDBuilderPlanarMode         & planarMode)
{
	// TODO: Look around for better heuristics. Empty leaves?
	// TODO: Tune depth vs. node size

    // Assumptions:
    //     - There is at least one triangle in the node
    //     - Each triangle at least partially overlaps the node, possible just touching (<= vs. <)
    //
    // This algorithm takes O(N*log^2(N)), and is discussed in the following paper:
    // http://dcgi.felk.cvut.cz/home/havran/ARTICLES/ingo06rtKdtree.pdf
    //
    // TODO:
    //     - Scale cost for empty nodes
    //     - Don't need to use a vector--an array would suffice and possibly be faster
    //     - Convert this to vector instructions, spread branches across multiple threads

    if (triangles.size() == 0)
        return false;

    // The algorithm is implemented by generating and processing a sequence of "events" along a
    // fixed axis (X, Y, or Z). "Begin" events mark the beginning of a triangle. "End" events mark
    // the end of a triangle. Triangles may start and stop at the same point, in which case a
    // "Plane" event is generated and handled specially. More than one triangle may start or stop
    // at the same point, so multiple events must be processed at each sweep plane location.

    // Make sure the builder's event list is big enough. We can use malloc here to avoid constructor
    // overhead. Allocate an upper bound that assumes each triangle generates both a begin and end
    // event.
    util::vector<SAHEvent, 8> & events = ctx.events;
    events.reserve(triangles.size() * 2);

    // We want to find the plane which minimizes the "surface area heuristic"
    int min_dir = -1;
    float min_dist = 0.0f;
    float min_cost = std::numeric_limits<float>::infinity();
    enum KDBuilderPlanarMode min_planarMode = PLANAR_LEFT;

    // Surface area of parent voxel
    float sa_v = bounds.surfaceArea();

    // Try each major axis in turn
    for (int axis = 0; axis < 3; ++axis) {
        // Min and max of parent node along this axis
        float min = bounds.min[axis];
        float max = bounds.max[axis];

        // Insert start/stop/planar locations of each triangle, clamped to the bounds of the parent
        // box. We assume we won't see triangles fully outside the parent node.
        for (auto tri : triangles) {
            // Triangle bounding box
            float tri_min = fminf(fminf(tri.v[0].position[axis], tri.v[1].position[axis]), tri.v[2].position[axis]);
            float tri_max = fmaxf(fmaxf(tri.v[0].position[axis], tri.v[1].position[axis]), tri.v[2].position[axis]);

#if 0
            // Clip triangle bounding box to voxel bounding box
            // TODO: This generates weirdness for triangles outside the box
            tri_min = fmax(min, tri_min);
            tri_max = fmin(max, tri_max);
#endif

            SAHEvent event;

            // If the triangle min is the same as the triangle max the triangle is planar
            if (tri_min == tri_max) {
                event.flag = SAH_PLANAR;
                event.dist = tri_min;
                events.push_back_inbounds(event);
            }
            // Otherwise, generate begin and end events
            else {
                event.flag = SAH_BEGIN;
                event.dist = tri_min;
                events.push_back_inbounds(event);

                event.flag = SAH_END;
                event.dist = tri_max;
                events.push_back_inbounds(event);
            }
        }

        // Sort events by type and then by position so that we can just sweep from the minimum
        // point to the maximum point instead of repeatedly partitioning. Sweeping along the vector
        // in order will be cache friendly as well.
        std::sort(events.begin(), events.end(), compareEvent);

        // Number of triangles entirely to the left and right of the sweep plane
        int count_left = 0;
        int count_right = (int)triangles.size(); // TODO: handle overflow

        int event_idx = 0;

        // Sweep along axis processing events
        while (event_idx < events.size()) {
            auto & event = events[event_idx];
            float dist = event.dist;

            // Triangles touching the sweep plane: starting, lying in (parallel), and ending
            int count_starting = 0;
            int count_ending = 0;
            int count_planar = 0;

            // Handle all events at this plane position
            while (true) {
                // SAH_* have known values, so the compiler is able to turn this into
                // bit masks, etc. to optimize away the switch here. The events are also
                // sorted, so the branch predictor would perform OK either way.
                switch(event.flag) {
                case SAH_END:
                    ++count_ending;
                    break;
                case SAH_PLANAR:
                    ++count_planar;
                    break;
                case SAH_BEGIN:
                    ++count_starting;
                    break;
                }

                if (++event_idx < events.size()) {
                    event = events[event_idx]; // TODO: past end by 1?

                    if (event.dist != dist)
                        break;
                }
                else
                    break;
            }

            // Move triangles that lie in or start on this plane out of the "right" set
            count_right -= count_planar;
            count_right -= count_ending;

            // Split the bounding volume
            AABB v1, v2;
            bounds.split(dist, axis, v1, v2);

            // Compute surface areas
            float sa_l = v1.surfaceArea();
            float sa_r = v2.surfaceArea();

			float costL = std::numeric_limits<float>::infinity();
			float costR = std::numeric_limits<float>::infinity();

			// Don't use this split if either child volume would have zero surface area, because the probability of hitting
			// those nodes would be 0
			if (sa_l != 0.0f && sa_r != 0.0f) {
				// Try placing planar triangles in the left and right sets and choose the lower cost
				costL = k_traversal + k_intersect * (sa_l / sa_v * (count_left + count_planar) + sa_r / sa_v * count_right);

#if 1
                if (count_left + count_planar == 0)
                    costL *= 0.8f;
#endif

				costR = k_traversal + k_intersect * (sa_l / sa_v * count_left + sa_r / sa_v * (count_right + count_planar));

#if 1
                if (count_right + count_planar == 0)
                    costR *= 0.8f;
#endif
			}

            enum KDBuilderPlanarMode minMode;
            float cost;

            if (costL < costR) {
                minMode = PLANAR_LEFT;
                cost = costL;
            }
            else {
                minMode = PLANAR_RIGHT;
                cost = costR;
            }

            // Update minimum heuristic
            if (cost < min_cost) {
                min_cost = cost;
                min_dist = dist;
                min_dir = axis;
                min_planarMode = minMode;
            }

            // Move triangles that started or lie on this plane into the "left" set for the next
            // pass.
            count_left += count_starting;
            count_left += count_planar;
        }

        events.clear();
    }

#if 0
    // If we didn't find a split plane, don't split. TODO.
    if (min_dir == -1)
        return false;
#endif
    assert(min_dir != -1);

    // If the minimum split cost is greater than the cost of not splitting, don't split
    if (min_cost > k_intersect * triangles.size())
        return false;

    // Otherwise, use this split
    split = min_dist;
    dir = min_dir;
    planarMode = min_planarMode;

    return true;
}
