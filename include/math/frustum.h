#ifndef __MATH_FRUSTUM_H
#define __MATH_FRUSTUM_H

#include <math/ray.h>
#include <math/matrix.h>

struct Frustum {

    float3 corners[8];
    Plane planes[6];

    Frustum() {
    }

    Frustum(const float4x4 & mat) {
        float4x4 invMat = inverse(mat);

        float4 min(-1,  1, 0, 1);
        float4 max( 1, -1, 1, 1);

        min = invMat * min;
        min = min / min.w;

        max = invMat * max;
        max = max / max.w;

        corners[0] = float3( min.x,  min.y, min.z);
        corners[1] = float3(-min.x,  min.y, min.z);
        corners[2] = float3( min.x, -min.y, min.z);
        corners[3] = float3(-min.x, -min.y, min.z);

        corners[4] = float3(-max.x, -max.y, max.z);
        corners[5] = float3( max.x, -max.y, max.z);
        corners[6] = float3(-max.x,  max.y, max.z);
        corners[7] = float3( max.x,  max.y, max.z);
    }

};

#endif