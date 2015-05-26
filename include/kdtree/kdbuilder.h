/**
 * @file kdtree/kdbuilder.h
 *
 * @brief KD-Tree builder base class
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __KDBUILDER_H
#define __KDBUILDER_H

#include <atomic>
#include <kdtree/kdtree.h>
#include <mutex>
#include <thread>
#include <util/queue.h>
#include <vector>

// How to handle triangles that lie in the plane
enum PlanarMode {
    PLANAR_LEFT =  1,
    PLANAR_RIGHT = 2,
    PLANAR_BOTH =  3
};

struct KDBuilderQueueNode {
    KDNode *node;
    AABB bounds;
    std::vector<Triangle *> triangles; // TODO: memcpy/malloc instead
    int depth;
    std::atomic_int refCount;
    KDBuilderQueueNode *parent;

    // TODO: false sharing
};

struct KDTreeStatistics {
    int num_nodes;
    int num_leaves;
    int num_internal;
    int num_triangles;
    int max_depth;
    int min_depth;
    int sum_depth;
    int num_zero_leaves;
    int tree_mem;
};

class KDBuilder {
private:

    // TODO: Pool or something of queue nodes might be better than new/delete constantly.
    // TODO: Check overhead of queue locking. Maybe use work stealing or something.
    // Note: We need a dynamic queue here because we don't know how deep we're going
    // to go while building the tree, and we need more storage space closer to the
    // leaves.
    util::queue<KDBuilderQueueNode *> node_queue;
    std::mutex                        queue_lock;
    std::atomic_int                   outstanding_nodes;

    // TODO
    void worker_thread();

    // TODO
    virtual void *prepareWorkerThread(int idx);

    // TODO
    virtual void destroyWorkerThread(void *threadCtx);

    /**
     * @brief Find the subset of triangles contained in a bounding box
     *
     * @param box       Bounding box
     * @param triangles     Items to check
     * @param contained Vector to fill with triangles overlapping box
     */
    void partition(void *threadCtx, float dist, int dir, const std::vector<Triangle *> & triangles,
        std::vector<Triangle *> & left, std::vector<Triangle *> & right,
        enum PlanarMode & planarMode);

    /**
     * @brief Build a leaf node
     *
     * @param q_node Queue node to process
     */
    void buildLeafNode(KDBuilderQueueNode *q_node);

    /**
     * @brief Build an inner node
     *
     * @param q_node     Queue node to process
     * @param dir        Split axis
     * @param split      Split distance
     * @param planarMode How to handle planar triangles
     * @param depth      Tree depth
     */
    void buildInnerNode(void *threadCtx, KDBuilderQueueNode *q_node, int dir, float split,
        enum PlanarMode planarMode, int depth);

    /**
     * @brief Build a KD node
     *
     * @param q_node Queue node to process
     */
    void buildNode(void *threadCtx, KDBuilderQueueNode *q_node);

    /**
     * @brief Compute a bounding box for a set of triangles
     *
     * @param item Items to bound
     */
    AABB buildAABB(const std::vector<Triangle *> & triangles);

protected:

    // TODO
    virtual bool splitNode(void *threadCtx, const AABB & bounds, const std::vector<Triangle *> & triangles,
        int depth, int & dir, float & split, enum PlanarMode & planarMode) = 0;

public:

    KDBuilder();

    virtual ~KDBuilder();

    KDTree *build(const std::vector<Triangle> & triangles, KDTreeStatistics *stats = nullptr);

};

#endif
