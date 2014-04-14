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
    KDNode(KDNode *left, KDNode *right, float split, int dir);

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

	void buildLeaf(KDNode *node, std::vector<PolygonAccel *> & items) {
		node->nItems = items.size();
		node->items = new PolygonAccel *[node->nItems];

		int i = 0;
			for (auto it = items.begin(); it != items.end(); it++)
				node->items[i++] = *it;
	}

	/**
	 * @brief Build a KD tree containing the given items
	 *
	 * @param bounds Bounding box containing all items
	 * @param items  Items to place in tree
	 * @param dir    Direction to split root
	 * @param depth  Recursion depth
	 */
	KDNode *build(AABB bounds, std::vector<PolygonAccel *> & items, int depth) {
		KDNode *node = new KDNode(NULL, NULL, 0.0f, 0);

		// TODO: possibly traverse after construction to remove useless cells, etc.

		if (depth >= 25 || items.size() < 4)
			buildLeaf(node, items);
		else {
#if 0
			std::vector<float> splits;

			for (int i = 0; i < items.size(); i++) {
				// TODO: calculate bbox while building instead of storing at runtime
				AABB box = items[i]->getBBox();

				splits.push_back(box.min.x);
				splits.push_back(box.min.y);
				splits.push_back(box.min.z);
				splits.push_back(box.max.x);
				splits.push_back(box.max.y);
				splits.push_back(box.max.z);
			}

			float minSplitVal = INFINITY32F;
			AABB minLeft, minRight;
			int minDir;
			float minSplitDist;

			std::vector<PolygonAccel *> leftItems;
			std::vector<PolygonAccel *> rightItems;

			float surfaceArea = bounds.surfaceArea();

			for (int i = 0; i < splits.size(); i++) {
				float split = splits[i];
				int dir = i % 3;

				float min = bounds.min.get(dir);
				float max = bounds.max.get(dir);

				if (split < min || split > max)
					continue;

				AABB leftBB, rightBB;
				bounds.split(split - min, node->dir, leftBB, rightBB);

				int countL = 0, countR = 0;

				for (int j = 0; j < items.size(); j++) {
					AABB box = items[j]->getBBox();

					float min = box.min.get(dir);
					float max = box.max.get(dir);

					if (min <= split && max >= split) {
						countL++;
						countR++;
					}
					else if (max <= split)
						countL++;
					else if (min >= split)
						countR++;
				}

				float cost = (countL == 0 || countR == 0) ? .8 : 1;
				cost *= (15.0f + 20.0f * (countL * leftBB.surfaceArea() / surfaceArea +
					countR * rightBB.surfaceArea() / surfaceArea));

				if (cost < minSplitVal) {
					minSplitVal = cost;
					minLeft = leftBB;
					minRight = rightBB;
					minDir = dir;
					minSplitDist = split;
				}

				leftItems.clear();
				rightItems.clear();
			}

			node->split = minSplitDist;
			node->dir = minDir;
			int count = countInBox(minLeft, items, leftItems);
			count += countInBox(minRight, items, rightItems);

			if(minSplitVal > count * 20.0f)
				buildLeaf(node, items);
			else {
				KDNode *left  = build(minLeft, leftItems, depth + 1);
				KDNode *right = build(minRight, rightItems, depth + 1);

				node->left  = left;
				node->right = right;
			}
#else
			int dir = depth % 3;
			float min = bounds.min.get(dir);
			float max = bounds.max.get(dir);

			node->dir = dir;
			node->split = (max - min) / 2.0f + min;

			AABB leftBB, rightBB;
			bounds.split(node->split - min, dir, leftBB, rightBB);

			std::vector<PolygonAccel *> leftItems;
			std::vector<PolygonAccel *> rightItems;

			countInBox(leftBB, items, leftItems);
			countInBox(rightBB, items, rightItems);

			node->left = build(leftBB, leftItems, depth + 1);
			node->right = build(rightBB, rightItems, depth + 1);
#endif
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
		result->distance = INFINITY32F;

		Collision tmpResult;

		for (int i = 0; i < leaf->nItems; i++) {
			PolygonAccel *item = leaf->items[i];

			if (item->intersects(ray, &tmpResult) && /* tmpResult.distance >= entry && */
				tmpResult.distance <= exit && tmpResult.distance < result->distance)
				*result = tmpResult;
		}

		return result->distance < INFINITY32F;
	}

public:

	/**
	 * @brief Constructor, builds a KD-Tree from the given items and bounds
	 *
	 * @param items Items to contain in the tree
	 */
	KDTree(std::vector<PolygonAccel *> & items) {
		sceneBounds = buildAABB(items);
		root = build(sceneBounds, items, 0);
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

				float split = currentNode->split;
				float origin = ray.origin.get(dir);

				float t = (split - origin) * ray.inv_direction.get(dir);
				
				KDNode *nearNode = currentNode->left;
				KDNode *farNode  = currentNode->right;

				if (split < origin) {
					KDNode *temp = nearNode;
					nearNode = farNode;
					farNode = temp;
				}

				if (t <= entry)
					currentNode = farNode;
				else if (t >= exit || t < 0)
					currentNode = nearNode;
				else {
					KDStackItem nextItem(farNode, t, exit);
					stack.push_back(nextItem);

					currentNode = nearNode;
					exit = t;
				}
			}

			// Again, nothing will be closer so we're done
			if (intersectLeaf(currentNode, ray, result, entry, maxDepth > 0.0f ?
				MIN2(maxDepth, exit) : exit))
				return true;
		}

		return false;
	}
};

#endif
