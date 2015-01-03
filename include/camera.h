/**
 * @file camera.h
 *
 * @brief Camera math
 *
 * @author Sean James
 */

#ifndef _CAMERA_H
#define _CAMERA_H

#include <rtmath.h>

/*
 * @brief Generic camera class that supports simple perspective projection
 */
class Camera {
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

    /** @brief Focal distance */
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
     * @param position Camera position
     * @param target   Camera target
     * @param aspect   Aspect ratio
     * @param fov      Field of view
     * @param focus    Focal distance
     * @param aperture Aperture radius. 0 for pinhole camera
     */
    Camera(const vec3 position, const vec3 target, float aspect, float fov,
        float focus, float aperture);

    /**
     * @brief Get a new view ray
     *
     * @param u        Horizontal coordinate of intersection point on view plane
     * @param v        Vertical coordinate of intersection point on view plane
     */
    Ray getViewRay(float u, float v);

    /**
     * @brief Get the camera position
     */
    vec3 getPosition();

    /**
     * @brief Set the camera position
     */
    void setPosition(const vec3 position);

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
     * @brief Get the camera target
     */
    vec3 getTarget();

    /**
     * @brief Get the field of view of the camera
     */
    float getFOV();

    /**
     * @brief Set the camera field of view
     */
    void setFOV(float fov);

    /**
     * @brief Get the aspect ratio of the camera
     */
    float getAspectRatio();

    /**
     * @brief Set the aspect ratio of the camera
     */
    void setAspectRatio(float aspectRatio);

    /**
     * @brief Get the focal length of the camera
     */
    float getFocalLength();

    /**
     * @brief Set the focal length of the camera
     */
    void setFocalLength(float focus);

    /**
     * @brief Get the aperture radius of the camera
     */
    float getAperture();

    /**
     * @brief Set the aperture radius of the camera
     */
    void setAperture(float aperture);

};

#endif
