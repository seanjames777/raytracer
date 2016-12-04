#ifndef __KDTREE_INL_H
#define __KDTREE_INL_H

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
#if 1
bool KDTree::intersect(THREAD KDStackFrame *stackMem, Ray ray, float tmax, THREAD Collision & result)
{
    // http://dcgi.felk.cvut.cz/home/havran/ARTICLES/cgf2011.pdf

	// TODO: use max to skip nodes, not just triangles
    
    util::stack<KDStackFrame> stack(stackMem);
    
    GLOBAL KDNode *currentNode;
    float entry, exit;

	result.distance = INFINITY;
	bool hit = false;

	// TODO: Can precompute inv_direction when ray is created, along with ray.direction[i] < 0
	float3 inv_direction = ray.invDirection();
    
    if (!bounds.intersects(ray.origin, inv_direction, entry, exit))
        return false;

	entry = max(entry, 0.0001f);
	exit = min(exit, tmax);

	if (entry > exit)
		return false;
    
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
            
            GLOBAL KDNode *nearNode = currentNode->left(&nodes[0]);
            GLOBAL KDNode *farNode = currentNode->right(&nodes[0]);
            
			if (ray.direction[type] < 0.0f) {
				GLOBAL KDNode *temp = nearNode;
				nearNode = farNode;
				farNode = temp;
			}

			// TODO: Avoid doing all the work for empty leaves
			if (t > exit || (exit < entry))
				currentNode = nearNode;
			else if (t < entry || (exit < entry))
				currentNode = farNode;
			else {
				stack.push(KDStackFrame(farNode, max(t, entry), exit));

				currentNode = nearNode;
				exit = min(t, exit);
			}

			// TODO: Significant cache miss here due to pulling node in from memory. Try prefetching or sorting rays by traversed nodes.
            type = currentNode->type();
        }
        
		// TODO: inlining this function may help
		hit = hit || intersects(
			ray,
			currentNode->triangles(&triangles[0]),
			currentNode->count,
			entry,
			exit,
			result);

		if (hit)
			return true;
    }
    
    return hit;
}
#elif 1
bool KDTree::intersect(THREAD KDStackFrame *stackMem, Ray ray, float tmax, THREAD Collision & result)
{
	Packet<1> packet;
	
	packet.origin[0][0] = ray.origin[0];
	packet.origin[1][0] = ray.origin[1];
	packet.origin[2][0] = ray.origin[2];

	packet.direction[0][0] = ray.direction[0];
	packet.direction[1][0] = ray.direction[1];
	packet.direction[2][0] = ray.direction[2];

	PacketCollision<1> _collision;

	vector<bmask, 1> hit = intersectPacket((KDPacketStackFrame<1> *)stackMem, packet, vector<float, 1>(tmax), _collision);

	result.beta = _collision.beta[0];
	result.gamma = _collision.gamma[0];
	result.distance = _collision.distance[0];
	result.triangle_id = _collision.triangle_id[0];

	return hit[0];
}
#else
bool KDTree::intersect(THREAD KDStackFrame *stackMem, Ray ray, float tmax, THREAD Collision & result)
{
	// http://dcgi.felk.cvut.cz/home/havran/ARTICLES/cgf2011.pdf

	// TODO: use max to skip nodes, not just triangles

	util::stack<KDStackFrame> stack(stackMem);

	GLOBAL KDNode *currentNode;
	float entry, exit;

	result.distance = INFINITY;

	// TODO: Can precompute inv_direction when ray is created, along with ray.direction[i] < 0
	float3 inv_direction = ray.invDirection();

	if (!bounds.intersects(ray.origin, inv_direction, entry, exit))
		return false;

	entry = max(entry, 0.0001f);
	exit = min(exit, tmax);

	if (entry > exit)
		return false;

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

			if (ray.direction[type] < 0.0f) {
				GLOBAL KDNode *temp = nearNode;
				nearNode = farNode;
				farNode = temp;
			}

			// TODO: Avoid doing all the work for empty leaves
			if (t > exit)
				currentNode = nearNode;
			else if (t < entry)
				currentNode = farNode;
			else {
				stack.push(KDStackFrame(farNode, t, exit));

				currentNode = nearNode;
				exit = t;
			}

			// TODO: Significant cache miss here due to pulling node in from memory. Try prefetching or sorting rays by traversed nodes.
			type = currentNode->type();
		}

		// TODO: inlining this function may help
		if (intersects(
			ray,
			currentNode->triangles(triangles),
			currentNode->count,
			entry,
			exit,
			result))
		{
			return true;
		}
}

	return false;
}
#endif

template<unsigned int N>
vector<bmask, N> KDTree::intersectPacket(
	THREAD KDPacketStackFrame<N> *stackMem,
	THREAD const vector<float, N> (&origin)[3],
	THREAD const vector<float, N> (&direction)[3],
	THREAD const vector<float, N> & maxDist,
	bool occlusionOnly,
	THREAD PacketCollision<N> & result)
{
	// http://dcgi.felk.cvut.cz/home/havran/ARTICLES/cgf2011.pdf

	// TODO: use max to skip nodes, not just triangles

	util::stack<KDPacketStackFrame<N>> stack(stackMem);

	GLOBAL KDNode *currentNode;
	vector<float, N> entry, exit;

	result.distance = INFINITY;
	vector<bmask, N> hit = vector<bmask, N>(false);

	// TODO: Can precompute inv_direction when ray is created, along with ray.direction[i] < 0
	vector<float, N> inv_direction[3] = {
		vector<float, N>(1.0f) / direction[0],
		vector<float, N>(1.0f) / direction[1],
		vector<float, N>(1.0f) / direction[2]
	};

	if (!any(bounds.intersectsPacket(origin, inv_direction, entry, exit)))
		return vector<bmask, N>(0x00000000);

	entry = max(entry, vector<float, N>(0.0001f));
	exit = min(exit, maxDist);

	if (all(entry > exit))
		return vector<bmask, N>(0x00000000);

	stack.push(KDPacketStackFrame<N>(root, entry, exit));

	while (!stack.empty()) {
		KDPacketStackFrame<N> curr_stack = stack.pop();

		currentNode = curr_stack.node;
		entry = curr_stack.enter;
		exit = curr_stack.exit;

		uint32_t type = currentNode->type();

		while (type != KD_LEAF) {
			vector<float, N> split = currentNode->split_dist;

			vector<float, N> t = (split - origin[type]) * inv_direction[type];

			GLOBAL KDNode *nearNode = currentNode->left(&nodes[0]);
			GLOBAL KDNode *farNode = currentNode->right(&nodes[0]);

			if (direction[type][0] < 0.0f) {
				GLOBAL KDNode *temp = nearNode;
				nearNode = farNode;
				farNode = temp;
			}

			vector<bmask, 4> inactive = exit < entry;

			// TODO: Avoid doing all the work for empty leaves
			if (all((t > exit) | inactive))
				currentNode = nearNode;
			else if (all((t < entry) | inactive))
				currentNode = farNode;
			else {
				stack.push(KDPacketStackFrame<N>(farNode, max(t, entry), exit));

				currentNode = nearNode;
				exit = min(t, exit);
			}

			// TODO: Significant cache miss here due to pulling node in from memory. Try prefetching or sorting rays by traversed nodes.
			type = currentNode->type();
		}

		// TODO: inlining this function may help
		// Note: some rays may not have wanted to traverse this branch because they would not have hit anything. Therefore,
		// there is no need to mask out the inactive rays' hit results.
		hit = hit | intersectsPacket(
			origin,
			direction,
			currentNode->triangles(&triangles[0]),
			currentNode->count,
			entry,
			exit,
			occlusionOnly,
			result);

		// TODO: If a ray has hit something, should we invalidate it so it doesn't impact future branching tests?

		// We traverse nodes in near to far order, so if all rays have hit something there won't be anything closer
		if (all(hit))
			return vector<bmask, N>(0xFFFFFFFF);
	}

	return hit;
}

template vector<bmask, SIMD> KDTree::intersectPacket(
	THREAD KDPacketStackFrame<SIMD> *stackMem,
	THREAD const vector<float, SIMD> (&origin)[3],
	THREAD const vector<float, SIMD> (&direction)[3],
	THREAD const vector<float, SIMD> & maxDist,
	bool occlusionOnly, // TODO: could templatize
	THREAD PacketCollision<SIMD> & result);

#endif
