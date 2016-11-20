/**
 * @file kdtree/kdmedianbuilder.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <kdtree/kdmedianbuilder.h>

KDMedianBuilder::KDMedianBuilder() {
}

KDMedianBuilder::~KDMedianBuilder() {
}

bool KDMedianBuilder::splitNode(
    void                          * threadCtx,
    const AABB                    & bounds,
    const std::vector<Triangle *> & triangles,
    int                             depth,
    float                         & split,
    int                           & dir,
    enum KDBuilderPlanarMode      & planarMode)
{
    if (depth >= 25 || triangles.size() < 4)
        return false;

#if 1
    // Split along longest axis
    float3 axis_len = bounds.max - bounds.min;

    if (axis_len.x > axis_len.y)
        dir = axis_len.x > axis_len.z ? KD_INTERNAL_X : KD_INTERNAL_Z;
    else
        dir = axis_len.y > axis_len.z ? KD_INTERNAL_Y : KD_INTERNAL_Z;
#else
    // Split round robin
    dir = depth % 3;
#endif

    float min = bounds.min[dir];
    float max = bounds.max[dir];

    split = (max - min) / 2.0f + min;

    planarMode = PLANAR_BOTH;

    return true;
}
