/**
 * @file kdtree/kdmedianbuilder.h
 *
 * @brief KD-tree "spatial median" builder
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __KDMEDIANBUILDER_H
#define __KDMEDIANBUILDER_H

#include <kdtree/kdbuilder.h>

/**
 * @brief KD-tree builder which splits nodes along their spatial median. The split
 * axis is chosen by round-robin or by choosing the longest dimension.
 */
class KDMedianBuilder : public KDBuilder {
protected:

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
     */
    KDMedianBuilder();

    /**
     * @brief Destructor
     */
    virtual ~KDMedianBuilder();

};

#endif
