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
#include <math/sampling.h>
#include <rt_defs.h>

/*
 * @brief Perspective projection camera. TODO: other types: fisheye, ortho, etc.
 */
class RT_EXPORT Camera {
private:

    /** @brief Position */
    vec3 position;

    /** @brief Forward direction */
    vec3 forward;

    /** @brief Right directiom */
    vec3 right;

    /** @brief Up direction */
    vec3 up;

    /** @brief Target position */
    vec3 target;

    /** @brief Field of view */
    float fov;

    /** @brief Aspect ratio */
    float aspect;

    /** @brief Focal length */
    float focus;

    /** @brief Aperture radius */
    float aperture;

    /** @brief Half the width of the view plane */
    float halfWidth;

    /** @brief Half the height of the view plane */
    float halfHeight;

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
        const vec3 & position,
        const vec3 & target,
        float        aspect,
        float        fov,
        float        focus,
        float        aperture);

    // TODO
#if 0
    /**
     * @brief Get samples on virtual sensor, jittered across a sample range.
     *
     * @param[in]  nSamples Square root of number of samples to get
     * @param[out] samples  Output samples
     * @param[in]  min      Minimum
     * @param[in]  max
     *
     * @return Returns false if this is a pinhole camera, in which case there will only be one
     * output sample at the center of the sample range. Otherwise, returns true and fills in
     * nSamples * nSamples output samples.
     */
    bool getSamples(int nSamples, vec2 *samples, const vec2 & min, const vec2 & max);
#endif

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
    Ray getViewRay(
        const vec2 &uv,
        const vec3 &weight,
        short       px,
        short       py,
        char        depth) const;

    /**
     * @brief Get the camera position
     */
    vec3 getPosition();

    /**
     * @brief Get the camera target position
     */
    vec3 getTarget();

    /**
     * @brief Get the forward vector of the camera
     */
    vec3 getForward();

    /**
     * @brief Get the right vector of the camera
     */
    vec3 getRight();

    /**
     * @brief Get the up vector of the camera
     */
    vec3 getUp();

    /**
     * @brief Get the field of view of the camera
     */
    float getFOV();

    /**
     * @brief Get the aspect ratio of the camera
     */
    float getAspectRatio();

    /**
     * @brief Get the focal length of the camera
     */
    float getFocalLength();

    /**
     * @brief Get the aperture radius of the camera
     */
    float getAperture();

    /**
     * @brief Set the camera position
     */
    void setPosition(const vec3 & position);

    /**
     * @brief Set the camera target position
     */
    void setTarget(const vec3 & target);

    /**
     * @brief Set the camera field of view
     */
    void setFOV(float fov);

    /**
     * @brief Set the aspect ratio of the camera
     */
    void setAspectRatio(float aspectRatio);

    /**
     * @brief Set the focal length of the camera
     */
    void setFocalLength(float focus);

    /**
     * @brief Set the aperture radius of the camera
     */
    void setAperture(float aperture);

};

inline Ray Camera::getViewRay(
    const vec2 &uv,
    const vec3 &weight,
    short       px,
    short       py,
    char        depth) const
{
    // TODO: Lens coordinates

    float x = uv.x * 2.0f - 1.0f;
    float y = uv.y * 2.0f - 1.0f;

    vec3 rightAmt = right * halfWidth  * x; // TODO: Premultiply right/up
    vec3 upAmt    = up    * halfHeight * y;

    vec3 targ = forward * focus + rightAmt + upAmt;
    targ = normalize(targ);

    return Ray(position, targ, weight, 0.0f, px, py, depth, false);
}

inline vec3 Camera::getPosition() {
    return position;
}

inline vec3 Camera::getTarget() {
    return target;
}

inline vec3 Camera::getForward() {
    return forward;
}

inline vec3 Camera::getRight() {
    return right;
}

inline vec3 Camera::getUp() {
    return up;
}

inline float Camera::getFOV() {
    return fov;
}

inline float Camera::getAspectRatio() {
    return aspect;
}

inline float Camera::getFocalLength() {
    return focus;
}

inline float Camera::getAperture() {
    return aperture;
}

inline void Camera::setPosition(const vec3 & position) {
    this->position = position;
    refresh();
}

inline void Camera::setTarget(const vec3 & target) {
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

inline void Camera::setFocalLength(float focus) {
    this->focus = focus;
    refresh();
}

inline void Camera::setAperture(float aperture) {
    this->aperture = aperture;
    refresh();
}

#endif
