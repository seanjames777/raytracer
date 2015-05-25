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

#define KDNODE_LEFT(node)      (&((KDNode *)(node->ptr & 0xfffffffffffffffc))[0])
#define KDNODE_RIGHT(node)     (&((KDNode *)(node->ptr & 0xfffffffffffffffc))[1])
#define KDNODE_TYPE(node)      (node->ptr & 0x0000000000000003)
#define KDNODE_TRIANGLES(node) ((char *)(node->ptr & 0xfffffffffffffffc))

#define KD_INTERNAL_X 0
#define KD_INTERNAL_Y 1
#define KD_INTERNAL_Z 2
#define KD_LEAF       3 // TODO: if this was 0 comparisons could be faster

// 32 bytes = 1/2 cache line size
struct KDNode {
    // TODO: could get this down to 8 bytes. With proper alignment, it's actually
    // 16 bytes

    // 8. Triangle pointer for leaves, children (adjacent) pointer for internal.
    // Bottom two bits store node type, nodes are at least 16 bit aligned.
    unsigned long long ptr;

    // 4/4. Split distance or triangle count
    union {
        float          split_dist;
        unsigned int   count;
    };
};

#endif
