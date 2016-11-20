/**
 * @file kdtree/kdtree.h
 *
 * @brief KD-Tree acceleration structure
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __KDTREE_H
#define __KDTREE_H

#include <rt_defs.h>

#include <kdtree/kdnode.h>
#include <math/aabb.h>
#include <util/stack.h>
#include <core/triangle.h>

// TODO:
//     - Generic template type instead of requiring triangles only
//     - Photon map tree: nearest neighbor search and balanced tree
//     - Handle size overflow in leaves

/**
 * @brief KD-tree traversal stack frame
 */
struct KDStackFrame {
    GLOBAL KDNode *node; //!< KD-node to traverse
    float enter;  //!< Distance from ray origin to bounding box entry point
    float exit;   //!< Distance from ray origin to bounding box exit point
    
    KDStackFrame() {
    }

    /**
     * @brief Constructor
     *
     * @param[in] node  KD-node to traverse
     * @param[in] enter Distance from ray origin to bounding box entry point
     * @param[in] exit  Distance from ray origin to bounding box exit point
     */
    KDStackFrame(GLOBAL KDNode *node, float enter, float exit)
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

    GLOBAL KDNode *root;   //!< Root node
    GLOBAL KDNode *nodes;
    GLOBAL SetupTriangle *triangles;    
    AABB    _bounds; //!< Bounding box for entire tree

public:

    /**
     * @brief Constructor
     *
     * @param[in] root   Root node
     * @param[in] bounds Bounding box for entire tree
     */

    KDTree(GLOBAL KDNode *root, GLOBAL KDNode *nodes, GLOBAL SetupTriangle *triangles, AABB bounds)
		: root(root),
		  nodes(nodes),
		  triangles(triangles),
		  _bounds(bounds)
	{
	}

    AABB bounds() {
        return _bounds;
    }

    /**
     * @brief Intersect a ray against the KD-Tree
     *
     * @param[in] stack  Reusable traversal stack
     * @param[in] ray    Ray to test
     * @param[in] result Information about collision, if there is one
     *
     * @return True if there is a collision, or false if there is not
     */
    bool intersect(THREAD KDStackFrame *stack, Ray ray, bool anyCollision, THREAD Collision & result);

};

#endif
