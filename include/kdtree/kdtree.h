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
#include <core/triangle.h>

// TODO:
//     - Generic template type instead of requiring triangles only
//     - Photon map tree: nearest neighbor search and balanced tree
//     - Handle size overflow in leaves

#if GPU
/**
 * @brief KD-tree traversal stack frame
 */
struct KDStackFrame {
    device KDNode *node; //!< KD-node to traverse
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
    KDStackFrame(device KDNode *node, float enter, float exit)
        : node(node),
          enter(enter),
          exit(exit)
    {
    }
};
#endif

/**
 * @brief KD-Tree acceleration structure
 */
class KDTree {
private:

#if GPU
    device KDNode *root;   //!< Root node
    device KDNode *nodes;
    device SetupTriangle *triangles;
#else
    KDNode *root;
#endif
    
    AABB    _bounds; //!< Bounding box for entire tree

public:

    /**
     * @brief Constructor
     *
     * @param[in] root   Root node
     * @param[in] bounds Bounding box for entire tree
     */
#if GPU
    KDTree(device KDNode *root, device KDNode *nodes, device SetupTriangle *triangles, AABB bounds)
#else
    KDTree(KDNode *root, AABB bounds)
#endif
        : root(root),
#if GPU
          nodes(nodes),
          triangles(triangles),
#endif
          _bounds(bounds)
    {
    }
    
    AABB bounds() {
        return _bounds;
    }

#if GPU
    /**
     * @brief Intersect a ray against the KD-Tree
     *
     * @param[in] stack  Reusable traversal stack
     * @param[in] ray    Ray to test
     * @param[in] result Information about collision, if there is one
     *
     * @return True if there is a collision, or false if there is not
     */
    bool intersect(thread KDStackFrame *stack, Ray ray, bool anyCollision, thread Collision & result);
#endif

};

// TODO: Can do 2, 4, 8, etc. at a time with SSE. Need to transpose to SOA
// TODO: SSE has a min and bit scan
// TODO: Might want to inline triangle code
// TODO: Shorter triangle code might be faser if it stores less
// TODO: KD heuristic could prefer larger leaves if we use SSE -> better
//       caching and less memory. Heuristic could prefer leaves that
//       are a multiple of SSE line size
// TODO: Search back to front along split plane assuming triangles are
//       sorted within KD node. Early reject triangles outside running
//       min and max distance, backfacing triangles.
// TODO: Intersection-test-only mode to avoid computing barycentric? probably not
// TODO: Might be faster to store less data and compute a bit more in triangle test
// TODO: Cache align KD nodes, maybe other stuff: avoids false sharing during
//       construction, allows clean caching at runtime
// TODO: Group KD nodes in memory by some criteria. Probably want to keep top of
//       tree in cache, allow locality to determine bottom portion to keep in
//       cache. Might be able to micromanage this? Want to keep child nodes
//       near parent or near each other or something. We do a depth first
//       traversal on one thread.
// TODO: Clone TBB's work queue for construction
// TODO: Mailboxing is a thing. Figure out what it is.
// TODO: Is this actually depth first? Make sure. And make sure we want that.
// TODO: Might want a tree for light extents
// TODO: Maybe help the heuristic with creating big empty gaps? Something about this
//       in the SAH paper. Creating empty nodes or whatever.
// TODO: Tweak heursitic constants
#if GPU
bool KDTree::intersect(thread KDStackFrame *stackMem, Ray ray, bool anyCollision, thread Collision & result)
{
    // http://dcgi.felk.cvut.cz/home/havran/ARTICLES/cgf2011.pdf
    
    util::stack<KDStackFrame> stack(stackMem);
    
    device KDNode *currentNode;
    float entry, exit;
    
    if (!_bounds.intersects(ray, entry, exit))
        return false;
    
    float3 inv_direction = ray.invDirection();
    
    stack.push(KDStackFrame(root, entry, exit));
    
    while (!stack.empty()) {
        KDStackFrame curr_stack = stack.pop();
        
        currentNode = curr_stack.node;
        entry = curr_stack.enter;
        exit = curr_stack.exit;
        
        uint32_t type = currentNode->type();
        
        while (type != KD_LEAF) {
            float split = currentNode->split_dist;
            float origin = ray.origin[type];
            
            float t = (split - origin) * inv_direction[type];
            
            device KDNode *nearNode = currentNode->left(nodes);
            device KDNode *farNode  = currentNode->right(nodes);
            
            if (split < origin) {
                device KDNode *temp = nearNode;
                nearNode = farNode;
                farNode = temp;
            }
            
            // TODO: Avoid doing all the work for empty leaves
            if (t > exit || t < 0)
                currentNode = nearNode;
            else if (t < entry)
                currentNode = farNode;
            else {
                stack.push(KDStackFrame(farNode, t, exit));
                currentNode = nearNode;
                exit = t;
            }
            
            type = currentNode->type();
        }
        
        // TODO: Used to have tmpResult.distance >= entry && tmpResult.distance <= exit
        // TODO: Ignores max depth
        
        if (intersects(ray, currentNode->triangles(triangles),
                                            currentNode->count,
                       anyCollision,
                       entry,
                        exit, // TODO
                        result))
        {
            return true;
        }
    }
    
    return false;
}
#endif

#endif
