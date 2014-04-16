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
 * @brief KD-Tree
 */
class KDTree {
private:

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
        KDStackItem(KDNode *node, float enter, float exit);

    };

    /**
     * @brief KD-tree SAH event
     */
    struct SAHEvent {
        /** @brief Position */
        float position;

        /** @brief Type of event */
        enum EVENTTYPE {
            EVENTTYPE_BEGIN,
            EVENTTYPE_END
        } type;

        /** @brief Polygon */
        PolygonAccel *poly;

        /**
         * @brief Constructor
         *
         * @param position Position
         * @param type     Type of event
         * @param poly     Polygon
         */
        SAHEvent(float position, EVENTTYPE type, PolygonAccel *poly);
    };

    /** @brief Root of the KD-Tree */
    KDNode *root;

    /** @brief Bounding box containing the entire scene */
    AABB sceneBounds;

    /**
     * @brief Count the subset of items contained in a bounding box
     *
     * @param box       Bounding box
     * @param items     Items to check
     * @param contained Vector to fill with items overlapping box
     */
    void partitionMean(AABB box, std::vector<PolygonAccel *> & items,
        std::vector<PolygonAccel *> & contained);

    /**
     * @brief Build a leaf node
     *
     * @param items Items to place in leaf node
     */
    KDNode *buildLeaf(std::vector<PolygonAccel *> & items);

    /**
     * @brief Build a KD tree containing the given items
     *
     * @param bounds Bounding box containing all items
     * @param items  Items to place in tree
     * @param dir    Direction to split root
     * @param depth  Recursion depth
     */
    KDNode *buildMean(AABB bounds, std::vector<PolygonAccel *> & items, int depth);

#if 0
    KDTree::KDNode *KDTree::buildSAHRec(AABB bounds, std::vector<SAHEvent> & events, int depth);

    /**
     * @brief Build a KD tree according to the surface area heuristic
     *
     * @param bounds Bounding box containing all items
     * @param items  Items to place in tree
     * @param dir    Direction to split root
     * @param depth  Recursion depth
     */
    KDNode *buildSAH(AABB bounds, std::vector<PolygonAccel *> & items, int depth);
#endif

    /**
     * @brief Compute a bounding box for a set of items
     *
     * @param item Items to bound
     */
    AABB buildAABB(std::vector<PolygonAccel *> & items);

    /**
     * @brief Check for collision with a leaf node
     *
     * @param leaf         Leaf node to check
     * @param ray          Ray to check for collision against
     * @param result       Output collision information
     * @param entry        Minimum collision distance
     * @param exit         Maximum collision distance
     * @param anyCollision Whether to accept any collision or to find the closest
     */
    bool intersectLeaf(KDNode *leaf, Ray ray, Collision *result, float entry, float exit,
        bool anyCollision);

public:

    /**
     * @brief Constructor
     *
     * @param items Items to contain in the tree
     */
    KDTree(std::vector<PolygonAccel *> & items);

    /**
     * @brief Intersect a ray against the KD-Tree
     *
     * @param ray          Ray to test against
     * @param result       Will be filled with collision information
     * @param maxDepth     Maximum intersection distance, or 0 for any distance
     * @param anyCollision Whether to stop after any collision or to find the closest collision
     *
     * @return Whether a collision occured
     */
    bool intersect(Ray ray, Collision *result, float maxDepth, bool anyCollision);

};

#endif
