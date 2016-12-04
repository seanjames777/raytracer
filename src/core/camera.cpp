/**
 * @file core/camera.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <core/camera.h>

Camera::Camera(
    const float3 & position,
    const float3 & target,
    float fov,
    float aperture,
    float focalLength)
    : position(position),
      target(target),
      aspect(1.0f),
      fov(fov),
      aperture(aperture),
      focalLength(focalLength)
{
    refresh();
}

void Camera::refresh() {
    forward = normalize(target - position);

    // TODO: Handle pointing along axes
    float3 gup = float3(0, 1, 0);
    right = normalize(-cross(forward, gup));
    up = normalize(cross(right, forward));

    halfWidth  = focalLength * tanf(fov / 2.0f);
    halfHeight = halfWidth / aspect;
}
