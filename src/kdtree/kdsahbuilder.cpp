/**
 * @file kdsahbuilder.cpp
 *
 * @author Sean James
 */

#include <kdtree/kdsahbuilder.h>
#include <algorithm>
#include <math/macro.h>
#include <iostream> // TODO

KDSAHBuilder::KDSAHBuilder() {
}

KDSAHBuilder::~KDSAHBuilder() {
}

void *KDSAHBuilder::prepareWorkerThread(int threadIdx) {
	KDSAHBuilderThreadCtx *ctx = new KDSAHBuilderThreadCtx();

	ctx->events = nullptr;
	ctx->event_capacity = 0;

	return ctx;
}

void KDSAHBuilder::destroyWorkerThread(void *threadCtx) {
	KDSAHBuilderThreadCtx *ctx = (KDSAHBuilderThreadCtx *)threadCtx;

	if (ctx->events)
		free(ctx->events);

	delete ctx;
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

bool KDSAHBuilder::splitNode(
	void *threadCtx,
    const AABB & bounds,
    const std::vector<Triangle *> & triangles,
    int depth,
    int & dir,
    float & split,
    enum PlanarMode & planarMode)
{
	KDSAHBuilderThreadCtx *ctx = (KDSAHBuilderThreadCtx *)threadCtx;

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
    //     - Clip triangles to bounding box. "perfect splits"

    if (triangles.size() == 0)
        return false;

    // The algorithm is implemented by generating and processing a sequence of "events" along a
    // fixed axis (X, Y, or Z). "Begin" events mark the beginning of a triangle. "End" events mark
    // the end of a triangle. Triangles may start and stop at the same point, in which case a
    // "Plane" event is generated and handled specially. More than one triangle may start or stop
    // at the same point, so multiple events must be processed at each sweep plane location.

    // Make sure the builder's event list is big enough. We can use malloc here to avoid constructor
    // overhead. Allocate an upper bound that assumes each triangle generates both a begin and end
    // event. TODO null check. TODO reuse a single list or something.
    if (ctx->event_capacity < triangles.size() * 2) {
        // TODO: It seems like it's nice to allocate powers of two, but it might not be necessary.
        // TODO: We're not going to ever resize after the first node. There should be a
        // prepareBuilde(number_of_triangles) or something to avoid this check.

        while (ctx->event_capacity < triangles.size() * 2) {
			if (ctx->event_capacity == 0)
				ctx->event_capacity = 128; // TODO arbitrary constant, branch
            else
				ctx->event_capacity *= 2;
        }

        if (ctx->events)
            free(ctx->events);

		ctx->events = (SAHEvent *)malloc(sizeof(SAHEvent) * ctx->event_capacity);
		assert(ctx->events != nullptr); // TODO
    }

	SAHEvent *events = ctx->events;

    int num_events = 0;

    // We want to find the plane which minimizes the "surface area heuristic"
    int             min_dir        = -1;                                     // Split direction
    float           min_dist       = 0.0f;                                   // Split location
	float           min_cost       = std::numeric_limits<float>::infinity(); // Split cost
    enum PlanarMode min_planarMode = PLANAR_LEFT;                            // How to handle planar triangles

    // Heuristic constants
    static const float k_t = 1.0f; // Cost of traversing a KD node
    static const float k_i = 1.0f; // Cost of intersecting a triangle

    // Surface area of parent voxel
    float sa_v = bounds.surfaceArea();

    // Try each major axis in turn
    for (int axis = 0; axis < 3; ++axis) {
        num_events = 0;

        // Min and max of parent node along this axis
		float min = bounds.min.v[axis];
		float max = bounds.max.v[axis];

        // Insert start/stop/planar locations of each triangle, clamped to the bounds of the parent
        // box. We assume we won't see triangles fully outside the parent node.
        for (auto tri : triangles) {
            // Triangle bounding box
			float tri_min = tri->bbox.min.v[axis];
			float tri_max = tri->bbox.max.v[axis];

            // Clip triangle bounding box to voxel bounding box
            tri_min = tri_min < min ? min : tri_min;
            tri_max = tri_max > max ? max : tri_max;

            SAHEvent *event = &events[num_events];

            // If the triangle min is the same as the triangle max the triangle is planar
            if (tri_min == tri_max) {
                event->flag = SAH_PLANAR;
                event->dist = tri_min;
                num_events++;
            }
            // Otherwise, generate begin and end events
            else {
                event->flag = SAH_END;
                event->dist = tri_max;
                num_events++;

                event = &events[num_events];
                event->flag = SAH_BEGIN;
                event->dist = tri_min;
                num_events++;
            }
        }

        // Sort events by type and then by position so that we can just sweep from the minimum
        // point to the maximum point instead of repeatedly partitioning. Sweeping along the vector
        // in order will be cache friendly as well.
        std::sort(events, events + num_events, compareEvent);

        // Number of triangles entirely to the left and right of the sweep plane
        int count_left = 0;
        int count_right = (int)triangles.size(); // TODO: handle overflow

        int event_idx = 0;

        // Sweep along axis processing events
        while (event_idx < num_events) {
            auto & event = events[event_idx];
            float dist = event.dist;

            // Triangles touching the sweep plane: starting, lying in (parallel), and ending
            int count_starting = 0;
            int count_ending = 0;
            int count_planar = 0;

            // Handle all events at this plane position
            while (event_idx < num_events && event.dist == dist) {
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

                event = events[++event_idx];
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

            // Try placing planar triangles in the left and right sets and choose the lower cost
            float costL = k_t + k_i * (sa_l / sa_v * (count_left + count_planar) + sa_r / sa_v * count_right);
            float costR = k_t + k_i * (sa_l / sa_v * count_left + sa_r / sa_v * (count_right + count_planar));

            enum PlanarMode minMode;
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
    }

    // If we didn't find a split plane, don't split. TODO.
    if (min_dir == -1)
        return false;

    // If the minimum split cost is greater than the cost of not splitting, don't split
    if (min_cost > k_i * triangles.size())
        return false;

    // Otherwise, use this split
    split = min_dist;
    dir = min_dir;
    planarMode = min_planarMode;

    return true;
}
