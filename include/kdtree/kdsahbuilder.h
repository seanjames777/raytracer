/**
 * @file kdsahbuilder.h
 *
 * @brief KD-Tree builder which splits voxels based on the "surface area heuristic"
 *
 * @author Sean James
 */

#ifndef _KDSAHBUILDER_H
#define _KDSAHBUILDER_H

#include <kdtree/kdbuilder.h>

class KDSAHBuilder : public KDBuilder {
protected:

    virtual bool splitNode(const AABB & bounds, const std::vector<Triangle *> & triangles,
        int depth, int & dir, float & split, enum PlanarMode & planarMode) override;

public:

    KDSAHBuilder();

    virtual ~KDSAHBuilder();

};

#endif
