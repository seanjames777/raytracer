/**
 * @file kdtree.h
 *
 * @brief KD-Tree
 *
 * @author Sean James
 */

#ifndef _KDTREE_H
#define _KDTREE_H

#include <rtmath.h>
#include <polygon.h>

// TODO:
//     - Photon tree
//     - Mean -> Median?
//     - SAH

/**
 * @brief KD-Tree
 */
class KDTree {
private:

    enum KDNodeFlags {
        // If the node is a leaf, the bottom 31 bits are the size and the top
        // bit is 1. Otherwise, the node is not a leaf, so the top bit is 0
        // and the bottom two bits are the split direction.

        KD_SIZE_MASK       = ~(1 << 31),
        KD_SPLIT_DIR_X     =   0,
        KD_SPLIT_DIR_Y     =   1,
        KD_SPLIT_DIR_Z     =   2,
        KD_SPLIT_DIR_MASK  =   3,
        KD_IS_LEAF         =   1 << 31,
    };

    // TODO cache alignment, maybe compact, and child node heap?
    struct KDNode {
        // 32 bytes = 1/2 cache line

        KDNode        *left;
        KDNode        *right;
        SetupTriangle *triangles;
        float          split_dist;
        unsigned int   flags;

        KDNode(KDNode *left, KDNode *right, float split_dist,
            SetupTriangle *triangles, unsigned int flags);

        ~KDNode();

    };

    /**
     * @brief KD-tree traversal stack item
     */
    struct KDStackFrame {
        /** @brief KD-Node to traverse */
        KDNode *node;

        /** @brief Entry distance from ray origin */
        float enter;

        /** @brief Exit distance from ray origin */
        float exit;
    };

    KDNode *root;
    AABB    sceneBounds;

    /**
     * @brief Find the subset of triangles contained in a bounding box
     *
     * @param box       Bounding box
     * @param triangles     Items to check
     * @param contained Vector to fill with triangles overlapping box
     */
    void partition(const AABB & box, const std::vector<Triangle *> & triangles,
        std::vector<Triangle *> & contained);

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
    KDNode *buildMean(AABB bounds, const std::vector<Triangle *> & triangles, int depth);

    /**
     * @brief Compute a bounding box for a set of triangles
     *
     * @param item Items to bound
     */
    AABB buildAABB(const std::vector<Triangle *> & triangles);

    /**
     * @brief Check for collision with a leaf node
     *
     * @param leaf         Leaf node to check
     * @param ray          Ray to check for collision against
     * @param result       Output collision information
     * @param entry        Minimum collision distance
     * @param exit         Maximum collision distance
     * @param anyCollision Whether to accept any collision or to find the closest
     */
    bool intersectLeaf(KDNode *leaf, const Ray & ray, Collision & result, float entry, float exit,
        bool anyCollision);

public:

    /**
     * @brief Constructor
     *
     * @param triangles Items to contain in the tree
     */
    KDTree(const std::vector<Triangle> & triangles);

    /**
     * @brief Intersect a ray against the KD-Tree
     *
     * @param ray          Ray to test against
     * @param result       Will be filled with collision information
     * @param maxDepth     Maximum intersection distance, or 0 for any distance
     * @param anyCollision Whether to stop after any collision or to find the closest collision
     *
     * @return Whether a collision occured
     */
    bool intersect(const Ray & ray, Collision & result, float maxDepth, bool anyCollision);

};

#endif
