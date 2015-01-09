/**
 * @file kdbuilder.cpp
 *
 * @author Sean James
 */

#include <kdtree/kdbuilder.h>
#include <iostream>
#include <sys/time.h>

KDBuilder::KDBuilder() {
}

KDBuilder::~KDBuilder() {
}

void KDBuilder::partition(float dist, int dir, const std::vector<Triangle *> & triangles,
        std::vector<Triangle *> & left, std::vector<Triangle *> & right,
        enum PlanarMode & planarMode)
{
    for (auto it = triangles.begin(); it != triangles.end(); it++) {
        Triangle *tri = *it;

        float min = tri->bbox.min.v[dir];
        float max = tri->bbox.max.v[dir];

        if (min == dist && max == dist) {
            // TODO bitwise optimization
            if (planarMode == PLANAR_LEFT)
                left.push_back(tri);
            else if (planarMode == PLANAR_RIGHT)
                right.push_back(tri);
            else {
                left.push_back(tri);
                right.push_back(tri);
            }
        }
        else {
            if (min < dist)
                left.push_back(tri);

            if (max > dist)
                right.push_back(tri);
        }
    }
}

KDNode *KDBuilder::buildLeaf(const std::vector<Triangle *> & triangles) {
    // Set up triangles, and pack them together for locality
    unsigned int num_triangles = (unsigned int)triangles.size();
    SetupTriangle *setup = (SetupTriangle *)malloc(sizeof(SetupTriangle) * num_triangles);

    for (unsigned int i = 0; i < num_triangles; i++)
        setup[i] = SetupTriangle(*triangles[i]); // TODO constructor in place

    // TODO: Might want to change constructor of KDNode to do this stuff,
    // especially to not mix allocations

    // TODO: handle overflow of size?

    return new KDNode(NULL, NULL, 0.0f, setup, num_triangles | KD_IS_LEAF);
}

KDNode *KDBuilder::buildNode(const AABB & bounds, const std::vector<Triangle *> & triangles, int depth) {
    // TODO: possibly traverse after construction to remove useless cells, etc.

    int dir;
    float split;
    enum PlanarMode planarMode;

    if (splitNode(bounds, triangles, depth, dir, split, planarMode)) {
        float min = bounds.min.v[dir];

        AABB leftBB, rightBB;
        bounds.split(split, dir, leftBB, rightBB);

        // TODO might want to reserve space to avoid allocations
        std::vector<Triangle *> leftContained;
        std::vector<Triangle *> rightContained;
        partition(split, dir, triangles, leftContained, rightContained, planarMode);

        KDNode *left = buildNode(leftBB, leftContained, depth + 1);
        KDNode *right = buildNode(rightBB, rightContained, depth + 1);

        return new KDNode(left, right, split, NULL, (KDNodeFlags)dir);
    }
    else
        return buildLeaf(triangles);
}

AABB KDBuilder::buildAABB(const std::vector<Triangle *> & triangles) {
    if (triangles.size() == 0)
        return AABB();

    AABB box = triangles[0]->bbox;

    for (auto it = triangles.begin(); it != triangles.end(); it++)
        box.join((*it)->bbox);

    return box;
}

KDTree *KDBuilder::build(const std::vector<Triangle> & triangles) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long long start = tv.tv_sec * 1000000 + tv.tv_usec;

    // Use pointers while building the tree, because we need to be able to sort, etc.
    // Triangles are converted to "setup" triangles, so we don't actually need the
    // triangle data anyway.
    std::vector<Triangle *> pointers;

    pointers.reserve(triangles.size());

    for (auto & it : triangles)
        pointers.push_back(const_cast<Triangle *>(&it));

    AABB bounds = buildAABB(pointers);

    KDNode *root = buildNode(bounds, pointers, 0);

    // TODO
    gettimeofday(&tv, NULL);
    unsigned long long end = tv.tv_sec * 1000000 + tv.tv_usec;
    std::cout << "KD Build Time: " << (end - start) / 1000.0f << "ms" << std::endl;

    return new KDTree(root, bounds);
}
