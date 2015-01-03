/**
 * @file polygon.cpp
 *
 * @author Sean James
 */

#include <polygon.h>

// TODO Align me
static const int modlookup[5] = { 0, 1, 2, 0, 1 }; // TODO: questionable optimization

Vertex::Vertex() {
}

Vertex::Vertex(vec3 position, vec3 normal, vec2 uv, vec4 color)
    : position(position),
      normal(normal),
      uv(uv),
      color(color)
{
}

Collision::Collision()
    : distance(INFINITY32F)
{
}

PolygonAccel::PolygonAccel(vec3 v1, vec3 v2, vec3 v3, unsigned int polygonID)
    : polygonID(polygonID)
{
    // Edges and normal
    vec3 b = v3 - v1;
    vec3 c = v2 - v1;
    vec3 n = cross(c, b);

    // Choose which dimension to project
    if (fabs(n.x) > fabs(n.y))
        k = fabs(n.x) > fabs(n.z) ? 0 : 2;
    else
        k = fabs(n.y) > fabs(n.z) ? 1 : 2;

    int u = modlookup[k + 1];
    int v = modlookup[k + 2];

    n = n / n.v[k];

    n_u = n.v[u];
    n_v = n.v[v];
    n_d = dot(v1, n);

    float denom = b.v[u] * c.v[v] - b.v[v] * c.v[u];
    b_nu = -b.v[v] / denom;
    b_nv =  b.v[u] / denom;
    b_d  =  (b.v[v] * v1.v[u] - b.v[u] * v1.v[v]) / denom;

    c_nu =  c.v[v] / denom;
    c_nv = -c.v[u] / denom;
    c_d  =  (c.v[u] * v1.v[v] - c.v[v] * v1.v[u]) / denom;

    AABB box(v1, v1);
    box.join(v2);
    box.join(v3);

    min = box.min;
    max = box.max;
}

AABB PolygonAccel::getBBox() {
    return AABB(min, max);
}

bool PolygonAccel::intersects(Ray ray, Collision *result) {
    // http://www.sci.utah.edu/~wald/PhD/wald_phd.pdf

    int u = modlookup[k + 1];
    int v = modlookup[k + 2];

    const float dot = (ray.direction.v[k] + n_u * ray.direction.v[u] + n_v *
        ray.direction.v[v]);

    if (dot == 0.0f)
        return false;

    const float nd = 1.0f / dot;
    const float t_plane = (n_d - ray.origin.v[k]
        - n_u * ray.origin.v[u] - n_v * ray.origin.v[v]) * nd;

    // Behind camera
    if (t_plane <= 0.0f)
        return false;

    const float hu = ray.origin.v[u] + t_plane * ray.direction.v[u];
    const float hv = ray.origin.v[v] + t_plane * ray.direction.v[v];

    const float beta  = (hu * b_nu + hv * b_nv + b_d);
    if (beta < 0.0f)
        return false;

    const float gamma = (hu * c_nu + hv * c_nv + c_d);
    if (gamma < 0.0f)
        return false;

    if (beta + gamma > 1.0f)
        return false;

    result->distance = t_plane;
    result->beta = beta;
    result->gamma = gamma;
    result->polygonID = polygonID;

    return true;
}

Polygon::Polygon() {
}

Polygon::Polygon(Vertex v1, Vertex v2, Vertex v3)
    : v1(v1),
      v2(v2),
      v3(v3)
{
    vec3 b = normalize(v3.position - v1.position);
    vec3 c = normalize(v2.position - v1.position);

    normal = normalize(cross(c, b));
}

void Polygon::getCollisionEx(Ray ray, Collision *collision, CollisionEx *collisionEx,
    bool interpolate)
{
    collisionEx->ray = ray;
    collisionEx->position = ray.at(collision->distance);
    collisionEx->polyNormal = normal;

    if (interpolate) {
        float alpha = 1.0f - collision->beta - collision->gamma;

        vec3 an = v1.normal * alpha;
        vec3 bn = v2.normal * collision->beta;
        vec3 cn = v3.normal * collision->gamma;
        collisionEx->normal = an + bn + cn;

        vec2 auv = v1.uv * alpha;
        vec2 buv = v2.uv * collision->beta;
        vec2 cuv = v3.uv * collision->gamma;
        collisionEx->uv = auv + buv + cuv;

        vec4 acolor = v1.color * alpha;
        vec4 bcolor = v2.color * collision->beta;
        vec4 ccolor = v3.color * collision->gamma;
        collisionEx->color = acolor + bcolor + ccolor;
    }
}
