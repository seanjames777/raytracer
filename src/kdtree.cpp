/**
 * @file kdtree.cpp
 *
 * @author Sean James
 */

#include <kdtree.h>

KDTree::KDNode::KDNode(KDNode *left, KDNode *right, float split_dist,
    SetupTriangle *triangles, unsigned int num_triangles, unsigned int flags)
    : left(left),
      right(right),
      split_dist(split_dist),
      triangles(triangles),
      num_triangles(num_triangles),
      flags(flags)
{
}

void KDTree::partition(const AABB & box, const std::vector<Triangle *> & triangles,
    std::vector<Triangle *> & contained)
{
    for (auto it = triangles.begin(); it != triangles.end(); it++)
        if (box.intersectsBbox((*it)->bbox))
            contained.push_back(*it);
}

KDTree::KDNode *KDTree::buildLeaf(const std::vector<Triangle *> & triangles) {
    // Set up triangles, and pack them together for locality
    unsigned int num_triangles = (unsigned int)triangles.size();
    SetupTriangle *setup = (SetupTriangle *)malloc(sizeof(SetupTriangle) * num_triangles);

    for (unsigned int i = 0; i < num_triangles; i++)
        setup[i] = SetupTriangle(*triangles[i]);

    // TODO: delete the setup triangles. Might also want to change constructor of KDNode
    // to do this stuff, especially to not mix allocations

    return new KDNode(NULL, NULL, 0.0f, setup, num_triangles, 4); // TODO constant
}

KDTree::KDNode *KDTree::buildMean(AABB bounds, const std::vector<Triangle *> & triangles, int depth) {
    // TODO: possibly traverse after construction to remove useless cells, etc.

    if (depth >= 25 || triangles.size() < 4)
        return buildLeaf(triangles);
    else {
        int dir = depth % 3;
        float min = bounds.min.v[dir];
        float max = bounds.max.v[dir];
        float split = (max - min) / 2.0f + min;

        AABB leftBB, rightBB;
        bounds.split(split - min, dir, leftBB, rightBB);

        std::vector<Triangle *> leftItems;
        std::vector<Triangle *> rightItems;

        leftItems.reserve(triangles.size()); // TODO
        rightItems.reserve(triangles.size()); // TODO

        partition(leftBB, triangles, leftItems);
        partition(rightBB, triangles, rightItems);

        KDNode *left = buildMean(leftBB, leftItems, depth + 1);
        KDNode *right = buildMean(rightBB, rightItems, depth + 1);

        return new KDNode(left, right, split, NULL, 0, dir);
    }
}

AABB KDTree::buildAABB(const std::vector<Triangle *> & triangles) {
    if (triangles.size() == 0)
        return AABB();

    AABB box = triangles[0]->bbox;

    for (auto it = triangles.begin(); it != triangles.end(); it++)
        box.join((*it)->bbox);

    return box;
}

bool KDTree::intersectLeaf(KDNode *leaf, Ray ray, Collision *result, float entry, float exit,
    bool anyCollision)
{
    bool found = false;
    Collision tmpResult;

    for (int i = 0; i < leaf->num_triangles; i++) {
        SetupTriangle *triangle = &leaf->triangles[i];

        if (triangle->intersects(ray, &tmpResult) && tmpResult.distance >= entry &&
            tmpResult.distance <= exit && (tmpResult.distance < result->distance || !found))
        {
            *result = tmpResult;

            if (anyCollision)
                return true;

            found = true;
        }
    }

    return found;
}

KDTree::KDTree(const std::vector<Triangle> & triangles) {
    // Use pointers while building the tree, because we need to be able to sort, etc.
    // Triangles are converted to "setup" triangles, so we don't actually need the
    // triangle data anyway.
    std::vector<Triangle *> pointers;

    pointers.reserve(triangles.size());

    for (auto & it : triangles)
        pointers.push_back(const_cast<Triangle *>(&it));

    sceneBounds = buildAABB(pointers);
    root = buildMean(sceneBounds, pointers, 0);
}

bool KDTree::intersect(Ray ray, Collision *result, float maxDepth, bool anyCollision) {
    float entry, exit;

    if (!sceneBounds.intersects(ray, &entry, &exit))
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

        while (!(currentNode->flags & 4)) {
            int dir = currentNode->flags & 3;

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

            if (t >= exit || t < 0)
                currentNode = nearNode;
            else if (t <= entry)
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
