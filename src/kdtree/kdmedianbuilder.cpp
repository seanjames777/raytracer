/**
 * @file kdmedianbuilder.cpp
 *
 * @author Sean James
 */

#include <kdtree/kdmedianbuilder.h>

KDMedianBuilder::KDMedianBuilder() {
}

KDMedianBuilder::~KDMedianBuilder() {
}

bool KDMedianBuilder::splitNode(
    const AABB & bounds,
    const std::vector<Triangle *> & triangles,
    int depth,
    int & dir,
    float & split,
    enum PlanarMode & planarMode)
{
    if (depth >= 25 || triangles.size() < 4)
        return false;

#if 1
    // Split along longest axis
    vec3 axis_len = bounds.max - bounds.min;

    if (axis_len.x > axis_len.y)
        dir = axis_len.x > axis_len.z ? KD_SPLIT_DIR_X : KD_SPLIT_DIR_Z;
    else
        dir = axis_len.y > axis_len.z ? KD_SPLIT_DIR_Y : KD_SPLIT_DIR_Z;
#else
    // Split round robin
    dir = depth % 3;
#endif

    float min = bounds.min.v[dir];
    float max = bounds.max.v[dir];

    split = (max - min) / 2.0f + min;

    planarMode = PLANAR_BOTH;

    return true;
}