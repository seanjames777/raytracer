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
    
#if GPU
    inline uint32_t type() const device {
        return offset & 0x00000003;
    }
    
    inline device KDNode *left(device KDNode *nodes) const device {
        device KDNode *children = (device KDNode *)((device char *)nodes + (offset & 0xFFFFFFFC));
        return &children[0];
    }
    
    inline device KDNode *right(device KDNode *nodes) const device {
        device KDNode *children = (device KDNode *)((device char *)nodes + (offset & 0xFFFFFFFC));
        return &children[1];
    }
    
    inline device SetupTriangle *triangles(device SetupTriangle *triangles) const device {
        return (device SetupTriangle *)((device char *)triangles + (offset & 0xFFFFFFFC));
    }
#else
    inline uint32_t type() const {
        return offset & 0x00000003;
    }
    
    inline KDNode *left(KDNode *nodes) const {
        KDNode *children = (KDNode *)((char *)nodes + (offset & 0xFFFFFFFC));
        return &children[0];
    }
    
    inline KDNode *right(KDNode *nodes) const {
        KDNode *children = (KDNode *)((char *)nodes + (offset & 0xFFFFFFFC));
        return &children[1];
    }
    
    inline SetupTriangle *triangles(SetupTriangle *triangles) const {
        return (SetupTriangle *)((char *)triangles + (offset & 0xFFFFFFFC));
    }
#endif

};

#endif
