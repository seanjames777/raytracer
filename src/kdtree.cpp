/**
 * @file kdtree.cpp
 *
 * @author Sean James
 */

#include <kdtree.h>

KDTree::KDNode::KDNode(KDNode *left, KDNode *right, float split, int dir)
  : left(left),
    right(right),
    split(split),
    dir(dir),
    items(NULL),
    nItems(0)
{
}

KDTree::KDStackItem::KDStackItem(KDNode *node, float enter, float exit)
    : node(node),
      enter(enter),
      exit(exit)
{
}

KDTree::SAHEvent::SAHEvent(float position, EVENTTYPE type, PolygonAccel *poly)
    : position(position),
      type(type),
      poly(poly)
{
}

#if 0
void KDTree::partitionSAH(std::vector<SAHEvent> & events, std::vector<int> & left,
    std::vector<int> & right)
{
    int countL = 0;
    int countR = events.size() / 2;
    bool prevEnd = false;

    for (int i = 0; i < events.size(); i++) {
        SAHEvent event = events[i];

        if (prevEnd)
            countR--;

        if (event.type == EVENTTYPE_BEGIN) {
            countL++;
            prevEnd = false;
        }
        else
            prevEnd = true;

        left.push_back(countL);
        right.push_back(countR);
    }
}

KDTree::KDNode *KDTree::buildSAHRec(AABB bounds, std::vector<SAHEvent> & d0,
    std::vector<SAHEvent> & d1, std::vector<SAHEvent> & d2, int depth)
{
    float minSplitVal = INFINITY32F;
    AABB minLeft, minRight;
    int minDir;
    float minSplitDist;

    std::vector<PolygonAccel *> leftItems;
    std::vector<PolygonAccel *> rightItems;

    float surfaceArea = bounds.surfaceArea();

    for (int i = 0; i < splits.size(); i++) {
        float split = splits[i];
        int dir = i % 3;

        float min = bounds.min.v[dir];
        float max = bounds.max.v[dir];

        if (split < min || split > max)
            continue;

        AABB leftBB, rightBB;
        bounds.split(split - min, dir, leftBB, rightBB);

        int countL = 0, countR = 0;

        for (int j = 0; j < items.size(); j++) {
            AABB box = items[j]->getBBox();

            float min = box.min.v[dir];
            float max = box.max.v[dir];

            if (min <= split && max >= split) {
                countL++;
                countR++;
            }
            else if (max <= split)
                countL++;
            else if (min >= split)
                countR++;
        }

        float cost = (countL == 0 || countR == 0) ? .8 : 1;
        cost *= (15.0f + 20.0f * (countL * leftBB.surfaceArea() / surfaceArea +
            countR * rightBB.surfaceArea() / surfaceArea));

        if (cost < minSplitVal) {
            minSplitVal = cost;
            minLeft = leftBB;
            minRight = rightBB;
            minDir = dir;
            minSplitDist = split;
        }

        leftItems.clear();
        rightItems.clear();
    }

    partitionMean(minLeft, items, leftItems);
    partitionMean(minRight, items, rightItems);

    int count = leftItems.size() + rightItems.size();

    if(minSplitVal > count * 20.0f)
        return buildLeaf(items);
    else {
        KDNode *left  = buildSAH(minLeft, leftItems, depth + 1);
        KDNode *right = buildSAH(minRight, rightItems, depth + 1);

        return new KDNode(left, right, minSplitDist, minDir);
    }
}

KDTree::KDNode *KDTree::buildSAH(AABB bounds, std::vector<PolygonAccel *> & items, int depth) {
    std::vector<SAHEvent> d0, d1, d2;

    for (int i = 0; i < items.size(); i++) {
        // TODO: calculate bbox while building instead of storing at runtime
        AABB box = items[i]->getBBox();

        d0.push_back(SAHEvent(box.min.x, EVENTTYPE_BEGIN, items[i]));
        d1.push_back(SAHEvent(box.min.y, EVENTTYPE_BEGIN, items[i]));
        d2.push_back(SAHEvent(box.min.z, EVENTTYPE_BEGIN, items[i]));

        d0.push_back(SAHEvent(box.max.x, EVENTTYPE_END, items[i]));
        d1.push_back(SAHEvent(box.max.y, EVENTTYPE_END, items[i]));
        d2.push_back(SAHEvent(box.max.z, EVENTTYPE_END, items[i]));
    }

    return buildSAHRec(bounds, items, depth);
}
#endif

void KDTree::partitionMean(AABB box, std::vector<PolygonAccel *> & items,
    std::vector<PolygonAccel *> & contained)
{
    for (auto it = items.begin(); it != items.end(); it++)
        if (box.intersectsBbox((*it)->getBBox()))
            contained.push_back(*it);
}

KDTree::KDNode *KDTree::buildLeaf(std::vector<PolygonAccel *> & items) {
    KDNode *node = new KDNode(NULL, NULL, 0.0f, 0);
    node->nItems = items.size();
    node->items = new PolygonAccel *[node->nItems];

    int i = 0;
    for (auto it = items.begin(); it != items.end(); it++)
        node->items[i++] = *it;

    return node;
}

KDTree::KDNode *KDTree::buildMean(AABB bounds, std::vector<PolygonAccel *> & items, int depth) {
    // TODO: possibly traverse after construction to remove useless cells, etc.

    if (depth >= 25 || items.size() < 4)
        return buildLeaf(items);
    else {
        int dir = depth % 3;
        float min = bounds.min.v[dir];
        float max = bounds.max.v[dir];
        float split = (max - min) / 2.0f + min;

        AABB leftBB, rightBB;
        bounds.split(split - min, dir, leftBB, rightBB);

        std::vector<PolygonAccel *> leftItems;
        std::vector<PolygonAccel *> rightItems;

        partitionMean(leftBB, items, leftItems);
        partitionMean(rightBB, items, rightItems);

        KDNode *left = buildMean(leftBB, leftItems, depth + 1);
        KDNode *right = buildMean(rightBB, rightItems, depth + 1);

        return new KDNode(left, right, split, dir);
    }
}

AABB KDTree::buildAABB(std::vector<PolygonAccel *> & items) {
    if (items.size() == 0)
        return AABB();

    AABB box = items[0]->getBBox();

    for (auto it = items.begin(); it != items.end(); it++)
        box.join((*it)->getBBox());

    return box;
}

bool KDTree::intersectLeaf(KDNode *leaf, Ray ray, Collision *result, float entry, float exit,
    bool anyCollision)
{
    bool found = false;
    Collision tmpResult;

    for (int i = 0; i < leaf->nItems; i++) {
        PolygonAccel *item = leaf->items[i];

        if (item->intersects(ray, &tmpResult) && tmpResult.distance >= entry &&
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

KDTree::KDTree(std::vector<PolygonAccel *> & items) {
    sceneBounds = buildAABB(items);
    root = buildMean(sceneBounds, items, 0);
}

bool KDTree::intersect(Ray ray, Collision *result, float maxDepth, bool anyCollision) {
    float entry, exit;

    if (!sceneBounds.intersects(ray, &entry, &exit))
        return false;

    std::vector<KDStackItem> stack;

    KDStackItem item(root, entry, exit);
    stack.push_back(item);

    KDNode *currentNode;

    while (stack.size() != 0) {
        KDStackItem top = stack[stack.size() - 1];
        stack.pop_back();

        currentNode = top.node;
        entry = top.enter;
        exit = top.exit;

        // Nothing will be closer, give up
        if (maxDepth > 0.0f && entry > maxDepth)
            return false;

        while (!(currentNode->left == NULL && currentNode->right == NULL)) {
            int dir = currentNode->dir;

            float split = currentNode->split;
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
                KDStackItem nextItem(farNode, t, exit);
                stack.push_back(nextItem);

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
