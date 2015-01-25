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
    SAH_END    = 1, // Triangle end
    SAH_PLANAR = 2, // Trianlge lying in plane
    SAH_BEGIN  = 4  // Triangle start
};

struct SAHEvent {
    enum SAHEventType flag; // Event type
    float dist;             // Split location
};

class KDSAHBuilder : public KDBuilder {
private:

    // Because we process one node at a time, we can reuse one event list allocation
    SAHEvent *events;
    int       event_capacity;

protected:

    virtual bool splitNode(const AABB & bounds, const std::vector<Triangle *> & triangles,
        int depth, int & dir, float & split, enum PlanarMode & planarMode) override;

public:

    KDSAHBuilder();

    virtual ~KDSAHBuilder();

};

#endif
