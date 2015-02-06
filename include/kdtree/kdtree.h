/**
 * @file kdtree.h
 *
 * @brief KD-Tree acceleration structure types and traversal code
 *
 * @author Sean James
 */

#ifndef __KDTREE_H
#define __KDTREE_H

#include <kdtree/kdnode.h>
#include <util/stack.h>

// TODO:
//     - Generic template type instead of requiring triangles only
//     - Photon map tree: nearest neighbor search and balanced tree
//     - Handle size overflow in leaves

/**
 * @brief KD-tree traversal stack item
 */
// TODO: Move this
struct KDStackFrame {
    /** @brief KD-Node to traverse */
    KDNode *node;

    /** @brief Entry distance from ray origin */
    float enter;

    /** @brief Exit distance from ray origin */
    float exit;
};

/**
 * @brief KD-Tree
 */
class KDTree {
private:

    KDNode *root;
    AABB    bounds;

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
    KDTree(KDNode *root, AABB bounds);

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
    bool intersect(util::stack<KDStackFrame> & stack, const Ray & ray, Collision & result, float maxDepth,
        bool anyCollision);

};

#endif
