/**
 * @file kdnode.cpp
 *
 * @author Sean James
 */

#include <kdtree/kdnode.h>

KDNode::KDNode()
{
}

KDNode::~KDNode() {
    if (flags & KD_IS_LEAF) {
		SetupTriangleBuffer::destroy(triangles);

        // TODO: Memory management strategy for the whole tree/builders.
    }
    else {
        delete left;
        delete right;
    }
}
