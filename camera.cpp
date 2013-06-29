/*
 * Sean James
 *
 * camera.cpp
 *
 * Camera math
 *
 */

#include "camera.h"

void Camera::refresh() {
	forward = target - position;
	forward.normalize();

	Vec3 gup = Vec3(0, 1, 0);

	right = forward.cross(gup);
	up = right.cross(forward);
}

/*
 * Constructor specifying position, forward, aspect ratio, and field of view
 */
Camera::Camera(const Vec3 & Position, const Vec3 & Target, float Aspect, float Fov) 
	: position(Position), target(Target), aspect(Aspect), fov(Fov)
{
	refresh();
}

/*
 * Get the viewing ray by uv coordinate on the camera plane. If
 * NUMCAMRAYS is set higher than 1, than the origin will be offset
 * randomly according to the aperture setting for depth of field
 */
Ray Camera::getViewRay(float u, float v, float focus, float aperture, int numCamRays) {
	float halfWidth  = tanf(fov / 2.0f) * focus;
	float halfHeight = halfWidth / aspect;

	float x = u * 2.0f - 1.0f;
	float y = v * 2.0f - 1.0f;

	Vec3 rightAmt = right * halfWidth  * x;
	Vec3 upAmt    = up    * halfHeight * y;

	Vec3 targ = forward * focus + rightAmt + upAmt;

	Vec3 orig = position;

	if (numCamRays > 1) {
		Vec2 rCirc = randCircle(aperture);

		orig = right * rCirc.x + up * rCirc.y;
		targ = targ - orig;

		orig = orig + position;
	}

	targ.normalize();

	return Ray(orig, targ);
}

/*
 * Get the position of the camera
 */
Vec3 Camera::getPosition() {
	return position;
}

/*
 * Set the camera position
 */
void Camera::setPosition(const Vec3 & Position) {
	position = Position;
	refresh();
}

/*
 * Get the forward vector of the camera
 */
Vec3 Camera::getForward() {
	return forward;
}

/*
 * Get the right vector of the camera
 */
Vec3 Camera::getRight() {
	return right;
}

/*
 * Get the up vector of the camera
 */
Vec3 Camera::getUp() {
	return up;
}

/*
 * Get the field of view of the camera
 */
float Camera::getFOV() {
	return fov;
}

/*
 * Set the camera field of view
 */
void Camera::setFOV(float FOV) {
	fov = FOV;
}

/*
 * Get the aspect ratio of the camera
 */
float Camera::getAspectRatio() {
	return aspect;
}

/*
 * Set the aspect ratio of the camera
 */
void Camera::setAspectRatio(float AspectRatio) {
	aspect = AspectRatio;
}
