/**
 * @file core/camera.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <core/camera.h>

Camera::Camera(
    const float3 & position,
    const float3 & target,
    float fov)
    : position(position),
      target(target),
      aspect(1.0f),
      fov(fov)
{
    refresh();
}

void Camera::refresh() {
    forward = normalize(target - position);

    float3 gup = float3(0, 1, 0);

    right = normalize(-cross(forward, gup));
    up = normalize(cross(right, forward));

    // TODO: Handle pointing along axes

    float halfWidth  = tanf(fov / 2.0f);
    float halfHeight = halfWidth / aspect;

	right = right * halfWidth;
	up = up * halfHeight;
}
