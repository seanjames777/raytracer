/**
 * @file kdtree/kdbuilder.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <kdtree/kdbuilder.h>
#include <kdtree/kdsahbuilder.h>
#include <kdtree/kdmedianbuilder.h>

#include <iostream>
#include <util/align.h>
#include <util/timer.h>
#include <vector>
#include <cassert>

template<typename T>
KDBuilder<T>::KDBuilder(KDTree & tree, util::vector<Triangle, 16> & triangles)
    : outstanding_nodes(0),
      tree(tree),
      triangles(triangles),
      numUnclippedTriangles(triangles.size()),
      triangleID(triangles.size())
{
}

template<typename T>
KDBuilder<T>::~KDBuilder() {
}

template<typename T>
void KDBuilder<T>::partition(
    float                              split,
    int                                dir,
    enum KDBuilderPlanarMode         & planarMode,
    const util::vector<Triangle, 16> & triangles,
    util::vector<Triangle, 16>       & left,
    util::vector<Triangle, 16>       & right)
{
    // TODO: By keeping references to triangles in the SAH event list, we could
    // avoid this whole loop.

    for (auto it = triangles.begin(); it != triangles.end(); it++) {
        const Triangle & tri = *it;

        float min = fminf(fminf(tri.v[0].position[dir], tri.v[1].position[dir]), tri.v[2].position[dir]);
        float max = fmaxf(fmaxf(tri.v[0].position[dir], tri.v[1].position[dir]), tri.v[2].position[dir]);

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
        else if (max <= split) {
            left.push_back(tri);
        }
        else if (min >= split) {
            right.push_back(tri);
        }
        else {
#if 0
            float3 positions[3] = { tri.v[0].position, tri.v[1].position, tri.v[2].position };
            
            float3 bary_r[6];
            int n_r = clip(positions, bary_r, split, dir, 0x1);

            float3 bary_l[6];
            int n_l = clip(positions, bary_l, split, dir, 0x2);

            assert(n_l > 0 && n_l <= 2);
            assert(n_r > 0 && n_r <= 2);

            for (int i = 0; i < n_r; i++) {
                Triangle clipped(
                    tri.interpolate(bary_r[i * 3 + 0].y, bary_r[i * 3 + 0].z),
                    tri.interpolate(bary_r[i * 3 + 1].y, bary_r[i * 3 + 1].z),
                    tri.interpolate(bary_r[i * 3 + 2].y, bary_r[i * 3 + 2].z),
                    triangleID++,
                    tri.material_id);

                // TODO: kill degenerate triangles on the way in as well
                #if 1
                if (length(cross(
                    clipped.v[2].position - clipped.v[0].position,
                    clipped.v[1].position - clipped.v[0].position
                )) > 0)
                #endif
                    right.push_back(clipped);
            }

            for (int i = 0; i < n_l; i++) {
                Triangle clipped(
                    tri.interpolate(bary_l[i * 3 + 0].y, bary_l[i * 3 + 0].z),
                    tri.interpolate(bary_l[i * 3 + 1].y, bary_l[i * 3 + 1].z),
                    tri.interpolate(bary_l[i * 3 + 2].y, bary_l[i * 3 + 2].z),
                    triangleID++,
                    tri.material_id);
                
                #if 1
                if (length(cross(
                    clipped.v[2].position - clipped.v[0].position,
                    clipped.v[1].position - clipped.v[0].position
                )) > 0)
                #endif
                    left.push_back(clipped);
            }

            // TODO: Original triangle is now dead
#else
            left.push_back(tri);
            right.push_back(tri);
#endif
        }
    }
}

template<typename T>
void KDBuilder<T>::splitNode(
    KDBuilderNode            & builderNode,
    float                      split,
    int                        dir,
    enum KDBuilderPlanarMode   planarMode)
{
    KDBuilderNode *left = new KDBuilderNode();
    KDBuilderNode *right = new KDBuilderNode();

    builderNode.left = left;
    builderNode.right = right;
    
    left->depth = builderNode.depth + 1;
    right->depth = builderNode.depth + 1;

    builderNode.bounds.split(split, dir, left->bounds, right->bounds);

    // TODO: If the partition produced empty nodes, skip traversal
    partition(split, dir, planarMode, builderNode.triangles, left->triangles, right->triangles);
    builderNode.triangles.clear();

    builderNode.dir = dir;
    builderNode.split = split;

    queue_lock.lock();
    node_queue.push_back(left);
    node_queue.push_back(right);
    outstanding_nodes += 2;
    queue_lock.unlock();
}

template<typename T>
void KDBuilder<T>::finalizeLeafNode(
    const KDBuilderNode             & builderNode,
    KDNode                          & node)
{
    uint32_t numTriangles = (uint32_t)builderNode.triangles.size();
    uint32_t offset = 0;

    if (numTriangles > 0) {
        // Add any triangles introduced by clipping
        // TODO: Discard triangles which are no longer referenced
        for (auto & tri : builderNode.triangles) {
            if (tri.triangle_id >= numUnclippedTriangles)
                triangles[tri.triangle_id] = tri;
        }
        
        offset = tree.triangles.size() * sizeof(SetupTriangle);
        setupTriangles(builderNode.triangles, tree.triangles);
    }

    node.offset = (uint32_t)offset | KD_LEAF;
    node.count = numTriangles;
}

template<typename T>
void KDBuilder<T>::finalizeInnerNode(
    const KDBuilderNode             & builderNode,
    KDNode                          & node)
{   
    uint32_t offset = tree.nodes.size();

    node.offset = (offset * sizeof(KDNode)) | builderNode.dir;
    node.split_dist = builderNode.split;

    tree.nodes.push_back(KDNode());
    tree.nodes.push_back(KDNode());

    finalizeNode(*builderNode.left,  tree.nodes[offset + 0]);
    finalizeNode(*builderNode.right, tree.nodes[offset + 1]);

    delete builderNode.left;
    delete builderNode.right;
}

template<typename T>
void KDBuilder<T>::finalizeNode(
    const KDBuilderNode             & builderNode,
    KDNode                          & node)
{
    if (builderNode.left || builderNode.right)
        finalizeInnerNode(builderNode, node);
    else
        finalizeLeafNode(builderNode, node);
}

// TODO: traverse after construction to remove useless cells, etc.

template<typename T>
void KDBuilder<T>::buildNode(
    T & threadCtx,
    KDBuilderNode & builderNode)
{
    int dir;
    float split;
    enum KDBuilderPlanarMode planarMode;

    bool shouldSplit = false;

    // TODO: Expose these constants to the build algorithm
    if (builderNode.depth < 23 && builderNode.triangles.size() > 4) {
        shouldSplit = shouldSplitNode(
            threadCtx,
            builderNode.bounds,
            builderNode.triangles,
            builderNode.depth,
            split,
            dir,
            planarMode);
    }

    if (shouldSplit)
        splitNode(builderNode, split, dir, planarMode);
}

template<typename T>
AABB KDBuilder<T>::buildAABB(const util::vector<Triangle, 16> & triangles) {
    if (triangles.size() == 0)
        return AABB();

    AABB box = AABB(triangles[0].v[0].position);

    for (auto it = triangles.begin(); it != triangles.end(); it++) {
        const Triangle & tri = *it;
        box.join(tri.v[0].position);
        box.join(tri.v[1].position);
        box.join(tri.v[2].position);
    }

    return box;
}

template<typename T>
void KDBuilder<T>::builder_thread() {
    T ctx;

    util::queue<KDBuilderNode *, 8> local_queue;

    while (true) {
        int local_count = 0;

        queue_lock.lock();

        // If there are no more nodes to process, then we will never have more
        // work to do.
        if (outstanding_nodes == 0) {
            queue_lock.unlock();
            break;
        }

#if 0
        // If the queue is empty, sleep until it isn't. We wake up with the lock again.
        // TODO use condition variable to do this.

        // The queue may now have work, but if another THREAD woke up first and consumed all of it,
        // then we will just loop around and go back to sleep.

        // TODO: Arbitary constant
        // Collect a batch of nodes of a reasonable size. This reduces locking overhead. The batch
        // should be big enough to avoid overhead of tiny nodes, but small enough to ensure that
        // other threads can do work in parallel.
        while (!node_queue.empty() && local_count < 1024) {
            KDBuilderNode *node = node_queue.front();
            node_queue.pop_front();
            local_queue.push_back(node);
            local_count += (int)node->triangles.size(); // TODO int
        }

        // TODO: Instead of waking up everybody, it may be better to wake up another THREAD if there
        // are more in the queue. We'd probably have to know how many threads are waiting.

        queue_lock.unlock();

        // Process the batch we collected. When we're done, go back to either
        // immediately grab another batch or go to sleep.

        while (!local_queue.empty()) {
            KDBuilderNode *node = local_queue.front();
            local_queue.pop_front();

            buildNode(ctx, *node);

            // Note: This must happen after the node is built, to ensure that its children
            // are enqueued.
            --outstanding_nodes;
        }
#endif

        if (node_queue.empty()) {
            queue_lock.unlock();
            continue;
        }

        KDBuilderNode *node = node_queue.front();

        node_queue.pop_front();

        queue_lock.unlock();

        buildNode(ctx, *node);

        --outstanding_nodes;
    }
}

template<typename T>
void KDBuilder<T>::computeStats(KDNode *root, KDTreeStats *stats, int depth) {
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

        computeStats(root->left(&tree.nodes[0]), stats, depth + 1);
        computeStats(root->right(&tree.nodes[0]), stats, depth + 1);
    }
}

template<typename T>
void KDBuilder<T>::build(KDTreeStats *stats) {
    Timer timer;

    std::cout << "Building KD tree" << std::endl;

    tree.bounds = buildAABB(triangles);

    KDBuilderNode *builderNode = new KDBuilderNode();

    // Use pointers while building the tree, because we need to be able to sort, etc.
    // Triangles are converted to "setup" triangles, so we don't actually need the
    // triangle data anyway.

    // TODO: Actually use pointers again
    // TODO: Compress triangle lists
    // TODO: redundant
    for (auto & tri : triangles)
        builderNode->triangles.push_back(tri);

    builderNode->bounds = tree.bounds;
    builderNode->depth = 0;

    outstanding_nodes = 1;
    node_queue.push_back(builderNode);

#ifndef NDEBUG
	// TODO: reserving a core for the rest of the system
	int num_threads = max((int)std::thread::hardware_concurrency() - 1, 1);
#else
	int num_threads = std::thread::hardware_concurrency();
#endif

#if 0
    num_threads = 1;
#endif

    std::vector<std::thread> workers;

    for (int i = 0; i < num_threads; i++)
        workers.push_back(std::thread(std::bind(&KDBuilder::builder_thread, this)));

    std::cout << "Started " << num_threads << " worker threads" << std::endl;

    for (auto & worker : workers)
        worker.join();

    workers.clear();

    std::cout << "Finalizing KD tree" << std::endl;

    // TODO: vector resize()
    for (uint32_t i = 0; i < triangleID - numUnclippedTriangles; i++)
        triangles.push_back(Triangle());

    tree.nodes.push_back(KDNode());

    finalizeNode(*builderNode, tree.nodes[0]);
    tree.root = &tree.nodes[0];

    delete builderNode;

    std::cout << "Computing KD tree statistics" << std::endl;

    if (stats) {
        memset(stats, 0, sizeof(KDTreeStats));
        computeStats(tree.root, stats, 1);
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
        printf("Triangles:        %d (average %.02f, %.02fx input)\n", stats->num_triangles, (float)stats->num_triangles / (float)stats->num_leaves, (float)stats->num_triangles / (float)numUnclippedTriangles);
        printf("Max Node Depth:   %d\n", stats->max_depth);
        printf("Min Node Depth:   %d\n", stats->min_depth);
        printf("Avg Node Depth:   %.02f\n", (float)stats->sum_depth / (float)stats->num_leaves);
        printf("Empty Leaf Nodes: %d (%.02f%%)\n", stats->num_zero_leaves, (float)stats->num_zero_leaves / (float)stats->num_leaves * 100.0f);
        printf("Tree Memory:      %.02fmb\n", stats->tree_mem / (1024.0f * 1024.0f));
        printf("Triangle Memory:  %.02fmb\n", stats->num_triangles * sizeof(SetupTriangle) / (1024.0f * 1024.0f));
    }
}

template class KDBuilder<KDSAHBuilderThreadCtx>;
template class KDBuilder<KDMedianBuilderThreadCtx>;
