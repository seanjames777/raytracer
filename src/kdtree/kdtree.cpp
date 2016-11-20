/**
 * @file kdtree/kdtree.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <kdtree/kdtree.h>

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
bool KDTree::intersect(THREAD KDStackFrame *stackMem, Ray ray, bool anyCollision, THREAD Collision & result)
{
	// http://dcgi.felk.cvut.cz/home/havran/ARTICLES/cgf2011.pdf

	util::stack<KDStackFrame> stack(stackMem);

	GLOBAL KDNode *currentNode;
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

			GLOBAL KDNode *nearNode = currentNode->left(nodes);
			GLOBAL KDNode *farNode = currentNode->right(nodes);

			if (split < origin) {
				GLOBAL KDNode *temp = nearNode;
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