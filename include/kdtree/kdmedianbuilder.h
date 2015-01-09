/**
 * @file kdmedianbuilder.h
 *
 * @brief KD-Tree builder which simply splits each voxel along its spatial median
 * along some dimension. The dimension is chosen either by round-robin or by
 * choosing the longest dimension.
 *
 * @author Sean James
 */

#ifndef _KDMEDIANBUILDER_H
#define _KDMEDIANBUILDER_H

#include <kdtree/kdbuilder.h>

class KDMedianBuilder : public KDBuilder {
protected:

    virtual bool splitNode(const AABB & bounds, const std::vector<Triangle *> & triangles,
        int depth, int & dir, float & split, enum PlanarMode & planarMode) override;

public:

    KDMedianBuilder();

    virtual ~KDMedianBuilder();

};

#endif
