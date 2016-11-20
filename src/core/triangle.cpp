/**
 * @file core/triangle.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <core/triangle.h>
#include <core/triangle.inl>

#include <string.h>
#include <util/align.h>

/**
 * @brief Pack triangle data into setup triangle data
 *
 * @param[in] triangles     An array of pointers to unpacked triangles
 * @param[in] num_triangles Number of triangles to pack
 */
void setupTriangles(Triangle **triangles, SetupTriangle *data, int num_triangles)
{
#if defined(WALD_INTERSECTION)
    // TODO: should this be aligned (and possibly padded) to a cache line to
    // make sure it only requires one memory request?
    static const int mod_table[5] = { 0, 1, 2, 0, 1 };
    
    for (int i = 0; i < num_triangles; i++) {
        const Triangle & tri = *(triangles[i]);
        SetupTriangle & setup = data[i];
        
        const float3 & v0 = tri.v[0].position;
        const float3 & v1 = tri.v[1].position;
        const float3 & v2 = tri.v[2].position;
        
        // Edges and normal
        float3 b = v2 - v0;
        float3 c = v1 - v0;
        float3 n = cross(c, b);
        
        // Choose which dimension to project
        if (fabs(n.x) > fabs(n.y))
            setup.k = fabs(n.x) > fabs(n.z) ? 0 : 2;
        else
            setup.k = fabs(n.y) > fabs(n.z) ? 1 : 2;
        
        int u = mod_table[setup.k + 1]; // TODO %
        int v = mod_table[setup.k + 2];
        
        n = n / n[setup.k];
        
        setup.n_u = n[u];
        setup.n_v = n[v];
        setup.n_d = dot(v0, n);
        
        // TODO: inv_denom
        
        float denom = b[u] * c[v] - b[v] * c[u];
        setup.b_nu = -b[v] / denom;
        setup.b_nv = b[u] / denom;
        setup.b_d = (b[v] * v0[u] - b[u] * v0[v]) / denom;
        
        setup.c_nu = c[v] / denom;
        setup.c_nv = -c[u] / denom;
        setup.c_d = (c[u] * v0[v] - c[v] * v0[u]) / denom;
        
        setup.triangle_id = tri.triangle_id;
    }
#elif defined(MOLLER_TRUMBORE_INTERSECTION)
    for (int i = 0; i < num_triangles; i++) {
        const Triangle & tri = *(triangles[i]);
        SetupTriangle & setup = data[i];
        
        setup.v[0] = tri.v[0].position;
        setup.e1 = tri.v[1].position - tri.v[0].position;
        setup.e2 = tri.v[2].position - tri.v[0].position;
        setup.triangle_id = tri.triangle_id;
    }
#endif
}
