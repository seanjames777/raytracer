/**
 * @file kdtree/kdbuilder.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <kdtree/kdbuilder.h>

#include <iostream>
#include <util/align.h>
#include <util/timer.h>

// TODO: the enqueue() function can fail

KDBuilder::KDBuilder() {
}

KDBuilder::~KDBuilder() {
}

void *KDBuilder::prepareWorkerThread(int idx) {
    return nullptr;
}

void KDBuilder::destroyWorkerThread(void *threadCtx) {
}

void KDBuilder::partition(
    float                           split,
    int                             dir,
    enum KDBuilderPlanarMode      & planarMode,
    const std::vector<Triangle *> & triangles,
    std::vector<Triangle *>       & left,
    std::vector<Triangle *>       & right)
{
    // TODO: By keeping references to triangles in the SAH event list, we could
    // avoid this whole loop.

    for (auto it = triangles.begin(); it != triangles.end(); it++) {
        Triangle *tri = *it;

        float min = fminf(fminf(tri->v[0].position[dir], tri->v[1].position[dir]), tri->v[2].position[dir]);
        float max = fmaxf(fmaxf(tri->v[0].position[dir], tri->v[1].position[dir]), tri->v[2].position[dir]);

        if (min == split && max == split) {
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
            if (min < split)
                left.push_back(tri);

            if (max > split)
                right.push_back(tri);
        }
    }
}

void KDBuilder::buildLeafNode(KDBuilderQueueNode *q_node) {
    // Set up triangles, and pack them together for locality
    unsigned int num_triangles = (unsigned int)q_node->triangles.size();
    
    uint32_t offset = 0;

    if (num_triangles > 0) {
        uint32_t size = num_triangles * sizeof(SetupTriangle);
        offset = _triangleAllocator->alloc(size, 8);
        
        // TODO: We sometimes get empty leaves, which is a total waste
        SetupTriangle *triangleData = (SetupTriangle *)((char *)_triangleAllocator->memory() + offset);
        
        setupTriangles(&q_node->triangles[0], triangleData, q_node->triangles.size());
    }

    // TODO: handle overflow of size?

    KDNode *node = q_node->node;

    node->offset = (uint32_t)offset | KD_LEAF;
    node->count = q_node->triangles.size();
}

void KDBuilder::buildInnerNode(
    KDBuilderQueueNode       *q_node,
    float                     split,
    int                       dir,
    enum KDBuilderPlanarMode  planarMode,
    int                       depth)
{
    KDBuilderQueueNode *left = new KDBuilderQueueNode();
    KDBuilderQueueNode *right = new KDBuilderQueueNode();
    
    uint32_t offset = _nodeAllocator->alloc(sizeof(KDNode) * 2, 4);

    left->depth = depth + 1;
    left->refCount = 2;
    left->parent = q_node;
    left->node = (KDNode *)((char *)_nodeAllocator->memory() + offset);

    right->depth = depth + 1;
    right->refCount = 2;
    right->parent = q_node;
    right->node = (KDNode *)((char *)_nodeAllocator->memory() + offset + sizeof(KDNode));

    q_node->bounds.split(split, dir, left->bounds, right->bounds);

    // TODO might want to reserve space to avoid allocations
    partition(split, dir, planarMode, q_node->triangles, left->triangles, right->triangles);

    KDNode *node = q_node->node;

    node->offset = offset | dir;
    node->split_dist = split;

    queue_lock.lock();
    node_queue.enqueue(left);
    node_queue.enqueue(right);
    outstanding_nodes += 2;
    queue_lock.unlock();
}

void KDBuilder::buildNode(void *threadCtx, KDBuilderQueueNode *q_node) {
    // TODO: possibly traverse after construction to remove useless cells, etc.

    int dir;
    float split;
    enum KDBuilderPlanarMode planarMode;

    if (splitNode(threadCtx, q_node->bounds, q_node->triangles, q_node->depth, split, dir, planarMode))
        buildInnerNode(q_node, split, dir, planarMode, q_node->depth);
    else
        buildLeafNode(q_node);
}

AABB KDBuilder::buildAABB(const std::vector<Triangle *> & triangles) {
    if (triangles.size() == 0)
        return AABB();

    AABB box = AABB(triangles[0]->v[0].position);

    for (auto it = triangles.begin(); it != triangles.end(); it++) {
        const Triangle & tri = **it;
        box.join(tri.v[0].position);
        box.join(tri.v[1].position);
        box.join(tri.v[2].position);
    }

    return box;
}

void KDBuilder::worker_thread() {
    void *ctx = prepareWorkerThread(0); // TODO: actual index

    util::queue<KDBuilderQueueNode *> local_queue;

    while (true) {
        int local_count = 0;

        queue_lock.lock();

        // If there are no more nodes to process, then we will never have more
        // work to do.
        if (outstanding_nodes == 0) {
            queue_lock.unlock();
            break;
        }

        // If the queue is empty, sleep until it isn't. We wake up with the lock again.
        // TODO use condition variable to do this.

        // The queue may now have work, but if another THREAD woke up first and consumed all of it,
        // then we will just loop around and go back to sleep.

        // TODO: Arbitary constant
        // Collect a batch of nodes of a reasonable size. This reduces locking overhead. The batch
        // should be big enough to avoid overhead of tiny nodes, but small enough to ensure that
        // other threads can do work in parallel.
        while (!node_queue.empty() && local_count < 20000) {
            KDBuilderQueueNode *node = node_queue.dequeue();
            local_queue.enqueue(node);
            local_count += (int)node->triangles.size(); // TODO int
        }

        // TODO: Instead of waking up everybody, it may be better to wake up another THREAD if there
        // are more in the queue. We'd probably have to know how many threads are waiting.

        queue_lock.unlock();

        // Process the batch we collected. When we're done, go back to either
        // immediately grab another batch or go to sleep.

        while (!local_queue.empty()) {
            KDBuilderQueueNode *node = local_queue.dequeue();

            buildNode(ctx, node);

            // Decrement reference count of parent. If we are the last child node to finish,
            // delete the parent and the memory containing its triangle list. The root node
            // doesn't have a parent.
            if (node->parent && --node->parent->refCount == 0)
                delete node->parent;

            // Note: This must happen after the node is built, to ensure that its children
            // are enqueued.
            --outstanding_nodes;
        }
    }
}

void KDBuilder::computeStats(KDNode *root, KDBuilderTreeStatistics *stats, int depth) {
    // TODO: Pass to eliminate empty leaves
    // TODO: Pass that eliminates leaves that don't improve on their parents split

    stats->num_nodes++;
    stats->tree_mem += sizeof(KDNode);

    if (root->type() == KD_LEAF) {
        stats->num_leaves++;
        stats->num_triangles += root->count;

        if (root->count == 0)
            stats->num_zero_leaves++;

        stats->sum_depth += depth;

        if (depth > stats->max_depth)
            stats->max_depth = depth;

        if (depth < stats->min_depth || stats->min_depth == 0)
            stats->min_depth = depth;
    }
    else {
        stats->num_internal++;

        computeStats(root->left((KDNode *)_nodeAllocator->memory()), stats, depth + 1);
        computeStats(root->right((KDNode *)_nodeAllocator->memory()), stats, depth + 1);
    }
}

KDTree *KDBuilder::build(const Triangle *triangles, int num_triangles, KDAllocator *nodeAllocator, KDAllocator *triangleAllocator, KDBuilderTreeStatistics *stats)
{
    _nodeAllocator = nodeAllocator;
    _triangleAllocator = triangleAllocator;
    
    Timer timer;

    std::cout << "Building KD tree" << std::endl;

    KDBuilderQueueNode *q_node = new KDBuilderQueueNode();

    // Use pointers while building the tree, because we need to be able to sort, etc.
    // Triangles are converted to "setup" triangles, so we don't actually need the
    // triangle data anyway.
    q_node->triangles.reserve(num_triangles);

    for (int i = 0; i < num_triangles; i++)
        q_node->triangles.push_back(const_cast<Triangle *>(&triangles[i]));
    
    uint32_t offset = nodeAllocator->alloc(sizeof(KDNode), 4);
    KDNode *root = (KDNode *)((char *)nodeAllocator->memory() + offset);

    q_node->bounds = buildAABB(q_node->triangles);
    q_node->depth = 0;
    q_node->node = root;
    q_node->parent = nullptr;

    // Reference count of 3 so that we can access its members after construction has
    // finished.
    q_node->refCount = 3;

    outstanding_nodes = 1;
    node_queue.enqueue(q_node);

#ifndef NDEBUG
	// TODO: reserving a core for the rest of the system
	int num_threads = max((int)std::thread::hardware_concurrency() - 1, 1);
#else
	int num_threads = std::thread::hardware_concurrency();
#endif

    std::vector<std::thread> workers;

    for (int i = 0; i < num_threads; i++)
        workers.push_back(std::thread(std::bind(&KDBuilder::worker_thread, this)));

    std::cout << "Started " << num_threads << " worker threads" << std::endl;

    for (auto & worker : workers)
        worker.join();

    workers.clear();

    if (stats) {
        memset(stats, 0, sizeof(KDBuilderTreeStatistics));
        computeStats(q_node->node, stats, 1);
    }

    double elapsed = timer.getElapsedMilliseconds() / 1000.0;
    double cpu     = timer.getCPUTime() / 1000.0;

    printf("Done: %f seconds (total), %f seconds (CPU), speedup: %.02f\n",
        elapsed, cpu, cpu / elapsed);

    if (stats) {
        std::cout << "KD Tree statistics:" << std::endl;

        printf("Nodes:            %d\n", stats->num_nodes);
        printf("Leaf Nodes:       %d (%.02f%%)\n", stats->num_leaves, (float)stats->num_leaves / (float)stats->num_nodes * 100.0f);
        printf("Internal Nodes:   %d (%.02f%%)\n", stats->num_internal, (float)stats->num_internal / (float)stats->num_nodes * 100.0f);
        printf("Triangles:        %d (average %.02f, %.02fx input)\n", stats->num_triangles, (float)stats->num_triangles / (float)stats->num_leaves, (float)stats->num_triangles / (float)num_triangles);
        printf("Max Node Depth:   %d\n", stats->max_depth);
        printf("Min Node Depth:   %d\n", stats->min_depth);
        printf("Avg Node Depth:   %.02f\n", (float)stats->sum_depth / (float)stats->num_leaves);
        printf("Empty Leaf Nodes: %d (%.02f%%)\n", stats->num_zero_leaves, (float)stats->num_zero_leaves / (float)stats->num_leaves * 100.0f);
        printf("Tree Memory:      %.02fmb\n", stats->tree_mem / (1024.0f * 1024.0f));
        printf("Triangle Memory:  %.02fmb\n", stats->num_triangles * sizeof(SetupTriangle) / (1024.0f * 1024.0f));
    }

    KDTree *tree = new KDTree(q_node->node, (KDNode *)nodeAllocator->memory(), (SetupTriangle *)triangleAllocator->memory(), q_node->bounds);

    // TODO
    // assert(--q_node->refCount == 0 || (q_node->node->flags & KD_IS_LEAF));

    delete q_node;

    return tree;
}
