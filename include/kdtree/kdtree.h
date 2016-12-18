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
    const GLOBAL KDNode *node; //!< KD-node to traverse
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
    KDStackFrame(const GLOBAL KDNode *node, float enter, float exit)
        : node(node),
          enter(enter),
          exit(exit)
    {
    }
};

template<unsigned int N>
struct KDPacketStackFrame {
	const GLOBAL KDNode *node;
	vector<float, N> enter;
	vector<float, N> exit;
	
	KDPacketStackFrame() {
	}

	KDPacketStackFrame(const GLOBAL KDNode *node, vector<float, N> enter, vector<float, N> exit)
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
public:

    KDNode                          *root;
    util::vector<KDNode, 8>          nodes;
    util::vector<SetupTriangle, 16>  triangles;
    AABB                             bounds;

    /**
     * @brief Intersect a ray against the KD-Tree
     *
     * @param[in] stack  Reusable traversal stack
     * @param[in] ray    Ray to test
     * @param[in] result Information about collision, if there is one
     *
     * @return True if there is a collision, or false if there is not
     */
    bool intersect(const Ray & ray, float max, THREAD Collision & result) const;

	template<unsigned int N>
	vector<bmask, N> intersectPacket(
		THREAD const vector<float, N> (&origin)[3],
		THREAD const vector<float, N> (&direction)[3],
		THREAD const vector<float, N> & maxDist,
		bool occlusionOnly,
		THREAD PacketCollision<N> & result) const;

};

#endif
