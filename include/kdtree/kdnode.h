/**
 * @file kdnode.h
 *
 * @brief KD-Tree internal or leaf node
 *
 * @author Sean James
 */

#ifndef _KDNODE_H
#define _KDNODE_H

#include <rtmath.h>
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

// TODO cache alignment, maybe compact, and child node heap?
struct KDNode {
    // 32 bytes = 1/2 cache line on x86/64

    KDNode        *left;
    KDNode        *right;
    SetupTriangle *triangles;
    float          split_dist;
    unsigned int   flags;

    KDNode();

    ~KDNode();

};

#endif
