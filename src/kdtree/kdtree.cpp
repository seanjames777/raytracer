/**
 * @file kdtree.cpp
 *
 * @author Sean James
 */

#include <kdtree/kdtree.h>

KDTree::KDTree(KDNode *root, AABB bounds)
    : root(root),
      bounds(bounds)
{
}

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
bool KDTree::intersectLeaf(KDNode *leaf, const Ray & ray, Collision & result, float entry, float exit,
    bool anyCollision)
{
	// TODO: Might want to inline this, though intersects() is already inlined
	// TODO: Used to have tmpResult.distance >= entry && tmpResult.distance <= exit
	return leaf->triangles->intersects(ray, leaf->flags & KD_SIZE_MASK, anyCollision, result);
}

bool KDTree::intersect(util::stack<KDStackFrame> & stack, const Ray & ray, Collision & result, float maxDepth,
    bool anyCollision)
{
    float entry, exit;

    if (!bounds.intersects(ray, &entry, &exit))
        return false;

    KDStackFrame curr_stack;
    curr_stack.node = root;
    curr_stack.enter = entry;
    curr_stack.exit = exit;
    stack.push(curr_stack);

    KDNode *currentNode;

    while (!stack.empty()) {
        curr_stack = stack.pop(); // TODO: copy

        currentNode = curr_stack.node;
        entry = curr_stack.enter;
        exit = curr_stack.exit;

        // Nothing will be closer, give up
        if (maxDepth > 0.0f && entry > maxDepth) {
            stack.clear();
            return false;
        }

        while (!(currentNode->flags & KD_IS_LEAF)) {
            int dir = currentNode->flags & KD_SPLIT_DIR_MASK;

            float split = currentNode->split_dist;
            float origin = ray.origin.v[dir];

            float t = (split - origin) * ray.inv_direction.v[dir];

            KDNode *nearNode = currentNode->left;
            KDNode *farNode  = currentNode->right;

            if (split < origin) {
                KDNode *temp = nearNode;
                nearNode = farNode;
                farNode = temp;
            }

            if (t > exit || t < 0)
                currentNode = nearNode;
            else if (t < entry)
                currentNode = farNode;
            else {
                curr_stack.node = farNode;
                curr_stack.enter = t;
                curr_stack.exit = exit;

                stack.push(curr_stack); // TODO: copy

                currentNode = nearNode;
                exit = t;
            }
        }

        // Again, nothing will be closer so we're done
        if (intersectLeaf(currentNode, ray, result, entry, maxDepth > 0.0f ?
            fminf(maxDepth, exit) : exit, anyCollision))
        {
            stack.clear();
            return true;
        }
    }

    return false;
}
