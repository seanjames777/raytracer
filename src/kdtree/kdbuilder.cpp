/**
 * @file kdbuilder.cpp
 *
 * @author Sean James
 */

#include <kdtree/kdbuilder.h>
#include <iostream>

#ifndef _WINDOWS
#include <sys/time.h>
#endif

KDBuilder::KDBuilder()
    : node_queue(nullptr),
      queue_capacity(0),
      queue_head(0),
      queue_tail(0),
      queue_size(0)
{
}

KDBuilder::~KDBuilder() {
    if (node_queue)
        free(node_queue);
}

void KDBuilder::enqueue_node(KDBuilderQueueNode *node) {
    // Resize the queue if needed
    if (queue_size + 1 > queue_capacity) {
        int old_capacity = queue_capacity;

        while (queue_size + 1 > queue_capacity) {
            // If queue hasn't been allocated, allocate (power of two)
            if (queue_capacity == 0)
                queue_capacity = 128; // TODO
            // Otherwise, double the size (power of two)
            else
                queue_capacity *= 2;
        }

        KDBuilderQueueNode ** new_queue = (KDBuilderQueueNode **)malloc(sizeof(KDBuilderQueueNode *) * queue_capacity);

        // Copy the old queue
        if (node_queue) {
            // TODO: Might want to realloc where possible instead of always copying, in case the
            // allocator can just extend in place.

            // Queue does not wrap around, a single copy is sufficient. Compact to beginning of new
            // queue for simplicity. We will never encounter queue_head == queue_tail because otherwise
            // we wouldnt be expanding the queue.
            if (queue_tail > queue_head) {
                memcpy(new_queue, node_queue + queue_head, queue_size * sizeof(KDBuilderQueueNode *));
                queue_head = 0;
                queue_tail = queue_size;
            }
            // Otherwise, queue wraps around, so we need two copies. Copy to beginning of new queue
            // so that everything is guaranteed to fit without wrapping, since we've doubled the size.
            else {
                // Size of chunks at the end and beginning of array, due to wrap around
                int end_size = old_capacity - queue_head;
                int begin_size = queue_tail;

                memcpy(new_queue, node_queue + queue_head, end_size * sizeof(KDBuilderQueueNode *));
                memcpy(new_queue + end_size, node_queue, begin_size * sizeof(KDBuilderQueueNode *));

                queue_head = 0;
                queue_tail = begin_size + end_size;
            }

            // Delete the old queue
            free(node_queue);
        }
        // Set the newly allocated queue
        else
            node_queue = new_queue;
    }

    node_queue[queue_tail] = node;

    queue_tail = (queue_tail + 1) & (queue_capacity - 1); // Queue capacity is always a power of two
    queue_size++;
}

KDBuilderQueueNode *KDBuilder::dequeue_node() {
    // If the two pointers are equal, the queue is empty.
    if (queue_head == queue_tail)
        return nullptr;

    KDBuilderQueueNode *node = node_queue[queue_head];

    queue_head = (queue_head + 1) & (queue_capacity - 1); // Queue capacity is always a power of two
    queue_size--;

    return node;
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

KDTree *KDBuilder::build(const std::vector<Triangle> & triangles) {
#ifndef _WINDOWS
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long long start = tv.tv_sec * 1000000 + tv.tv_usec;
#endif

    // Use pointers while building the tree, because we need to be able to sort, etc.
    // Triangles are converted to "setup" triangles, so we don't actually need the
    // triangle data anyway.
    std::vector<Triangle *> pointers;

    pointers.reserve(triangles.size());

    for (auto & it : triangles)
        pointers.push_back(const_cast<Triangle *>(&it));

    AABB bounds = buildAABB(pointers);

    KDNode *root = buildNode(bounds, pointers, 0);

#ifndef _WINDOWS
    // TODO
    gettimeofday(&tv, NULL);
    unsigned long long end = tv.tv_sec * 1000000 + tv.tv_usec;
    std::cout << "KD Build Time: " << (end - start) / 1000.0f << "ms" << std::endl;
#endif

    return new KDTree(root, bounds);
}
