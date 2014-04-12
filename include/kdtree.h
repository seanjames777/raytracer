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
#include <polygon.h>

/**
 * @brief A node in a KD tree
 */
struct KDNode {

    /** @brief Left sub-tree */
    KDNode *left;

    /** @brief Right sub-tree */
    KDNode *right;

    /** @brief World-space split position of the split plane along the node's axis */
    float split;

    /** @brief Split direction (x = 0, y = 1, z = 2) */
    int dir;

    /** @brief Items contained in this (leaf-only) node */
    PolygonAccel **items;

    /** @brief Number of items in this node's bounding box */
    int nItems;

    /**
     * @brief Constructor
     *
     * @param left  Left sub-tree
     * @param right Right sub-tree
     * @param split World-space split position of the split plane along the node's axis
     * @param dir   Split direction (x = 0, y = 1, z = 2)
     */
    KDNode(KDNode *left, KDNode *right, float split, int dir)
      : left(left),
        right(right),
        split(split),
        dir(dir),
        items(NULL),
        nItems(0)
    {
    }
};

/**
 * @brief KD-tree traversal stack item
 */
struct KDStackItem {

	/** @brief KD-Node to traverse */
	KDNode *node;

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
	KDStackItem(KDNode *node, float enter, float exit)
		: node(node),
		  enter(enter),
		  exit(exit)
	{
	}

};

/**
 * @brief KD-Tree
 */
class KDTree {
private:

	/** @brief Root of the KD-Tree */
	KDNode *root;

	/** @brief Bounding box containing the entire scene */
	AABB sceneBounds;

	/**
	 * @brief Count the subset of items contained in a bounding box
	 *
	 * @param box       Bounding box
	 * @param items     Items to check
	 * @param contained Vector where items in box will be placed
	 *
	 * @return Number of items added to contained set
	 */
	int countInBox(AABB box, std::vector<PolygonAccel *> & items,
		std::vector<PolygonAccel *> & contained)
	{
		int count = 0;

		for (auto it = items.begin(); it != items.end(); it++)
			if (box.intersectsBbox((*it)->getBBox())) {
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

		return ext.get(dir) / 2.0f + bounds.min.get(dir);
	}

	/**
	 * @brief Build a KD tree containing the given items
	 *
	 * @param bounds Bounding box containing all items
	 * @param items  Items to place in tree
	 * @param dir    Direction to split root
	 * @param depth  Recursion depth
	 */
	KDNode *build(AABB bounds, std::vector<PolygonAccel *> & items, int dir, int depth, int prevSize) {
		KDNode *node = new KDNode(NULL, NULL, 0.0f, 0);

		if (depth > 10 || items.size() < 4) {
			// leaf if
			// 1) too few items
			// 2) too deep

			node->nItems = items.size();
			node->items = new PolygonAccel *[node->nItems];

			int i = 0;
			for (auto it = items.begin(); it != items.end(); it++)
				node->items[i++] = *it;
		}
		else {
			// not a leaf

			node->split = splitDist(bounds, dir);
			node->dir = dir;

			float splitDist;

			switch(dir) {
			case 0:
				splitDist = node->split - bounds.min.x;
				break;
			case 1:
				splitDist = node->split - bounds.min.y;
				break;
			case 2:
				splitDist = node->split - bounds.min.z;
				break;
			}

			AABB leftBB, rightBB;
			bounds.split(splitDist, node->dir, leftBB, rightBB);

			std::vector<PolygonAccel *> leftItems;
			std::vector<PolygonAccel *> rightItems;

			int countL = countInBox(leftBB, items, leftItems);
			int countR = countInBox(rightBB, items, rightItems);

			KDNode *left  = build(leftBB, leftItems,  (dir + 1) % 3, depth + 1, items.size());
			KDNode *right = build(rightBB, rightItems, (dir + 1) % 3, depth + 1, items.size());

			node->left  = left;
			node->right = right;
		}

		return node;
	}

	/**
	 * @brief Compute a bounding box for a set of items
	 */
	AABB buildAABB(std::vector<PolygonAccel *> & items) {
		if (items.size() == 0)
			return AABB();

		AABB box = items[0]->getBBox();

		for (auto it = items.begin(); it != items.end(); it++)
			box.join((*it)->getBBox());

		return box;
	}

	/**
	 * @brief Check every item in a leaf node for intersection against a given ray
	 */
	bool intersectLeaf(KDNode *leaf, Ray ray, Collision *result, float entry, float exit) {
		bool found = false;

		Collision tmpResult;

		for (int i = 0; i < leaf->nItems; i++) {
			PolygonAccel *item = leaf->items[i];

			if (item->intersects(ray, &tmpResult) && tmpResult.distance >= entry &&
				tmpResult.distance <= exit &&
				(!found || tmpResult.distance < result->distance))
			{
				found = true;
				*result = tmpResult;
			}
		}

		return found;
	}

public:

	/**
	 * @brief Constructor, builds a KD-Tree from the given items and bounds
	 *
	 * @param items       Items to contain in the tree
	 */
	KDTree(std::vector<PolygonAccel *> & items) {
		sceneBounds = buildAABB(items);
		root = build(sceneBounds, items, 2, 0, -1);
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
	bool intersect(Ray ray, Collision *result, float maxDepth) {
		float entry, exit;

		if (!sceneBounds.intersects(ray, &entry, &exit))
			return false;

		std::vector<KDStackItem> stack;

		KDStackItem item(root, entry, exit);
		stack.push_back(item);

		KDNode *currentNode;

		while (stack.size() != 0) {
			KDStackItem top = stack[stack.size() - 1];
			stack.pop_back();

			currentNode = top.node;
			entry = top.enter;
			exit = top.exit;

			// Nothing will be closer, give up
			if (maxDepth > 0.0f && entry > maxDepth)
				return false;

			while (!(currentNode->left == NULL && currentNode->right == NULL)) {
				int dir = currentNode->dir;

				float radius = currentNode->split;
				float origin, direction;

				origin    = ray.origin.get(dir);
				direction = ray.direction.get(dir);

				float t = (radius - origin) / direction;
				
				KDNode *nearNode = currentNode->left;
				KDNode *farNode  = currentNode->right;

				if (radius < origin) {
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

			if (intersectLeaf(currentNode, ray, result, entry, maxDepth > 0.0f ?
				MIN2(maxDepth, exit) : exit))
				return true;
		}

		return false;
	}
};

#endif
