/**
 * @file kdtree/kdsahbuilder.h
 *
 * @brief KD-Tree "surface area heuristic" builder
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __KDSAHBUILDER_H
#define __KDSAHBUILDER_H

#include <kdtree/kdbuilder.h>

/**
 * @brief SAH builder triangle event type enumeration
 */
enum SAHEventType {
    SAH_END    = 0, //!< Triangle end
    SAH_PLANAR = 1, //!< Trianlge lying in plane
    SAH_BEGIN  = 2  //!< Triangle start
};

/**
 * @brief SAH builder triangle event
 */
struct SAHEvent {
    enum SAHEventType flag; //!< Event type
    float dist;             //!< Location along split axis
};

/**
 * @brief KD-tree SAH builder thread context object
 */
struct KDSAHBuilderThreadCtx {
    SAHEvent *events;         //!< Reuseable event list. One thread processes one list at a time.
    int       event_capacity; //!< Capacity of event list
};

/**
 * @brief KD-tree builder which splits nodes according to the Surface Area Heuristic
 */
class KDSAHBuilder : public KDBuilder {
private:

    float k_traversal; //!< Cost of traversing a KD-tree node
    float k_intersect; //!< Cost of intersecting a triangle

protected:

    /**
     * @copydoc KDBuilder::prepareWorkerThread
     */
    virtual void *prepareWorkerThread(int idx) override;

    /**
     * @copydoc KDBuilder::destroyWorkerThread
     */
    virtual void destroyWorkerThread(void *threadCtx) override;

    /**
     * @copydoc KDBuilder::splitNode
     */
    virtual bool splitNode(
        void                          * threadCtx,
        const AABB                    & bounds,
        const std::vector<Triangle *> & triangles,
        int                             depth,
        float                         & split,
        int                           & dir,
        enum KDBuilderPlanarMode      & planarMode) override;

public:

    /**
     * @brief Constructor
     *
     * @param[in] k_traversal Cost of traversing a KD-tree node
     * @param[in] k_intersect Cost of intersecting a KD-tree node
     */
    KDSAHBuilder(float k_traversal = 1.0f, float k_intersect = 0.5f);

    /**
     * @brief Destructor
     */
    virtual ~KDSAHBuilder();

};

#endif
