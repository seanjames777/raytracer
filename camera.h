/*
 * Sean James
 *
 * camera.h
 *
 * Camera math
 *
 */

#ifndef _CAMERA_H
#define _CAMERA_H

#include "rtmath.h"

/*
 * Generic camera class that supports simple perspective projection
 */
class Camera {
private:

	Vec3 position;
	Vec3 forward;
	Vec3 right;
	Vec3 up;
	Vec3 target;

	float fov;
	float aspect;

	void refresh();

public:

	/*
	 * Constructor specifying position, target, aspect ratio, and field of view
	 */
	Camera(const Vec3 & Position, const Vec3 & Target, float Aspect, float Fov);

	/*
	 * Get the viewing ray by uv coordinate on the camera plane. If
	 * NUMCAMRAYS is set higher than 1, than the origin will be offset
	 * randomly according to the aperture setting for depth of field
	 */
	Ray getViewRay(float u, float v, float focus, float aperture, int numCamRays);

	/*
	 * Get the position of the camera
	 */
	Vec3 getPosition();

	/*
	 * Set the camera position
	 */
	void setPosition(const Vec3 & Position);

	/*
	 * Get the forward vector of the camera
	 */
	Vec3 getForward();

	/*
	 * Get the right vector of the camera
	 */
	Vec3 getRight();

	/*
	 * Get the up vector of the camera
	 */
	Vec3 getUp();

	/*
	 * Get the field of view of the camera
	 */
	float getFOV();

	/*
	 * Set the camera field of view
	 */
	void setFOV(float FOV);

	/*
	 * Get the aspect ratio of the camera
	 */
	float getAspectRatio();

	/*
	 * Set the aspect ratio of the camera
	 */
	void setAspectRatio(float AspectRatio);
};

#endif
