/**
 * @file kdbuilder.h
 *
 * @brief KD-Tree builder base class
 *
 * @author Sean James
 */

#ifndef _KDBUILDER_H
#define _KDBUILDER_H

#include <kdtree/kdtree.h>
#include <util/queue.h>

// How to handle triangles that lie in the plane
enum PlanarMode {
    PLANAR_LEFT =  1,
    PLANAR_RIGHT = 2,
    PLANAR_BOTH =  3
};

class KDBuilderQueueNode {
    AABB bounds;
    std::vector<Triangle *> triangles; // TODO: memcpy/malloc instead
    int depth;

    // TODO: false sharing
};

class KDBuilder {
protected:

    // TODO: Pool or something of queue nodes might be better than new/delete constantly.
    // Note: We need a dynamic queue here because we don't know how deep we're going
    // to go while building the tree, and we need more storage space closer to the
    // leaves.
    Queue<KDBuilderQueueNode *> node_queue;

    /**
     * @brief Find the subset of triangles contained in a bounding box
     *
     * @param box       Bounding box
     * @param triangles     Items to check
     * @param contained Vector to fill with triangles overlapping box
     */
    void partition(float dist, int dir, const std::vector<Triangle *> & triangles,
        std::vector<Triangle *> & left, std::vector<Triangle *> & right,
        enum PlanarMode & planarMode);

    /**
     * @brief Build a leaf node
     *
     * @param triangles Items to place in leaf node
     */
    KDNode *buildLeaf(const std::vector<Triangle *> & triangles);

    /**
     * @brief Build a KD tree containing the given triangles
     *
     * @param bounds Bounding box containing all triangles
     * @param triangles  Items to place in tree
     * @param dir    Direction to split root
     * @param depth  Recursion depth
     */
    KDNode *buildNode(const AABB & bounds, const std::vector<Triangle *> & triangles, int depth);

    /**
     * @brief Compute a bounding box for a set of triangles
     *
     * @param item Items to bound
     */
    AABB buildAABB(const std::vector<Triangle *> & triangles);

    virtual bool splitNode(const AABB & bounds, const std::vector<Triangle *> & triangles,
        int depth, int & dir, float & split, enum PlanarMode & planarMode) = 0;

public:

    KDBuilder();

    virtual ~KDBuilder();

    KDTree *build(const std::vector<Triangle> & triangles);

};

#endif
