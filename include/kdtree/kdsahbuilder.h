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

enum SAHEventType {
    SAH_END = 0,    // Triangle end
    SAH_PLANAR = 1, // Trianlge lying in plane
    SAH_BEGIN = 2   // Triangle start
};

struct SAHEvent {
    enum SAHEventType flag; // Event type
    float dist;             // Split location
};

class KDSAHBuilder : public KDBuilder {
private:

    std::vector<SAHEvent> events;

protected:

    virtual bool splitNode(const AABB & bounds, const std::vector<Triangle *> & triangles,
        int depth, int & dir, float & split, enum PlanarMode & planarMode) override;

public:

    KDSAHBuilder();

    virtual ~KDSAHBuilder();

};

#endif
