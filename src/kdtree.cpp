/**
 * @file kdtree.cpp
 *
 * @author Sean James
 */

#include <kdtree.h>

KDNode::KDNode(KDNode *left, KDNode *right, float split, int dir)
  : left(left),
    right(right),
    split(split),
    dir(dir),
    items(NULL),
    nItems(0)
{
}