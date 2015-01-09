/**
 * @file kdtree.cpp
 *
 * @author Sean James
 */

#include <kdtree/kdtree.h>

KDTree::KDTree(KDNode *root, AABB bounds)
    : root(root),
      bounds(bounds)
{
}

bool KDTree::intersectLeaf(KDNode *leaf, const Ray & ray, Collision & result, float entry, float exit,
    bool anyCollision)
{
    bool found = false;
    Collision tmpResult; // TODO

    unsigned int num_triangles = leaf->flags & KD_SIZE_MASK;

    for (int i = 0; i < num_triangles; i++) {
        SetupTriangle *triangle = &leaf->triangles[i];

        bool intersects = triangle->intersects(ray, tmpResult);

        if (intersects && tmpResult.distance >= entry &&
            tmpResult.distance <= exit && (tmpResult.distance < result.distance || !found))
        {
            result = tmpResult;

            if (anyCollision)
                return true;

            found = true;
        }
    }

    return found;
}

bool KDTree::intersect(const Ray & ray, Collision & result, float maxDepth, bool anyCollision) {
    float entry, exit;

    if (!bounds.intersects(ray, &entry, &exit))
        return false;

    KDStackFrame stack[64]; // TODO max size
    KDStackFrame *curr_stack = stack;

    curr_stack->node = root;
    curr_stack->enter = entry;
    curr_stack->exit = exit;
    curr_stack++;

    KDNode *currentNode;

    while (curr_stack != stack) {
        curr_stack--;
        currentNode = curr_stack->node;
        entry = curr_stack->enter;
        exit = curr_stack->exit;

        // Nothing will be closer, give up
        if (maxDepth > 0.0f && entry > maxDepth)
            return false;

        while (!(currentNode->flags & KD_IS_LEAF)) {
            int dir = currentNode->flags & KD_SPLIT_DIR_MASK;

            float split = currentNode->split_dist;
            float origin = ray.origin.v[dir];

            float t = (split - origin) * ray.inv_direction.v[dir];

            KDNode *nearNode = currentNode->left;
            KDNode *farNode  = currentNode->right;

            if (split < origin) {
                KDNode *temp = nearNode;
                nearNode = farNode;
                farNode = temp;
            }

            if (t > exit || t < 0)
                currentNode = nearNode;
            else if (t < entry)
                currentNode = farNode;
            else {
                curr_stack->node = farNode;
                curr_stack->enter = t;
                curr_stack->exit = exit;
                curr_stack++;

                currentNode = nearNode;
                exit = t;
            }
        }

        // Again, nothing will be closer so we're done
        if (intersectLeaf(currentNode, ray, result, entry, maxDepth > 0.0f ?
            MIN2(maxDepth, exit) : exit, anyCollision))
            return true;
    }

    return false;
}
