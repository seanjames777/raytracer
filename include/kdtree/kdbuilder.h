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
#include <util/vector.h>

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
struct KDBuilderNode {
    uint32_t                    depth;     //!< Node depth in KD-tree
    AABB                        bounds;    //!< Bounds of all triangles
    util::vector<Triangle, 16>  triangles; //!< Triangles to place in node
    KDBuilderNode              *left;
    KDBuilderNode              *right;
    int                         dir;
    float                       split;

    KDBuilderNode()
        : depth(0),
          left(nullptr),
          right(nullptr),
          dir(0),
          split(0.0f)
    {
    }
};

/**
 * @brief Statistics about a KD-tree
 */
struct KDTreeStats {
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

template<typename T>
class KDBuilder {
private:

    // Note: We need a dynamic queue here because we don't know how deep we're going
    // to go while building the tree
    util::queue<KDBuilderNode *, 8> node_queue;
    std::mutex                      queue_lock;        //!< Work queue lock
    std::atomic_int                 outstanding_nodes; //!< Number of unfinished nodes

    KDTree                          & tree;
    util::vector<Triangle, 16>      & triangles;
    uint32_t                          numUnclippedTriangles;
    std::atomic_int                   triangleID;

    /**
     * @brief KD-builder worker THREAD entrypoint
     */
    void builder_thread();

    void partition(
        float                              split,
        int                                dir,
        enum KDBuilderPlanarMode         & planarMode,
        const util::vector<Triangle, 16> & triangles,
        util::vector<Triangle, 16>       & left,
        util::vector<Triangle, 16>       & right);

    void splitNode(
        KDBuilderNode            & builderNode,
        float                      split,
        int                        dir,
        enum KDBuilderPlanarMode   planarMode);

    void finalizeLeafNode(
        const KDBuilderNode             & builderNode,
        KDNode                          & node);

    void finalizeInnerNode(
        const KDBuilderNode             & builderNode,
        KDNode                          & node);

    void finalizeNode(
        const KDBuilderNode             & builderNode,
        KDNode                          & node);

    void buildNode(
        T & threadCtx,
        KDBuilderNode & builderNode);

    AABB buildAABB(const util::vector<Triangle, 16> & triangles);

protected:

    /**
     * @brief Should be overriden by builder implementations to decide how split a KD-tree
     * node
     *
     * @param[in]  threadCtx  Worker THREAD context
     * @param[in]  bounds     Bounding box for input triangles
     * @param[in]  triangles  Triangle list to partition
     * @param[in]  depth      KD-tree node depth
     * @param[out] split      Split plane location
     * @param[out] dir        Split plane axis
     * @param[out] planarMode How to treat triangles lying in the split plane
     *
     * @return True if the node should be split, or false to create a leaf node
     */
    virtual bool shouldSplitNode(
        T                                & threadCtx,
        const AABB                       & bounds,
        const util::vector<Triangle, 16> & triangles,
        int                                depth,
        float                            & split,
        int                              & dir,
        enum KDBuilderPlanarMode         & planarMode) = 0;
    
    void computeStats(KDNode *root, KDTreeStats *stats, int depth);

public:

    KDBuilder(KDTree & tree, util::vector<Triangle, 16> & triangles);

    virtual ~KDBuilder();

    void build(KDTreeStats *stats = nullptr);

};

#endif
