/**
 * @file kdnode.h
 *
 * @brief KD-Tree internal or leaf node
 *
 * @author Sean James
 */

#ifndef _KDNODE_H
#define _KDNODE_H

#include <polygon.h>

enum KDNodeFlags {
    // If the node is a leaf, the bottom 31 bits are the size and the top
    // bit is 1. Otherwise, the node is not a leaf, so the top bit is 0
    // and the bottom two bits are the split direction.

    KD_SIZE_MASK       = ~(1 << 31),
    KD_SPLIT_DIR_X     =   0,
    KD_SPLIT_DIR_Y     =   1,
    KD_SPLIT_DIR_Z     =   2,
    KD_SPLIT_DIR_MASK  =   3,
    KD_IS_LEAF         =   1 << 31,
};

// 32 bytes = 1/2 cache line size
struct KDNode {
    KDNode       *left;       // 8
    KDNode       *right;      // 8
    char         *triangles;  // 8
    float         split_dist; // 4
    unsigned int  flags;      // 4 -- Triangle count and leaf or flags and not leaf
};

#endif
