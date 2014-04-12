/**
 * @file kdtree.cpp
 *
 * @author Sean James
 */

#include <kdtree.h>

static int node_id = 0;

KDNode::KDNode(KDNode *left, KDNode *right, float split, int dir)
  : left(left),
    right(right),
    split(split),
    dir(dir),
    items(NULL),
    nItems(0),
    id(node_id++)
{
}