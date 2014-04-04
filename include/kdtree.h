/**
 * @file kdtree.h
 *
 * @brief KD-Tree
 *
 * @author Sean James
 */

#ifndef _KDTREE_H
#define _KDTREE_H

#include <defs.h>
#include <rtmath.h>
#include <kdnode.h>

/**
 * @brief KD-tree traversal stack item
 */
template<class T, class R>
struct KDStackItem {

	/** @brief KD-Node to traverse */
	KDNode<T, R> *node;

	/** @brief Entry distance from ray origin */
	float enter;

	/** @brief Exit distance from ray origin */
	float exit;

	/**
	 * @brief Constructor
	 *
	 * @param node  KD-Node to traverse
	 * @param enter Entry distance from ray origin
	 * @param exit  Exit distance from ray origin
	 */
	KDStackItem<T, R>(KDNode<T, R> *node, float enter, float exit)
		: node(node),
		  enter(enter),
		  exit(exit)
	{
	}

};

/**
 * @brief KD-Tree
 */
template<class T, class R>
class KDTree {
private:

	/** @brief Root of the KD-Tree */
	KDNode<T, R> *root;

	/** @brief Bounding box containing the entire scene */
	AABB sceneBounds;

	/**
	 * @brief Unpack the components of a vector
	 *
	 * @param v   Vector
	 * @param idx Index to retrieve (x = 0, y = 1, z = 2)
	 */
	inline float unpack_vec(Vec3 vector, int idx) {
		switch (idx) {
		case 0:
			return vector.x;
		case 1:
			return vector.y;
		case 2:
			return vector.z;
		}

		return vector.x; // Shouldn't happen
	}

	/**
	 * @brief Count the subset of items contained in a bounding box
	 *
	 * @param box       Bounding box
	 * @param items     Items to check
	 * @param contained Vector where items in box will be placed
	 *
	 * @return Number of items added to contained set
	 */
	int countInBox(AABB box, std::vector<T *> & items, std::vector<T *> & contained)
	{
		int count = 0;

		for (auto it = items.begin(); it != items.end(); it++)
			if ((*it)->containedIn(box)) {
				contained.push_back(*it);
				count++;
			}

		return count;
	}

	/**
	 * @brief Determine the split distance for the given bounding box and direction
	 *
	 * @param bounds Bounding box to split
	 * @param dir    Split direction (x = 0, y = 1, z = 2)
	 *
	 * @return World-space position to split box
	 */
	float splitDist(AABB bounds, int dir) {
		Vec3 max = bounds.max, min = bounds.min;
		Vec3 ext = max - min;

		return unpack_vec(ext, dir) / 2.0f + unpack_vec(bounds.min, dir);
	}

	/**
	 * @brief Build a KD tree containing the given items
	 *
	 * @param bounds Bounding box containing all items
	 * @param items  Items to place in tree
	 * @param dir    Direction to split root
	 * @param depth  Recursion depth
	 */
	KDNode<T, R> *build(AABB bounds, std::vector<T *> & items, int dir, int depth) {
		KDNode<T, R> *node = new KDNode<T, R>(NULL, NULL, 0.0f, 0);

		if (depth > 6 || items.size() < 4) {
			// leaf if
			// 1) too few items
			// 2) too deep

			node->nItems = items.size();
			node->items = new T *[node->nItems];

			int i = 0;
			for (auto it = items.begin(); it != items.end(); it++)
				node->items[i++] = *it;
		}
		else {
			// not a leaf

			node->split = splitDist(bounds, dir);
			node->dir = dir;

			AABB leftBB = node->getBBoxL(bounds);
			AABB rightBB = node->getBBoxR(bounds);

			std::vector<T *> leftItems;
			std::vector<T *> rightItems;

			int countL = countInBox(leftBB, items, leftItems);
			int countR = countInBox(rightBB, items, rightItems);

			KDNode<T, R> *left  = build(leftBB, leftItems,  (dir + 1) % 3, depth + 1);
			KDNode<T, R> *right = build(rightBB, rightItems, (dir + 1) % 3, depth + 1);

			node->left  = left;
			node->right = right;
		}

		return node;
	}

	/**
	 * @brief Check whether the given node is a leaf
	 */
	inline bool is_leaf(KDNode<T, R> *node) {
		return node->left == NULL && node->right == NULL;
	}

	/*
	 * Check every item in a leaf node for intersection against a given ray
	 */
	bool intersectLeaf(KDNode<T, R> *leaf, Ray ray, R *result, float enter,
		float exit)
	{
		result->shape = NULL;

		R tmpResult;

		for (int i = 0; i < leaf->nItems; i++) {
			T *item = leaf->items[i];

			if (item->intersects(ray, &tmpResult)) {
				if (tmpResult.distance < enter || tmpResult.distance > exit) {
					continue;
				}
				else if (result->shape == NULL || tmpResult.distance < result->distance)
					*result = tmpResult;
			}
		}

		return result->shape != NULL;
	}

	/**
	 * @brief Find the item closest to a point in a leaf node
	 *
	 * @param leaf      Leaf node to check
	 * @param pt        Point to search near
	 * @param maxRadius Maximum distance to item
	 * @param dist      Will be set to the distance to the found item
	 *
	 * @return Found item or NULL
	 */
	T *leafNearest(KDNode<T, R> *leaf, Vec3 pt, float maxRadius, float *dist) {
		T *closest = NULL;
		float closestDist = maxRadius;

		for (int i = 0; i < leaf->nItems; i++) {
			T *item = leaf->items[i];

			Vec3 p = item->getPosition();
			Vec3 r = p - pt;
			float len = r.len();

			if (closest == NULL || len < closestDist) {
				closestDist = len;
				closest = item;
			}
		}

		*dist = closestDist;
		return closest;
	}

	/**
	 * @brief Find the item closest to a point
	 *
	 * @param node      Node to search
	 * @param pt        Point to search near
	 * @param maxRadius Maximum distance to point
	 * @param dist      Will be set to distance to item
	 *
	 * @return The nearest item or NULL
	 */
	T *nearestInternal(KDNode<T, R> *node, Vec3 pt, float maxRadius, float *dist) {
		if (is_leaf(node)) {
			return leafNearest(node, pt, maxRadius, dist);
		}
		else {
			float ptR = unpack_vec(pt, node->dir);
			
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

				T *left, *right;
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

	/**
	 * @brief Compute a bounding box for a set of items
	 */
	AABB buildAABB(std::vector<T *> & items) {
		if (items.size() == 0)
			return AABB();

		AABB box = items[0]->getBBox();

		for (auto it = items.begin(); it != items.end(); it++)
			box.join((*it)->getBBox());

		return box;
	}

	/**
	 * @brief Print a view of the tree
	 *
	 * @param node  Root node
	 * @param depth Recursion depth
	 */
	void printInternal(KDNode<T, R> *node, int depth) {
		for (int i = 0; i < depth; i++)
			std::cout << "\t";

		if (is_leaf(node))
			std::cout << "Leaf: " << node->nItems << " items" << std::endl;
		else {
			std::cout << "Node: split=" << node->split << std::endl;
			printInternal(node->left, depth + 1);
			printInternal(node->right, depth + 1);
		}
	}

public:

	/**
	 * @brief Constructor, builds a KD-Tree from the given items and bounds
	 *
	 * @param items       Items to contain in the tree
	 */
	KDTree<T, R>(std::vector<T *> & items) {
		sceneBounds = buildAABB(items);
		root = build(sceneBounds, items, 2, 0);
	}

	/**
	 * @brief Intersect a ray against the KD-Tree, returning the closest intersecting shape and
	 * collision distance
	 *
	 * @param ray      Ray to test against 
	 * @param result   Will be filled with collision information
	 * @param maxDepth Maximum search distance, or 0 for any distance
	 *
	 * @return Whether a collision occured
	 */
	bool intersect(Ray ray, R *result, float maxDepth) {
		float entry, exit;

		if (!sceneBounds.intersects(ray, &entry, &exit))
			return false;

		std::vector<KDStackItem<T, R>> stack;

		KDStackItem<T, R> item(root, entry, exit);
		stack.push_back(item);

		KDNode<T, R> *currentNode;

		while (stack.size() != 0) {
			KDStackItem<T, R> top = stack[stack.size() - 1];
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

				origin    = unpack_vec(ray.origin, dir);
				direction = unpack_vec(ray.direction, dir);

				float t = (radius - origin) / direction;
				
				KDNode<T, R> *nearNode = currentNode->left;
				KDNode<T, R> *farNode  = currentNode->right;

				if (radius < origin)
				{
					KDNode<T, R> *temp = nearNode;
					nearNode = farNode;
					farNode = temp;
				}

				if (t >= exit || t < 0)
					currentNode = nearNode;
				else if (t <= entry)
					currentNode = farNode;
				else {
					KDStackItem<T, R> nextItem(farNode, t, exit);
					stack.push_back(nextItem);

					currentNode = nearNode;
					exit = t;
				}
			}

			float fakeExit = exit;

			if (maxDepth != 0.0f)
				fakeExit = exit < maxDepth ? exit : maxDepth;

			if (intersectLeaf(currentNode, ray, result, entry, fakeExit))
				return true;
		}

		return false;
	}

	/**
	 * @brief Find the item closest to a point
	 *
	 * @param pt        Point to search around
	 * @param maxRadius Maximum distance
	 * @param dist      Will be set to distance to item
	 *
	 * @return The nearest item or NULL
	 */
	T *nearest(Vec3 pt, float maxRadius, float *dist) {
		if (maxRadius == 0.0f) {
			Vec3 cross = sceneBounds.max - sceneBounds.min;
			maxRadius = cross.len();
		}

		return nearestInternal(root, pt, maxRadius, dist);
	}

	/**
	 * @brief Print a view of the tree
	 */
	void print() {
		printInternal(root, 0);
	}
};

#endif
