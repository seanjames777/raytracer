/**
 * @file kdtree/kdsahbuilder.h
 *
 * @brief KD-Tree builder which splits voxels based on the "surface area heuristic"
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __KDSAHBUILDER_H
#define __KDSAHBUILDER_H

#include <kdtree/kdbuilder.h>

enum SAHEventType {
    SAH_END    = 0, // Triangle end
    SAH_PLANAR = 1, // Trianlge lying in plane
    SAH_BEGIN  = 2  // Triangle start
};

struct SAHEvent {
    enum SAHEventType flag; // Event type
    float dist;             // Split location
};

struct KDSAHBuilderThreadCtx {
    // Because we process one node at a time, we can reuse one event list allocation
    SAHEvent *events;
    int       event_capacity;
};

class KDSAHBuilder : public KDBuilder {
protected:

    virtual void *prepareWorkerThread(int idx) override;

    virtual void destroyWorkerThread(void *threadCtx) override;

    virtual bool splitNode(void *threadCtx, const AABB & bounds, const std::vector<Triangle *> & triangles,
        int depth, int & dir, float & split, enum PlanarMode & planarMode) override;

public:

    KDSAHBuilder();

    virtual ~KDSAHBuilder();

};

#endif
