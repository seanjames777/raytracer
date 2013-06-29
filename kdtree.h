/*
 * Sean James
 *
 * kdtree.h
 *
 * KD-Tree
 *
 */

#ifndef _KDTREE_H
#define _KDTREE_H

#include "defs.h"
#include "rtmath.h"
#include "timer.h"

#include "atomicint.h"
#include "loggers.h"

#include "kdcontainable.h"

/*
 * A node in a KD tree
 */
struct KDNode {
	KDNode         *left;   // Pointer to the left sub-tree
	KDNode         *right;  // Pointer to the right sub-tree
	float           split;  // World-space position of the split plane along the node's axis
	int             dir;    // Axis of the node's split plane (x, y, z) = (0, 1, 2)
	KDContainable **items;  // Pointer to an array of items in this node if it is a leaf
	int nItems;             // Number of items in this node's bounding box
	int id;

	/*
	 * Default constructor accepts a split position and direction, and pointers to
	 * left and right sub trees. Items is assumed to be null and empty.
	 */
	KDNode(KDNode *Left, KDNode *Right, float Split, int Dir, int ID)
	  : left(Left),
		right(Right),
		split(Split),
		dir(Dir),
		items(NULL),
		nItems(0),
		id(ID)
	{
	}

	/*
	 * Get the left bounding box for this node
	 */
	AABB getBBoxL(const AABB & parent) {
		switch(dir) {
		case 0:
			return AABB(parent.min, Vec3(split, parent.max.y, parent.max.z));
		case 1:
			return AABB(parent.min, Vec3(parent.max.x, split, parent.max.z));
		case 2:
			return AABB(parent.min, Vec3(parent.max.x, parent.max.y, split));
		}

		return AABB(); // Shouldn't happen
	}

	/*
	 * Get the right bounding box for this node
	 */
	AABB getBBoxR(const AABB & parent) {
		switch(dir) {
		case 0:
			return AABB(Vec3(split, parent.min.y, parent.min.z), parent.max);
		case 1:
			return AABB(Vec3(parent.min.x, split, parent.min.z), parent.max);
		case 2:
			return AABB(Vec3(parent.min.x, parent.min.y, split), parent.max);
		}

		return AABB(); // Shouldn't happen
	}
};

/*
 * Data that can be pushed onto a KD-tree's traversal stack
 */
struct KDStackItem {
	KDNode *node;   // Pointer to the KDNode to traverse
	float enter;    // Entry distance from the ray origin
	float exit;     // Exit distance from the ray origin

	/*
	 * Default constructor accepts member values
	 */
	KDStackItem(KDNode *Node, float Enter, float Exit)
		: node(Node),
		enter(Enter),
		exit(Exit)
	{
	}
};

/*
 * Per-thread state used to collect statistics and avoid
 * continuously allocating a stack
 */
struct KDThreadState {
	vector<KDStackItem> stack;  // Thread's stack
	int numRaysCast;            // Number of rays this thread has cast
	pthread_t thread;           // This thread's handle
	bool initialized;           // Whether this KDThreadState has been assigned a thread

	/*
	 * Empty constructor initializes KDThreadState
	 */
	KDThreadState()
		: numRaysCast(0),
		  initialized(false)
	{
	}
};

/*
 * KD-Tree
 */
class KDTree {
private:

	KDNode *root;                 // The root node of the KD-Tree
	AABB sceneBounds;             // Bounding box containing the entire scene

	KDThreadState *threadStates;  // Array of per-thread state
	int numThreads;               // Number of threads accessing the tree
	Mutex threadStatesInitLock;   // Lock for assigning thread state blocks to threads

	int id;

	/*
	 * Get the subset and size of the subset of items whose bounding boxes
	 * intersect contained
	 */
	int countInBox(const AABB & box, vector<KDContainable *> & items, vector<KDContainable *> & contained);

	/*
	 * Determine the split distance for the given bounding box and direction
	 */
	float splitDist(const AABB & bounds, int dir);

	/*
	 * Check every item in a leaf node for intersection against a given ray
	 */
	bool intersectLeaf(KDNode *leaf, const Ray & ray, KDThreadState *thread, float enter, float exit);

	bool intersectLeaf(KDNode *leaf, const Ray & ray, CollisionResult *result, KDThreadState *thread, float enter, float exit);

	/*
	 * Build a KD tree containing the given items
	 */
	KDNode *build(const AABB & bounds, vector<KDContainable *> & items, int dir, int depth);

	/*
	 * Whether the given node is a leaf
	 */
	inline bool is_leaf(KDNode *node) {
		return node->left == NULL && node->right == NULL;
	}

	/*
	 * Locate or assign a thread state for the calling thread
	 */
	KDThreadState *getMyThreadState();

public:

	/*
	 * Default constructor builds a KD tree from the given items and bounds
	 * allowing access by up to nThreads threads
	 */
	KDTree(vector<KDContainable *> items, const AABB & itemsBounds, int nThreads);

	/*
	 * Print KD-tree statistics
	 */
	void printDebugInfo();

	/*
	 * Perform an intersection of a ray against the KD tree, returning the intersecting
	 * shape and collision distance
	 */
	bool intersect(const Ray & ray, float maxDepth);

	bool intersect(const Ray & ray, CollisionResult *result, float maxDepth);

	KDContainable *leafNearest(KDNode *leaf, const Vec3 & pt, float maxRadius, float *dist);

	KDContainable *nearestInternal(KDNode *node, const Vec3 & pt, float maxRadius, float *dist);

	KDContainable *nearest(const Vec3 & pt, float maxRadius, float *dist);
};

#endif