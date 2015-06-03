/**
 * @file kdtree/kdtree.h
 *
 * @brief KD-Tree acceleration structure
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __KDTREE_H
#define __KDTREE_H

#include <kdtree/kdnode.h>
#include <math/aabb.h>
#include <util/stack.h>

// TODO:
//     - Generic template type instead of requiring triangles only
//     - Photon map tree: nearest neighbor search and balanced tree
//     - Handle size overflow in leaves

/**
 * @brief KD-tree traversal stack frame
 */
struct KDStackFrame {
    KDNode *node; //!< KD-node to traverse
    float enter;  //!< Distance from ray origin to bounding box entry point
    float exit;   //!< Distance from ray origin to bounding box exit point

    /**
     * @brief Constructor
     *
     * @param[in] node  KD-node to traverse
     * @param[in] enter Distance from ray origin to bounding box entry point
     * @param[in] exit  Distance from ray origin to bounding box exit point
     */
    KDStackFrame(KDNode *node, float enter, float exit)
        : node(node),
          enter(enter),
          exit(exit)
    {
    }
};

/**
 * @brief KD-Tree acceleration structure
 */
class KDTree {
private:

    KDNode *root;   //!< Root node
    AABB    bounds; //!< Bounding box for entire tree

public:

    /**
     * @brief Constructor
     *
     * @param[in] root   Root node
     * @param[in] bounds Bounding box for entire tree
     */
    KDTree(KDNode *root, AABB bounds);

    /**
     * @brief Destructor
     */
    ~KDTree();

    /**
     * @brief Intersect a ray against the KD-Tree
     *
     * @param[in] stack  Reusable traversal stack
     * @param[in] ray    Ray to test
     * @param[in] result Information about collision, if there is one
     *
     * @return True if there is a collision, or false if there is not
     */
    bool intersect(util::stack<KDStackFrame> & stack, const Ray & ray, Collision & result);

};

#endif
