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
void setupTriangles(
    const util::vector<Triangle, 16> & triangles, 
    util::vector<SetupTriangle, 16>  & setupTriangles)
{
#if defined(WALD_INTERSECTION)
    // TODO: should this be aligned (and possibly padded) to a cache line to
    // make sure it only requires one memory request?
    static const int mod_table[5] = { 0, 1, 2, 0, 1 };
    
    for (auto & tri : triangles) {
        SetupTriangle setup;
        
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

        setupTriangles.push_back(setup);
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

int clip(float3 *input,
         float3 *output,
         float   plane,
         int     axis,
         int     keep)
{
    int count = 0;
 
    // TODO: Eliminate branch, function call
    // TODO: Could use SIMD

    if (keep & 0x1) {
        float3 vertices[4] = {};
        int vertex = 0;
        
        for (int edge = 0; edge < 3; edge++) {
            int i = edge;
            int j = (edge + 1) % 3;
            
            if (input[i][axis] == plane) {
                vertices[vertex++][i] = 1;
            }
            else if (input[i][axis] > plane) {
                vertices[vertex++][i] = 1;
                
                // Edge exits
                if (input[j][axis] < plane) {
                    float t = (plane - input[j][axis]) / (input[i][axis] - input[j][axis]);
                    vertices[vertex][j] = 1 - t;
                    vertices[vertex++][i] = t;
                }
            }
            else {
                // Edge enters
                if (input[j][axis] > plane) {
                    float t = (plane - input[i][axis]) / (input[j][axis] - input[i][axis]);
                    vertices[vertex][i] = 1 - t;
                    vertices[vertex++][j] = t;
                }
                ;
                // Edge outside
            }
        }
        
        if (vertex >= 3) {
            output[count++] = vertices[0];
            output[count++] = vertices[1];
            output[count++] = vertices[2];
            
            if (vertex == 4) {
                output[count++] = vertices[0];
                output[count++] = vertices[2];
                output[count++] = vertices[3];
            }
        }
    }
    
    if (keep & 0x2) {
        float3 vertices[4] = {};
        int vertex = 0;
        
        for (int edge = 0; edge < 3; edge++) {
            int i = edge;
            int j = (edge + 1) % 3;
            
            if (input[i][axis] == plane) {
                // Edge inside
                vertices[vertex++][i] = 1; // TODO
            }
            else if (input[i][axis] > plane) {
                // Edge exits
                if (input[j][axis] < plane) {
                    float t = (plane - input[j][axis]) / (input[i][axis] - input[j][axis]);
                    vertices[vertex][j] = 1 - t;
                    vertices[vertex++][i] = t;
                }
                
                // Edge inside
            }
            else {
                vertices[vertex++][i] = 1;
                
                // Edge enters
                if (input[j][axis] > plane) {
                    float t = (plane - input[i][axis]) / (input[j][axis] - input[i][axis]);
                    vertices[vertex][i] = 1 - t;
                    vertices[vertex++][j] = t;
                }
            }
        }
        
        if (vertex >= 3) {
            output[count++] = vertices[0];
            output[count++] = vertices[1];
            output[count++] = vertices[2];
            
            if (vertex == 4) {
                output[count++] = vertices[0];
                output[count++] = vertices[2];
                output[count++] = vertices[3];
            }
        }
    }
    
    return count / 3;
}
