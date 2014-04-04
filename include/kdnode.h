/**
 * @file kdnode.h
 *
 * @brief KD-Tree node
 *
 * @author Sean James
 */

#ifndef _KDNODE_H
#define _KDNODE_H

#include <defs.h>
#include <rtmath.h>

/**
 * @brief A node in a KD tree
 */
template<class T, class R>
struct KDNode {

    /** @brief Left sub-tree */
    KDNode<T, R> *left;

    /** @brief Right sub-tree */
    KDNode<T, R> *right;

    /** @brief World-space split position of the split plane along the node's axis */
    float split;

    /** @brief Split direction (x = 0, y = 1, z = 2) */
    int dir;

    /** @brief Items contained in this (leaf-only) node */
    T **items;

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
    KDNode<T, R>(KDNode<T, R> *left, KDNode *right, float split, int dir)
      : left(left),
        right(right),
        split(split),
        dir(dir),
        items(NULL),
        nItems(0)
    {
    }

    /**
     * @brief Get the left bounding box for this node
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

    /**
     * @brief Get the right bounding box for this node
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

#endif
