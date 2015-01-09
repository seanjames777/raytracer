/**
 * @file kdnode.cpp
 *
 * @author Sean James
 */

#include <kdtree/kdnode.h>

KDNode::KDNode(KDNode *left, KDNode *right, float split_dist,
    SetupTriangle *triangles, unsigned int flags)
    : left(left),
      right(right),
      split_dist(split_dist),
      triangles(triangles),
      flags(flags)
{
}

KDNode::~KDNode() {
    if (flags & KD_IS_LEAF)
        free(triangles);
    else {
        delete left;
        delete right;
    }
}
