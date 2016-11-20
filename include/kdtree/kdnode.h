/**
 * @file kdtree/kdnode.h
 *
 * @brief KD-Tree internal or leaf node
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __KDNODE_H
#define __KDNODE_H

#if !GPU
#include <stdint.h>
#endif

#include <core/triangle.h>

#define KD_INTERNAL_X 0
#define KD_INTERNAL_Y 1
#define KD_INTERNAL_Z 2
#define KD_LEAF       3

// TODO: pack node

struct KDNode {
    // TODO: could get this down to 8 bytes. With proper alignment, it's actually
    // 16 bytes

    // 8. Triangle pointer for leaves, children (adjacent) pointer for internal.
    // Bottom two bits store node type, nodes are at least 16 bit aligned.
    uint32_t offset;

    // 4/4. Split distance or triangle count
    union {
        float          split_dist;
        unsigned int   count;
    };
    
    uint32_t magic;
    
    inline uint32_t type() const GLOBAL {
        return offset & 0x00000003;
    }
    
    inline GLOBAL KDNode *left(GLOBAL KDNode *nodes) const GLOBAL {
        GLOBAL KDNode *children = (GLOBAL KDNode *)((GLOBAL char *)nodes + (offset & 0xFFFFFFFC));
        return &children[0];
    }
    
    inline GLOBAL KDNode *right(GLOBAL KDNode *nodes) const GLOBAL {
        GLOBAL KDNode *children = (GLOBAL KDNode *)((GLOBAL char *)nodes + (offset & 0xFFFFFFFC));
        return &children[1];
    }
    
    inline GLOBAL SetupTriangle *triangles(GLOBAL SetupTriangle *triangles) const GLOBAL {
        return (GLOBAL SetupTriangle *)((GLOBAL char *)triangles + (offset & 0xFFFFFFFC));
    }
};

#endif
