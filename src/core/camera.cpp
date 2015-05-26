/**
 * @file core/camera.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <core/camera.h>

Camera::Camera(
    const vec3 & position,
    const vec3 & target,
    float aspect,
    float fov,
    float focus,
    float aperture)
    : position(position),
      target(target),
      aspect(aspect),
      fov(fov),
      focus(focus),
      aperture(aperture)
{
    refresh();
}

void Camera::refresh() {
    forward = normalize(target - position);

    vec3 gup = vec3(0, 1, 0);

    right = normalize(-cross(forward, gup));
    up = normalize(cross(right, forward));

    // TODO: Handle pointing along axes

    halfWidth  = tanf(fov / 2.0f) * focus;
    halfHeight = halfWidth / aspect;
}

// TODO
#if 0
bool Camera::getSamples(int nSamples, vec2 *samples, const vec2 & min, const vec2 & max) {
    // TODO: Make sure samples align with pixel grid in a nice way

    if (aperture == 0.0f) {
        samples[0] = (min + max) * 0.5f;
        return false;
    }

    randJittered2D(nSamples, samples);
    mapSamplesDisk(nSamples, samples);

    return true;
}
#endif
