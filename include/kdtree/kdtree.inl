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
bool KDTree::intersect(THREAD KDStackFrame *stackMem, Ray ray, bool anyCollision, float tmax, THREAD Collision & result)
{
    // http://dcgi.felk.cvut.cz/home/havran/ARTICLES/cgf2011.pdf

	// TODO: use max to skip nodes, not just triangles
    
    util::stack<KDStackFrame> stack(stackMem);
    
    GLOBAL KDNode *currentNode;
    float entry, exit;
    
    if (!_bounds.intersects(ray, entry, exit))
        return false;

	entry = max(entry, 0.0001f);
	exit = min(exit, tmax);

	if (entry > exit)
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
            
			// TODO: Significant cache miss here due to pulling node in from memory. Try prefetching or sorting rays by traversed nodes.
            type = currentNode->type();
        }
        
        // TODO: Used to have tmpResult.distance >= entry && tmpResult.distance <= exit
        // TODO: Ignores max depth
        
		// TODO: inlining this function may help
        if (intersects(
				ray,
				currentNode->triangles(triangles),
                currentNode->count,
                anyCollision,
                entry,
                exit,
                result))
        {
            return true;
        }
    }
    
    return false;
}

template<typename T, unsigned int N>
T any(vector<T, N> v) {
	for (unsigned int i = 0; i < N; i++)
		if (v[i])
			return true;
	
	return false;
}

template<typename T, unsigned int N>
T all(vector<T, N> v) {
	for (unsigned int i = 0; i < N; i++)
		if (!v[i])
			return false;

	return true;
}

template<typename T, unsigned int N>
vector<bool, N> operator<(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] < rhs[i];

	return out;
}

template<typename T, unsigned int N>
vector<bool, N> operator<=(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] <= rhs[i];

	return out;
}

template<typename T, unsigned int N>
vector<bool, N> operator==(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] == rhs[i];

	return out;
}

template<typename T, unsigned int N>
vector<bool, N> operator>=(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] >= rhs[i];

	return out;
}

template<typename T, unsigned int N>
vector<bool, N> operator>(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] > rhs[i];

	return out;
}

template<typename T, unsigned int N>
vector<bool, N> operator!=(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] != rhs[i];

	return out;
}

template<typename T, unsigned int N>
vector<bool, N> operator&&(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] && rhs[i];

	return out;
}

template<typename T, unsigned int N>
vector<bool, N> operator||(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] || rhs[i];

	return out;
}

struct Packet {
	vector<float, 4> origin[3];
	vector<float, 4> direction[3];
};

struct KDPacketStackFrame {
	GLOBAL KDNode *node;
	vector<float, 4> enter;
	vector<float, 4> exit;
	vector<bool, 4> active;

	KDPacketStackFrame() {
	}

	KDPacketStackFrame(GLOBAL KDNode *node, vector<float, 4> enter, vector<float, 4> exit, vector<bool, 4> active)
		: node(node),
		  enter(enter),
		  exit(exit),
		  active(active)
	{
	}
};

#if 0
void KDTree::intersectPacket(THREAD KDPacketStackFrame *stackMem, const Packet & packet, bool anyCollision, vector<float, 4> max, vector<bool, 4> & collision, THREAD Collision (&result)[4])
{
	// http://dcgi.felk.cvut.cz/home/havran/ARTICLES/cgf2011.pdf

	// TODO: use max to skip nodes, not just triangles

	util::stack<KDPacketStackFrame> stack(stackMem);

	GLOBAL KDNode *currentNode;
	vector<float, 4> entry, exit;

	vector<bool, 4> active(true);

	for (int i = 0; i < 4; i++)
		active[i] = _bounds.intersectPacket(packet, entry, exit) && entry <= max;

	if (!any(active)) {
		collision = false;
		return;
	}

	vector<float, 4> inv_direction[3] = {
		1.0f / packet.direction[0],
		1.0f / packet.direction[1],
		1.0f / packet.direction[2]
	};

	stack.push(KDPacketStackFrame(root, entry, exit, active));

	while (!stack.empty()) {
		KDPacketStackFrame curr_stack = stack.pop();

		currentNode = curr_stack.node;
		entry = curr_stack.enter;
		exit = curr_stack.exit;

		uint32_t type = currentNode->type();

		while (type != KD_LEAF) {
			vector<float, 4> split = currentNode->split_dist; // Note: Broadcast

			vector<float, 4> origin = packet.origin[type];
			vector<float, 4> t = (split - origin) * inv_direction[type];

			GLOBAL KDNode *nearNode = currentNode->left(nodes);
			GLOBAL KDNode *farNode = currentNode->right(nodes);

			if (split < origin) {
				GLOBAL KDNode *temp = nearNode;
				nearNode = farNode;
				farNode = temp;
			}

			vector<bool, 4> near = (t > exit || t < vector<float, 4>(0.0f)) && active;
			vector<bool, 4> far = (t < entry) && active;

			// TODO: Avoid doing all the work for empty leaves
			if (t > exit || t < vector<float, 4>(0.0f))
				currentNode = nearNode;
			else if (t < entry)
				currentNode = farNode;
			else {
				if (t < max)
					stack.push(KDStackFrame(farNode, t, exit));
				currentNode = nearNode;
				exit = t;
			}

			// TODO: Significant cache miss here due to pulling node in from memory. Try prefetching or sorting rays by traversed nodes.
			type = currentNode->type();
		}

		// TODO: Used to have tmpResult.distance >= entry && tmpResult.distance <= exit
		// TODO: Ignores max depth

		// TODO: inlining this function may help
		if (intersects(ray, currentNode->triangles(triangles),
			currentNode->count,
			anyCollision,
			entry,
			min(exit, max), // TODO
			result))
		{
			return true;
		}
	}

	return false;
}
#endif

#endif
