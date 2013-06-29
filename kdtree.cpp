/*
 * Sean James
 *
 * kdtree.cpp
 *
 * KD-Tree
 *
 */

#include "kdtree.h"

/*
 * Get the subset and size of the subset of items whose bounding boxes
 * intersect contained
 */
int KDTree::countInBox(const AABB & box, vector<KDContainable *> & items, vector<KDContainable *> & contained) {
	int count = 0;

	vector<KDContainable *>::iterator it;
	for (it = items.begin(); it != items.end(); it++)
		if ((*it)->containedIn(box)) {
			contained.push_back(*it);
			count++;
		}

	return count;
}

/*
 * Determine the split distance for the given bounding box and direction
 */
float KDTree::splitDist(const AABB & bounds, int dir) {
	Vec3 max = bounds.max, min = bounds.min;
	Vec3 ext = max - min;

	return ext.components[dir] / 2.0f + bounds.min.components[dir];
}

/*
 * Check every item in a leaf node for intersection against a given ray
 */
bool KDTree::intersectLeaf(KDNode *leaf, const Ray & ray, CollisionResult *result, KDThreadState *thread, float enter, float exit) {
	result->shape = NULL;

	CollisionResult tmpResult;

	for (int i = 0; i < leaf->nItems; i++) {
		KDContainable *KDContainable = leaf->items[i];

		thread->numRaysCast++;

		if (KDContainable->intersects(ray, &tmpResult)) {
			if (tmpResult.distance < enter || tmpResult.distance > exit)
				continue;
			else if (result->shape == NULL || tmpResult.distance < result->distance)
				*result = tmpResult;
		}
	}

	return result->shape != NULL;
}

/*
 * Check every item in a leaf node for intersection against a given ray
 */
bool KDTree::intersectLeaf(KDNode *leaf, const Ray & ray, KDThreadState *thread, float enter, float exit) {
	float tmp_closestDist;

	for (int i = 0; i < leaf->nItems; i++) {
		KDContainable *KDContainable = leaf->items[i];

		thread->numRaysCast++;

		if (KDContainable->intersects(ray, &tmp_closestDist)) {
			if (tmp_closestDist >= enter && tmp_closestDist <= exit)
				return true;
		}
	}
	
	return false;
}

/*
 * Build a KD tree containing the given items
 */
KDNode *KDTree::build(const AABB & bounds, vector<KDContainable *> & items, int dir, int depth) {
	KDNode *node = new KDNode(NULL, NULL, 0.0f, 0, id++);

	if (depth > 4 || items.size() < 4) {
		// leaf if
		// 1) too few items
		// 2) too deep
		// 3) dividing did not change the # items


		node->nItems = items.size();
		node->items = new KDContainable *[node->nItems];

		int i = 0;

		vector<KDContainable *>::iterator it;
		for (it = items.begin(); it != items.end(); it++)
			node->items[i++] = *it;
	}
	else {
		// not a leaf

		node->split = splitDist(bounds, dir);
		node->dir = dir;

		AABB leftBB = node->getBBoxL(bounds);
		AABB rightBB = node->getBBoxR(bounds);

		// TODO init with parent size?
		vector<KDContainable *> leftItems;
		vector<KDContainable *> rightItems;

		int countL = countInBox(leftBB, items, leftItems);
		int countR = countInBox(rightBB, items, rightItems);

		KDNode *left  = build(leftBB, leftItems,  (dir + 1) % 3, depth + 1);
		KDNode *right = build(rightBB, rightItems, (dir + 1) % 3, depth + 1);

		node->left  = left;
		node->right = right;
	}

	return node;
}

/*
 * Locate or assign a thread state for the calling thread
 */
KDThreadState *KDTree::getMyThreadState() {
	pthread_t self = thread_self();

	for (int i = 0; i < numThreads; i++) {
		if (threadStates[i].initialized && pthread_equal(self, threadStates[i].thread))
			return &threadStates[i];
	}

	threadStatesInitLock.acquire();

	KDThreadState *ret = NULL;

	for (int i = 0; i < numThreads; i++) {
		if (!threadStates[i].initialized) {
			threadStates[i].thread = self;
			threadStates[i].initialized = true;
			ret = &threadStates[i];
			break;
		}
	}

	threadStatesInitLock.release();

	return ret;
}

/*
 * Default constructor builds a KD tree from the given items and bounds
 * allowing access by up to nThreads threads
 */
KDTree::KDTree(vector<KDContainable *> items, const AABB & itemsBounds, int nThreads) 
	: numThreads(nThreads),
	  sceneBounds(itemsBounds),
	  id(0)
{
	threadStates = new KDThreadState[nThreads];

	Timer timer;
	timer.start();

	root = build(itemsBounds, items, 2, 0);

	timer.stopAndPrint("KD-Tree build");
}

/*
 * Print KD-tree statistics
 */
void KDTree::printDebugInfo() {
	int numRaysCast = 0;

	for (int i = 0; i < numThreads; i++)
		numRaysCast += threadStates[i].numRaysCast;

	Loggers::Main.Log(LOGGER_NONCRITICAL, "KD tree: %d rays cast\n", numRaysCast);
}

KDContainable *KDTree::leafNearest(KDNode *leaf, const Vec3 & pt, float maxRadius, float *dist) {
	KDContainable *closestKDContainable = NULL;
	float closestDist = maxRadius;

	for (int i = 0; i < leaf->nItems; i++) {
		KDContainable *KDContainable = leaf->items[i];

		Vec3 p = KDContainable->getPosition();
		Vec3 r = p - pt;
		float len = r.len();

		if (closestKDContainable == NULL || len < closestDist) {
			closestDist = len;
			closestKDContainable = KDContainable;
		}
	}

	*dist = closestDist;
	return closestKDContainable;
}

KDContainable *KDTree::nearestInternal(KDNode *node, const Vec3 & pt, float maxRadius, float *dist) {
	if (is_leaf(node)) {
		return leafNearest(node, pt, maxRadius, dist);
	}
	else {
		float ptR = pt.components[node->dir];

		if (ptR + maxRadius < node->split) {
			// left only
			return nearestInternal(node->left, pt, maxRadius, dist);
		}
		else if (ptR - maxRadius > node->split) {
			// right only
			return nearestInternal(node->right, pt, maxRadius, dist);
		}
		else {
			// both

			KDContainable *left, *right;
			float distLeft, distRight;

			left  = nearestInternal(node->left,  pt, maxRadius, &distLeft);
			right = nearestInternal(node->right, pt, distLeft , &distRight);

			if (left != NULL && distLeft <= distRight) {
				*dist = distLeft;
				return left;
			}
			else if (right != NULL && distRight <= distLeft) {
				*dist = distRight;
				return right;
			}
			else
				return NULL;
		}
	}
}

KDContainable *KDTree::nearest(const Vec3 & pt, float maxRadius, float *dist) {
	if (maxRadius == 0.0f) {
		Vec3 cross = sceneBounds.max - sceneBounds.min;
		maxRadius = cross.len();
	}

	return nearestInternal(root, pt, maxRadius, dist);
}

/*
 * Perform an intersection of a ray against the KD tree, returning the intersecting
 * shape and collision distance
 */
bool KDTree::intersect(const Ray & ray, float maxDepth) {
	float entry, exit;

	if (!sceneBounds.intersects(ray, &entry, &exit))
		return false;

	KDThreadState *thread = getMyThreadState();

	vector<KDStackItem> stack = thread->stack;
	stack.clear();

	KDStackItem item(root, entry, exit);
	stack.push_back(item);

	KDNode *currentNode;

	while (stack.size() != 0) {
		KDStackItem top = stack[stack.size() - 1];
		stack.pop_back();

		currentNode = top.node;
		entry = top.enter;
		exit = top.exit;

		if (maxDepth != 0.0f && entry > maxDepth)
			continue;

		while (!is_leaf(currentNode)) {
			int dir = currentNode->dir;

			float radius = currentNode->split;
			float origin, direction;

			origin    = ray.origin.components[dir];
			direction = ray.direction.components[dir];

			float t = (radius - origin) / direction;
			
			KDNode *nearNode = currentNode->left;
			KDNode *farNode  = currentNode->right;

			if (radius < origin)
			{
				KDNode *temp = nearNode;
				nearNode = farNode;
				farNode = temp;
			}

			if (t >= exit || t < 0)
				currentNode = nearNode;
			else if (t <= entry)
				currentNode = farNode;
			else {
				KDStackItem nextItem(farNode, t, exit);
				stack.push_back(nextItem);

				currentNode = nearNode;
				exit = t;
			}
		}

		float fakeExit = exit;

		if (maxDepth != 0.0f)
			fakeExit = exit < maxDepth ? exit : maxDepth;

		if (intersectLeaf(currentNode, ray, thread, entry, fakeExit))
			return true;
	}

	return false;
}

/*
 * Perform an intersection of a ray against the KD tree, returning the intersecting
 * shape and collision distance
 */
bool KDTree::intersect(const Ray & ray, CollisionResult *result, float maxDepth) {
	float entry, exit;

	if (!sceneBounds.intersects(ray, &entry, &exit))
		return false;

	KDThreadState *thread = getMyThreadState();

	vector<KDStackItem> stack = thread->stack;
	stack.clear();

	KDStackItem item(root, entry, exit);
	stack.push_back(item);

	KDNode *currentNode;

	while (stack.size() != 0) {
		KDStackItem top = stack[stack.size() - 1];
		stack.pop_back();

		currentNode = top.node;
		entry = top.enter;
		exit = top.exit;

		if (maxDepth != 0.0f && entry > maxDepth)
			continue;

		while (!is_leaf(currentNode)) {
			int dir = currentNode->dir;

			float radius = currentNode->split;
			float origin, direction;

			origin    = ray.origin.components[dir];
			direction = ray.direction.components[dir];

			float t = (radius - origin) / direction;
			
			KDNode *nearNode = currentNode->left;
			KDNode *farNode  = currentNode->right;

			if (radius < origin)
			{
				KDNode *temp = nearNode;
				nearNode = farNode;
				farNode = temp;
			}

			if (t >= exit || t < 0)
				currentNode = nearNode;
			else if (t <= entry)
				currentNode = farNode;
			else {
				KDStackItem nextItem(farNode, t, exit);
				stack.push_back(nextItem);

				currentNode = nearNode;
				exit = t;
			}
		}

		float fakeExit = exit;

		if (maxDepth != 0.0f)
			fakeExit = exit < maxDepth ? exit : maxDepth;

		if (intersectLeaf(currentNode, ray, result, thread, entry, fakeExit))
			return true;
	}

	return false;
}
