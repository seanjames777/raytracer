/**
 * @file camera.cpp
 *
 * @author Sean James
 */

#include <camera.h>

void Camera::refresh() {
    forward = normalize(target - position);

    vec3 gup = vec3(0, 1, 0);

    right = normalize(-cross(forward, gup));
    up = normalize(cross(right, forward));

    halfWidth  = tanf(fov / 2.0f) * focus;
    halfHeight = halfWidth / aspect;
}

Camera::Camera(const vec3 position, const vec3 target, float aspect, float fov, float focus,
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

Ray Camera::getViewRay(float u, float v) {
    float x = u * 2.0f - 1.0f;
    float y = v * 2.0f - 1.0f;

    vec3 rightAmt = right * halfWidth  * x;
    vec3 upAmt    = up    * halfHeight * y;

    vec3 targ = forward * focus + rightAmt + upAmt;
    vec3 orig = position;

    if (aperture > 0.0f) {
        vec2 rCirc = randCircle(aperture);

        orig = right * rCirc.x + up * rCirc.y;
        targ = targ - orig;

        orig = orig + position;
    }

    targ = normalize(targ);

    return Ray(orig, targ);
}

vec3 Camera::getPosition() {
    return position;
}

void Camera::setPosition(const vec3 position) {
    this->position = position;
    refresh();
}

vec3 Camera::getForward() {
    return forward;
}

vec3 Camera::getRight() {
    return right;
}

vec3 Camera::getUp() {
    return up;
}

vec3 Camera::getTarget() {
    return target;
}

float Camera::getFOV() {
    return fov;
}

void Camera::setFOV(float fov) {
    this->fov = fov;
    refresh();
}

float Camera::getAspectRatio() {
    return aspect;
}

void Camera::setAspectRatio(float aspectRatio) {
    this->aspect = aspectRatio;
    refresh();
}

float Camera::getFocalLength() {
    return focus;
}

void Camera::setFocalLength(float focus) {
    this->focus = focus;
    refresh();
}

float Camera::getAperture() {
    return aperture;
}

void Camera::setAperture(float aperture) {
    this->aperture = aperture;
    refresh();
}
