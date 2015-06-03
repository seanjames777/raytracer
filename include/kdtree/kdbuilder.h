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

// TODO: Queue node might have some false sharing/atomic contention
// TODO: Pool or something of queue nodes might be better than new/delete constantly.
// TODO: Check overhead of queue locking. Maybe use work stealing or something.
// TODO: The top-down recursive model here does not allow for bottom-up strategies
//       like agglomerative clustering.

/**
 * @brief Options for which child node to place triangles that lie in the split
 * plane of a node into
 */
enum KDBuilderPlanarMode {
    PLANAR_LEFT,  //!< Left child only
    PLANAR_RIGHT, //!< Right child only
    PLANAR_BOTH   //!< Duplicate and place in both children
};

/**
 * @brief Node in a KDBuilder's work queue
 */
struct KDBuilderQueueNode {
    KDNode                  *node;      //!< KD-tree node to build
    AABB                     bounds;    //!< Bounds of all triangles
    std::vector<Triangle *>  triangles; //!< Triangles to place in node
    int                      depth;     //!< Node depth in KD-tree
    std::atomic_int          refCount;  //!< Number of references to queue node
    KDBuilderQueueNode      *parent;    //!< Parent node
};

/**
 * @brief Statistics about a KD-tree
 */
struct KDBuilderTreeStatistics {
    int num_nodes;       //!< Number of nodes
    int num_leaves;      //!< Number of leaf nodes
    int num_internal;    //!< Number of non-leaf nodes
    int num_triangles;   //!< Number of triangles summed over all leaf nodes
    int max_depth;       //!< Maximum leaf node depth
    int min_depth;       //!< Minimum leaf node depth
    int sum_depth;       //!< Sum of the depths of all leaf nodes
    int num_zero_leaves; //!< Number of leaf nodes with no triangles
    int tree_mem;        //!< Approximate amount of memory used by the tree
};

class KDBuilder {
private:

    // Note: We need a dynamic queue here because we don't know how deep we're going
    // to go while building the tree
    util::queue<KDBuilderQueueNode *> node_queue;        //!< Work queue
    std::mutex                        queue_lock;        //!< Work queue lock
    std::atomic_int                   outstanding_nodes; //!< Number of unfinished nodes

    /**
     * @brief KD-builder worker thread entrypoint
     */
    void worker_thread();

    /**
     * @brief Split triangles into two child bounding boxes
     *
     * @param[in]  split      Split plane location
     * @param[in]  dir        Split plane axis
     * @param[in]  triangles  Input triangles
     * @param[in]  planarMode How to treat triangles that lie in the split plane
     * @param[out] left       Triangles to the left of the split plane
     * @param[out] right      Trianlges to the right of the split plane
     */
    void partition(
        float                           split,
        int                             dir,
        enum KDBuilderPlanarMode      & planarMode,
        const std::vector<Triangle *> & triangles,
        std::vector<Triangle *>       & left,
        std::vector<Triangle *>       & right);

    /**
     * @brief Build a leaf node
     *
     * @param[in] q_node Queue node to process
     */
    void buildLeafNode(KDBuilderQueueNode *q_node);

    /**
     * @brief Build an inner node
     *
     * @param[in] q_node     Queue node to process
     * @param[in] split      Split plane location
     * @param[in] dir        Split plane axis
     * @param[in] planarMode How to treat triangles that lie in the split plane
     * @param[in] depth      Tree depth
     */
    void buildInnerNode(
        KDBuilderQueueNode       *q_node,
        float                     split,
        int                       dir,
        enum KDBuilderPlanarMode  planarMode,
        int                       depth);

    /**
     * @brief Build a KD node
     *
     * @param[in] threadCtx Worker thread context
     * @param[in] q_node    Queue node to process
     */
    void buildNode(void *threadCtx, KDBuilderQueueNode *q_node);

    /**
     * @brief Compute a bounding box for a set of triangles
     */
    AABB buildAABB(const std::vector<Triangle *> & triangles);

protected:

    /**
     * @brief Should be overriden by builder implementations to construct a thread context
     * storing whatever information they would like to maintain during construction.
     *
     * @param[in] idx Thread index
     *
     * @return Thread context object
     */
    virtual void *prepareWorkerThread(int idx);

    /**
     * @brief Should be overriden by builder implementations to destroy the thread context
     * object for a worker thread.
     *
     * @param[in] threadCtx Worker thread context object
     */
    virtual void destroyWorkerThread(void *threadCtx);

    /**
     * @brief Should be overriden by builder implementations to decide how split a KD-tree
     * node
     *
     * @param[in]  threadCtx  Worker thread context
     * @param[in]  bounds     Bounding box for input triangles
     * @param[in]  triangles  Triangle list to partition
     * @param[in]  depth      KD-tree node depth
     * @param[out] split      Split plane location
     * @param[out] dir        Split plane axis
     * @param[out] planarMode How to treat triangles lying in the split plane
     *
     * @return True if the node should be split, or false to create a leaf node
     */
    virtual bool splitNode(
        void                          * threadCtx,
        const AABB                    & bounds,
        const std::vector<Triangle *> & triangles,
        int                             depth,
        float                         & split,
        int                           & dir,
        enum KDBuilderPlanarMode      & planarMode) = 0;

public:

    /**
     * @brief Constructor
     */
    KDBuilder();

    /**
     * @brief Destructor
     */
    virtual ~KDBuilder();

    /**
     * @brief Build a KD tree for a list of triangles
     *
     * @param[in]  triangles List of input triangles
     * @param[out] stats     If non-null, will be filled with information about constructed tree
     *
     * @return KD tree for input triangles
     */
    KDTree *build(const std::vector<Triangle> & triangles, KDBuilderTreeStatistics *stats = nullptr);

};

#endif
