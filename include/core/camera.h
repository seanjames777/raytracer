/**
 * @file core/camera.h
 *
 * @brief Look-at camera
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __CAMERA_H
#define __CAMERA_H

#include <math/ray.h>
#if 0
#include <math/sampling.h>
#endif
#include <rt_defs.h>

/*
 * @brief Perspective projection camera. TODO: other types: fisheye, ortho, etc.
 */
class RT_EXPORT Camera {
private:

    /** @brief Position */
    float3 position;

    /** @brief Right directiom */
    float3 right;

    /** @brief Up direction */
    float3 up;

	float3 forward;

    /** @brief Target position */
    float3 target;

    /** @brief Field of view */
    float fov;

    /** @brief Aspect ratio */
    float aspect;

    /** @brief Update values */
    void refresh();

public:

    /**
     * @brief Constructor
     *
     * @param[in] position Camera position
     * @param[in] target   Camera target
     * @param[in] aspect   Aspect ratio
     * @param[in] fov      Field of view
     * @param[in] focus    Focal length
     * @param[in] aperture Aperture radius. 0 for pinhole camera
     */
    Camera(
        const float3 & position,
        const float3 & target,
        float          fov);

    /**
     * @brief Get a ray from the camera sensor through the image plane
     *
     * @param[in] u      Normalized horizontal coordinate on image plane
     * @param[in] v      Normalized vertical coordinate on image plane
     * @param[in] weight Ray image contribution weight
     * @param[in] px     Output image X pixel coordinate
     * @param[in] py     Output image Y pixel coordinate
     * @param[in] depth  Raytracing recursion depth
     */
    Ray getViewRay(const float2 &uv) const;

    /**
     * @brief Get the camera position
     */
    float3 getPosition() const;

    /**
     * @brief Get the camera target position
     */
    float3 getTarget() const;

    /**
     * @brief Get the forward vector of the camera
     */
    float3 getForward() const;

    /**
     * @brief Get the right vector of the camera
     */
    float3 getRight() const;

    /**
     * @brief Get the up vector of the camera
     */
    float3 getUp() const;

    /**
     * @brief Get the field of view of the camera
     */
    float getFOV() const;

    /**
     * @brief Get the aspect ratio of the camera
     */
    float getAspectRatio() const;

    /**
     * @brief Set the camera position
     */
    void setPosition(const float3 & position);

    /**
     * @brief Set the camera target position
     */
    void setTarget(const float3 & target);

    /**
     * @brief Set the camera field of view
     */
    void setFOV(float fov);

    /**
     * @brief Set the aspect ratio of the camera
     */
    void setAspectRatio(float aspectRatio);

};

inline Ray Camera::getViewRay(const float2 & uv) const {
	// TODO: This tries to use SIMD efficiently, but the only way to get full SIMD utilization would be to
	// compute N samples at once and transpose the math operations

	// TODO: operation to swizzle x and y into all channels
    float2 xy = uv * 2.0f - 1.0f;
	
	// TODO: FMA
    float3 targ = forward + right * shuffle<3, 0, 0, 0, 0>(xy) + up * shuffle<3, 1, 1, 1, 1>(xy);

	// TODO: Is there a cheaper closed form for the reciprocal length?
    targ = normalize(targ);

    return Ray(position, targ);
}

inline float3 Camera::getPosition() const {
    return position;
}

inline float3 Camera::getTarget() const {
    return target;
}

inline float3 Camera::getForward() const {
    return target - position;
}

inline float3 Camera::getRight() const {
    return normalize(right); // TODO: if anyone seriously uses this, precompute the normalize version
}

inline float3 Camera::getUp() const {
    return normalize(up); // TODO: if anyone seriously uses this, precompute the normalize version
}

inline float Camera::getFOV() const {
    return fov;
}

inline float Camera::getAspectRatio() const {
    return aspect;
}

inline void Camera::setPosition(const float3 & position) {
    this->position = position;
    refresh();
}

inline void Camera::setTarget(const float3 & target) {
    this->target = target;
    refresh();
}

inline void Camera::setFOV(float fov) {
    this->fov = fov;
    refresh();
}

inline void Camera::setAspectRatio(float aspectRatio) {
    this->aspect = aspectRatio;
    refresh();
}

#endif
