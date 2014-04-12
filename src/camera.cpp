/**
 * @file camera.cpp
 *
 * @author Sean James
 */

#include <camera.h>

void Camera::refresh() {
	forward = target - position;
	forward.normalize();

	Vec3 gup = Vec3(0, 1, 0);

	right = -forward.cross(gup);
	right.normalize();

	up = right.cross(forward);
	up.normalize();

	halfWidth  = tanf(fov / 2.0f) * focus;
	halfHeight = halfWidth / aspect;
}

Camera::Camera(const Vec3 position, const Vec3 target, float aspect, float fov, float focus,
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

	Vec3 rightAmt = right * halfWidth  * x;
	Vec3 upAmt    = up    * halfHeight * y;

	Vec3 targ = forward * focus + rightAmt + upAmt;
	Vec3 orig = position;

	if (aperture > 0.0f) {
		Vec2 rCirc = randCircle(aperture);

		orig = right * rCirc.x + up * rCirc.y;
		targ = targ - orig;

		orig = orig + position;
	}

	targ.normalize();

	return Ray(orig, targ);
}

Vec3 Camera::getPosition() {
	return position;
}

void Camera::setPosition(const Vec3 position) {
	this->position = position;
	refresh();
}

Vec3 Camera::getForward() {
	return forward;
}

Vec3 Camera::getRight() {
	return right;
}

Vec3 Camera::getUp() {
	return up;
}

Vec3 Camera::getTarget() {
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
