/**
 * @file kdtree.cpp
 *
 * @author Sean James
 */

#include <kdtree.h>
#include <iostream>
#include <sys/time.h>

KDBuilder::KDBuilder() {
}

KDBuilder::~KDBuilder() {
}

KDMeanBuilder::KDMeanBuilder() {
}

KDMeanBuilder::~KDMeanBuilder() {
}

KDSAHBuilder::KDSAHBuilder() {
}

KDSAHBuilder::~KDSAHBuilder() {
}

bool KDMeanBuilder::splitNode(
    const AABB & bounds,
    const std::vector<Triangle *> & triangles,
    int depth,
    int & dir,
    float & split,
    enum PlanarMode & planarMode)
{
    if (depth >= 25 || triangles.size() < 4)
        return false;

#if 1
    // Split along longest axis
    vec3 axis_len = bounds.max - bounds.min;

    if (axis_len.x > axis_len.y)
        dir = axis_len.x > axis_len.z ? KD_SPLIT_DIR_X : KD_SPLIT_DIR_Z;
    else
        dir = axis_len.y > axis_len.z ? KD_SPLIT_DIR_Y : KD_SPLIT_DIR_Z;
#else
    // Split round robin
    dir = depth % 3;
#endif

    float min = bounds.min.v[dir];
    float max = bounds.max.v[dir];

    split = (max - min) / 2.0f + min;

    planarMode = PLANAR_BOTH;

    return true;
}

enum SAHEventType {
    SAH_END = 0,    // Triangle end
    SAH_PLANAR = 1, // Trianlge lying in plane
    SAH_BEGIN = 2   // Triangle start
};

struct SAHEvent {
    int flag;       // Event type
    float dist;     // Split location
};

bool compare(const SAHEvent & e1, const SAHEvent & e2) {
    // Sort by plane location then event type
    if (e1.dist < e2.dist)
        return true;
    else if (e1.dist == e2.dist)
        return e1.flag < e2.flag;
    else
        return false;
}

bool KDSAHBuilder::splitNode(
    const AABB & bounds,
    const std::vector<Triangle *> & triangles,
    int depth,
    int & dir,
    float & split,
    enum PlanarMode & planarMode)
{
    /*std::cout << "Bounds: <" << bounds.min.x << ", " << bounds.min.y << ", " << bounds.min.z << ">, <"
        << bounds.max.x << ", " << bounds.max.y << ", " << bounds.max.z << ">" << std::endl;
    std::cout << "Triangles: " << triangles.size() << std::endl;*/

    // Assumptions:
    //     - There is at least one triangle in the node
    //     - Each triangle at least partially overlaps the node, possible just touching (<= vs. <)
    //
    // This algorithm takes O(N*log^2(N)), and is discussed in the following paper:
    // http://dcgi.felk.cvut.cz/home/havran/ARTICLES/ingo06rtKdtree.pdf

    if (triangles.size() == 0)
        return false;

    // The algorithm is implemented by generating and processing a sequence of "events" along a
    // fixed axis (X, Y, or Z). "Begin" events mark the beginning of a triangle. "End" events mark
    // the end of a triangle. Triangles may start and stop at the same point, in which case a
    // "Plane" event is generated and handled specially. More than one triangle may start or stop
    // at the same point, so multiple events must be processed at each sweep plane location.
    std::vector<SAHEvent> events;
    events.reserve(triangles.size() * 2);

    // We want to find the plane which minimizes the "surface area heuristic"
    int             min_dir        = -1;          // Split direction
    float           min_dist       = 0.0f;        // Split location
    float           min_cost       = 1.0f / 0.0f; // Split cost. TODO
    enum PlanarMode min_planarMode = PLANAR_LEFT; // How to handle planar triangles

    // Heuristic constants
    static const float k_t = 1.0f;  // Cost of traversing a KD node
    static const float k_i = 1.0f; // Cost of intersecting a triangle

    // Surface area of parent voxel
    float sa_v = bounds.surfaceArea();

    // Try each major axis in turn
    for (int axis = 0; axis < 3; ++axis) {
        events.clear();

        // Min and max of parent node along this axis
        float min = bounds.min.v[axis];
        float max = bounds.max.v[axis];

        SAHEvent event;

        // Insert start/stop/planar locations of each triangle, clamped to the bounds of the parent
        // box. We assume we won't see triangles fully outside the parent node.
        for (auto tri : triangles) {
            // Triangle bounding box
            float tri_min = tri->bbox.min.v[axis];
            float tri_max = tri->bbox.max.v[axis];

            // Clip triangle bounding box to voxel bounding box
            tri_min = tri_min < min ? min : tri_min;
            tri_max = tri_max > max ? max : tri_max;

            // If the triangle min is the same as the triangle max the triangle is planar
            if (tri_min == tri_max) {
                event.flag = SAH_PLANAR;
                event.dist = tri_min;
                events.push_back(event);
            }
            // Otherwise, generate begin and end events
            else {
                // TODO: What if the clipped bounding box is planar

                event.flag = SAH_END;
                event.dist = tri_max;
                events.push_back(event);

                event.flag = SAH_BEGIN;
                event.dist = tri_min;
                events.push_back(event);
            }
        }

        // Sort events by type and then by position so that we can just sweep from the minimum
        // point to the maximum point instead of repeatedly partitioning. Sweeping along the vector
        // in order will be cache friendly as well.
        std::sort(events.begin(), events.end(), compare);

        /*for (auto & evt : events)
            std::cout <<
                (evt.flag == SAH_PLANAR ? "PLANAR " : (evt.flag == SAH_BEGIN ? "BEGIN " : "END ")) << " " <<
                evt.dist << std::endl;*/

        // Number of triangles entirely to the left and right of the sweep plane
        int count_left = 0;
        int count_right = triangles.size();

        int num_events = events.size();
        int event_idx = 0;

        // Sweep along axis processing events
        while (event_idx < num_events) {
            auto & event = events[event_idx];
            float dist = event.dist;

            // Trianglgs touching the sweep plane: starting, lying in (parallel), and ending
            int count_starting = 0;
            int count_ending = 0;
            int count_planar = 0;

            // Handle all events at this plane position
            while (event_idx < num_events) {
                auto & event = events[event_idx];

                if (event.dist != dist)
                    break;

                // TODO: can be broken into 3 loops to avoid the switch, although
                // they are at least sorted.
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

                ++event_idx;
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

    /*std::cout << "Split: " <<
        "(" << (dir == 0 ? "X" : (dir == 1 ? "Y" : "Z")) << ")" << " " <<
        "(" << (min_planarMode == PLANAR_LEFT ? "L" : "R") << "), " <<
        bounds.min.v[dir] << " < " << split << " < " << bounds.max.v[dir] << std::endl;*/

    return true;
}

KDNode::KDNode(KDNode *left, KDNode *right, float split_dist,
    SetupTriangle *triangles, unsigned int flags)
    : left(left),
      right(right),
      split_dist(split_dist),
      triangles(triangles),
      flags(flags)
{
}

KDNode::~KDNode() {
    if (flags & KD_IS_LEAF)
        free(triangles);
    else {
        delete left;
        delete right;
    }
}

void KDBuilder::partition(float dist, int dir, const std::vector<Triangle *> & triangles,
        std::vector<Triangle *> & left, std::vector<Triangle *> & right,
        enum PlanarMode & planarMode)
{
    for (auto it = triangles.begin(); it != triangles.end(); it++) {
        Triangle *tri = *it;

        float min = tri->bbox.min.v[dir];
        float max = tri->bbox.max.v[dir];

        if (min == dist && max == dist) {
            // TODO bitwise optimization
            if (planarMode == PLANAR_LEFT)
                left.push_back(tri);
            else if (planarMode == PLANAR_RIGHT)
                right.push_back(tri);
            else {
                left.push_back(tri);
                right.push_back(tri);
            }
        }
        else {
            if (min < dist)
                left.push_back(tri);

            if (max > dist)
                right.push_back(tri);
        }
    }
}

KDNode *KDBuilder::buildLeaf(const std::vector<Triangle *> & triangles) {
    // Set up triangles, and pack them together for locality
    unsigned int num_triangles = (unsigned int)triangles.size();
    SetupTriangle *setup = (SetupTriangle *)malloc(sizeof(SetupTriangle) * num_triangles);

    for (unsigned int i = 0; i < num_triangles; i++)
        setup[i] = SetupTriangle(*triangles[i]); // TODO constructor in place

    // TODO: Might want to change constructor of KDNode to do this stuff,
    // especially to not mix allocations

    // TODO: handle overflow of size?

    return new KDNode(NULL, NULL, 0.0f, setup, num_triangles | KD_IS_LEAF);
}

KDNode *KDBuilder::buildNode(const AABB & bounds, const std::vector<Triangle *> & triangles, int depth) {
    // TODO: possibly traverse after construction to remove useless cells, etc.

    int dir;
    float split;
    enum PlanarMode planarMode;

    if (splitNode(bounds, triangles, depth, dir, split, planarMode)) {
        float min = bounds.min.v[dir];

        AABB leftBB, rightBB;
        bounds.split(split, dir, leftBB, rightBB);

        // TODO might want to reserve space to avoid allocations
        std::vector<Triangle *> leftContained;
        std::vector<Triangle *> rightContained;
        partition(split, dir, triangles, leftContained, rightContained, planarMode);

        KDNode *left = buildNode(leftBB, leftContained, depth + 1);
        KDNode *right = buildNode(rightBB, rightContained, depth + 1);

        return new KDNode(left, right, split, NULL, (KDNodeFlags)dir);
    }
    else
        return buildLeaf(triangles);
}

AABB KDBuilder::buildAABB(const std::vector<Triangle *> & triangles) {
    if (triangles.size() == 0)
        return AABB();

    AABB box = triangles[0]->bbox;

    for (auto it = triangles.begin(); it != triangles.end(); it++)
        box.join((*it)->bbox);

    return box;
}

bool KDTree::intersectLeaf(KDNode *leaf, const Ray & ray, Collision & result, float entry, float exit,
    bool anyCollision)
{
    bool found = false;
    Collision tmpResult; // TODO

    unsigned int num_triangles = leaf->flags & KD_SIZE_MASK;

    for (int i = 0; i < num_triangles; i++) {
        SetupTriangle *triangle = &leaf->triangles[i];

        bool intersects = triangle->intersects(ray, tmpResult);

        if (intersects && tmpResult.distance >= entry &&
            tmpResult.distance <= exit && (tmpResult.distance < result.distance || !found))
        {
            result = tmpResult;

            if (anyCollision)
                return true;

            found = true;
        }
    }

    return found;
}

KDTree *KDBuilder::build(const std::vector<Triangle> & triangles) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long long start = tv.tv_sec * 1000000 + tv.tv_usec;

    // Use pointers while building the tree, because we need to be able to sort, etc.
    // Triangles are converted to "setup" triangles, so we don't actually need the
    // triangle data anyway.
    std::vector<Triangle *> pointers;

    pointers.reserve(triangles.size());

    for (auto & it : triangles)
        pointers.push_back(const_cast<Triangle *>(&it));

    AABB bounds = buildAABB(pointers);

    KDNode *root = buildNode(bounds, pointers, 0);

    gettimeofday(&tv, NULL);
    unsigned long long end = tv.tv_sec * 1000000 + tv.tv_usec;
    std::cout << "KD Build Time: " << (end - start) / 1000.0f << "ms" << std::endl;

    return new KDTree(root, bounds);
}

KDTree::KDTree(KDNode *root, AABB bounds)
    : root(root),
      bounds(bounds)
{
}

bool KDTree::intersect(const Ray & ray, Collision & result, float maxDepth, bool anyCollision) {
    float entry, exit;

    if (!bounds.intersects(ray, &entry, &exit))
        return false;

    KDStackFrame stack[64]; // TODO max size
    KDStackFrame *curr_stack = stack;

    curr_stack->node = root;
    curr_stack->enter = entry;
    curr_stack->exit = exit;
    curr_stack++;

    KDNode *currentNode;

    while (curr_stack != stack) {
        curr_stack--;
        currentNode = curr_stack->node;
        entry = curr_stack->enter;
        exit = curr_stack->exit;

        // Nothing will be closer, give up
        if (maxDepth > 0.0f && entry > maxDepth)
            return false;

        while (!(currentNode->flags & KD_IS_LEAF)) {
            int dir = currentNode->flags & KD_SPLIT_DIR_MASK;

            float split = currentNode->split_dist;
            float origin = ray.origin.v[dir];

            float t = (split - origin) * ray.inv_direction.v[dir];

            KDNode *nearNode = currentNode->left;
            KDNode *farNode  = currentNode->right;

            if (split < origin) {
                KDNode *temp = nearNode;
                nearNode = farNode;
                farNode = temp;
            }

            if (t > exit || t < 0)
                currentNode = nearNode;
            else if (t < entry)
                currentNode = farNode;
            else {
                curr_stack->node = farNode;
                curr_stack->enter = t;
                curr_stack->exit = exit;
                curr_stack++;

                currentNode = nearNode;
                exit = t;
            }
        }

        // Again, nothing will be closer so we're done
        if (intersectLeaf(currentNode, ray, result, entry, maxDepth > 0.0f ?
            MIN2(maxDepth, exit) : exit, anyCollision))
            return true;
    }

    return false;
}
