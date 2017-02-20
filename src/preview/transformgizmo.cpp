#include <preview/transformgizmo.h>
#include <math/plane.h>

void transformGizmo(
    float4x4 & transform,
    const float4x4 & viewProjection,
    bool mouseDown,
    const float2 & mousePos,
    const float2 & viewPos,
    const float2 & viewSize,
    TransformGizmoState & state,
    std::vector<SolidVertex> & vertices,
    std::vector<SolidDrawCmd> & commands)
{
    float4x4 tp = transpose(transform);

    float3 c0 = tp.rows[0].xyz();
    float3 c1 = tp.rows[1].xyz();
    float3 c2 = tp.rows[2].xyz();

    float3 origin = tp.rows[3].xyz();

    float3 scale(length(c0), length(c1), length(c2));

    c0 = c0 / scale.x;
    c1 = c1 / scale.y;
    c2 = c2 / scale.z;

    float3x3 rotation;
    rotation.rows[0] = c0;
    rotation.rows[1] = c1;
    rotation.rows[2] = c2;
    rotation = transpose(rotation);

    float axisLength = 0.0f;

    float4x4 invViewProjection = inverse(viewProjection);

    // Compute gizmo scale
    {
        // Transform gizmo position into screen space
        float4 positionNDC(origin, 1);
        positionNDC = viewProjection * positionNDC;
        positionNDC = positionNDC / positionNDC.w;

        // Create another point a constant screen space distance away
        float desiredSize = state.pixelSize / viewSize.x * 2.0f;
        float4 offsetPosNDC = positionNDC;
        offsetPosNDC.x += desiredSize;

        // Unproject second point
        float4 offsetPos = invViewProjection * offsetPosNDC;
        offsetPos = offsetPos / offsetPos.w;

        // Get world space distance between two points
        axisLength = length(offsetPos.xyz() - origin);
    }

    float axisRadius = 0.022f * axisLength;
    float quadLength = axisLength * 0.33f;

    Ray ray;

    // Compute view ray
    {
        float2 ndc = (mousePos - viewPos) / viewSize;
        ndc = ndc * 2.0f - 1.0f;
        ndc.y = -ndc.y;

        float4 near(ndc, 0.0f, 1.0f); // TODO: -1 or 0 ?
        float4 far(ndc, 1.0f, 1.0f);

        near = invViewProjection * near;
        far = invViewProjection * far;
        near = near / near.w;
        far = far / far.w;

        ray = Ray(near.xyz(), normalize(far.xyz() - near.xyz()));
    }

    float3 axes[3] = {
        float3(1, 0, 0),
        float3(0, 1, 0),
        float3(0, 0, 1)
    };

    // Don't allow world space scaling because it leads to skewing
    if (state.local || state.mode == TransformModeScale)
        for (int i = 0; i < 3; i++)
            axes[i] = rotation * axes[i];

    Plane planes[3] = {
        Plane(axes[0], dot(origin, axes[0])),
        Plane(axes[1], dot(origin, axes[1])),
        Plane(axes[2], dot(origin, axes[2]))
    };

    float3 axisColors[3];
    float3 quadColors[3];

    for (int i = 0; i < 3; i++) {
        axisColors[i] = float3(i == 0 ? 1.0f : 0.0f, i == 1 ? 1.0f : 0.0f, i == 2 ? 1.0f : 0.0f);
        quadColors[i] = float3(i == 0 ? 1.0f : 0.0f, i == 1 ? 1.0f : 0.0f, i == 2 ? 1.0f : 0.0f);
    }

    if (state.active) {
        if (!mouseDown) {
            state.active = false;
        }
        else {
            float hitDist;

            if (planes[state.selectedPlane].intersects(ray, hitDist)) {
                float3 hitPos = ray.at(hitDist) - origin;

                float3 delta = hitPos - state.selectionOffset;
                float3 constrainedDelta = 0;

                // TODO: can just filter out the unselected axes directly
                for (int i = 0; i < 3; i++) {
                    if ((1 << i) & state.selectedAxes)
                        constrainedDelta = constrainedDelta + dot(axes[i], delta) * axes[i];
                }

                if (state.mode == TransformModeTranslation) {
                    origin = constrainedDelta + origin;
                    // Does not affect offset
                }
                else if (state.mode == TransformModeScale) {
                    float3 scaleAmount(1, 1, 1);

                    for (int i = 0; i < 3; i++) {
                        if ((1 << i) & state.selectedAxes)
                            scaleAmount[i] = max(dot(axes[i], hitPos), 0.00001f) / max(dot(axes[i], state.selectionOffset), 0.00001f);
                    }
                    scale = scaleAmount * scale;

                    // Scales offset
                    state.selectionOffset = scaleAmount[0] * dot(state.selectionOffset, axes[0]) * axes[0] +
                                     scaleAmount[1] * dot(state.selectionOffset, axes[1]) * axes[1] +
                                     scaleAmount[2] * dot(state.selectionOffset, axes[2]) * axes[2];
                }
                else if (state.mode == TransformModeRotation) {
                    float3 S_norm = normalize(state.selectionOffset);

                    float3 HonS = dot(hitPos, S_norm) * S_norm;
                    float3 HoffS = hitPos - HonS;

                    float theta = atan2(length(HoffS), length(HonS));

                    if (dot(cross(S_norm, hitPos), planes[state.selectedPlane].normal) < 0.0f)
                        theta = -theta;

                    float3x3 rotationMatrix = upper3x3(::rotation(planes[state.selectedPlane].normal, theta));

                    rotation = rotationMatrix * rotation;

                    // Rotates offset
                    state.selectionOffset = rotationMatrix * state.selectionOffset;
                }

                float4x4 fullRotation;
                fullRotation.rows[0] = float4(rotation.rows[0], 0);
                fullRotation.rows[1] = float4(rotation.rows[1], 0);
                fullRotation.rows[2] = float4(rotation.rows[2], 0);

                transform = translation(origin) * fullRotation * ::scale(scale);
            }

            for (int i = 0; i < 3; i++) {
                if ((1 << i) & state.selectedAxes)
                    axisColors[i] = float3(1, 1, 0);

                if (((1 << ((i + 1) % 3)) & state.selectedAxes) && ((1 << ((i + 2) % 3)) & state.selectedAxes))
                    quadColors[i] = float3(1, 1, 0);
            }
        }
    }
    else {
        int hoveredAxes = 0;
        int hoveredPlane;
        float hoveredDist = INFINITY;

        if (state.mode == TransformModeTranslation || state.mode == TransformModeScale) {
            for (int pickAxis = 0; pickAxis < 3; pickAxis++) {
                // TODO: pick the plane with the greatest precision?
                for (int pickPlane = 0; pickPlane < 3; pickPlane++) {
                    if (pickPlane == pickAxis)
                        continue;

                    float hitDist;
                    if (planes[pickPlane].intersects(ray, hitDist)) {
                        float3 hitPos = ray.at(hitDist) - origin;
                        float onAxisAmt = dot(axes[pickAxis], hitPos);
                        float3 onAxis = onAxisAmt * axes[pickAxis];
                        float3 offAxis = hitPos - onAxis;
                        float offAxisAmt = length(offAxis); // TODO: probably a simpler expression

                        // TODO: Could take cone radius into account
                        if (onAxisAmt > 0.0f && onAxisAmt < axisLength && offAxisAmt < axisRadius * 4.0f && hitDist < hoveredDist) {
                            hoveredAxes = (1 << pickAxis);
                            hoveredDist = hitDist;
                            hoveredPlane = pickPlane;
                        }
                    }
                }
            }

            for (int pickPlane = 0; pickPlane < 3; pickPlane++) {
                int axis0 = (pickPlane + 1) % 3;
                int axis1 = (pickPlane + 2) % 3;

                float hitDist;
                if (planes[pickPlane].intersects(ray, hitDist)) {
                    float3 hitPos = ray.at(hitDist) - origin;

                    float onAxis0 = dot(axes[axis0], hitPos);
                    float onAxis1= dot(axes[axis1], hitPos);

                    // TODO: Selection radius should probably be in screen space

                    if (onAxis0 > axisRadius * 4.0f && onAxis0 < quadLength && onAxis1 > axisRadius * 4.0f && onAxis1 < quadLength && hitDist < hoveredDist) {
                        hoveredAxes = (1 << axis0) | (1 << axis1);
                        hoveredDist = hitDist;
                        hoveredPlane = pickPlane;
                    }
                }
            }
        }
        else if (state.mode == TransformModeRotation) {
            for (int pickPlane = 0; pickPlane < 3; pickPlane++) {
                float hitDist;
                if (planes[pickPlane].intersects(ray, hitDist)) {
                    float3 hitPos = ray.at(hitDist) - origin;

                    float r = length(hitPos);

                    if (r > axisLength * 0.8f && r < axisLength * 1.2f) {
                        hoveredAxes = (1 << ((pickPlane + 1) % 3)) | (1 << ((pickPlane + 2) %3));
                        hoveredDist = hitDist;
                        hoveredPlane = pickPlane;
                    }
                }
            }
        }

        if (hoveredAxes != 0) {
            state.hovered = true;

            if (mouseDown) {
                state.selectedAxes = hoveredAxes;
                state.selectionOffset = ray.at(hoveredDist) - origin;
                state.selectedPlane = hoveredPlane;
                state.active = true;
            }

            for (int i = 0; i < 3; i++) {
                if ((1 << i) & hoveredAxes)
                    axisColors[i] = float3(1, 1, 0);

                if (((1 << ((i + 1) % 3)) & hoveredAxes) && ((1 << ((i + 2) % 3)) & hoveredAxes))
                    quadColors[i] = float3(1, 1, 0);
            }
        }
        else
            state.hovered = false;
    }

    float4x4 gizmoTransform = translation(origin);

    if (state.local || state.mode == TransformModeScale) {
        float4x4 fullRotation;
        fullRotation.rows[0] = float4(rotation.rows[0], 0);
        fullRotation.rows[1] = float4(rotation.rows[1], 0);
        fullRotation.rows[2] = float4(rotation.rows[2], 0);

        gizmoTransform = gizmoTransform * fullRotation;
    }

    if (state.mode == TransformModeTranslation || state.mode == TransformModeScale) {
        drawAxes(gizmoTransform, 8, axisRadius, 0.66f * axisLength, 0.11f * axisLength, 0.33f * axisLength, float4(axisColors[0], 1), float4(axisColors[1], 1), float4(axisColors[2], 1), vertices, commands);

        // TODO: Sort for blending
        float4x4 quadTransform;
        drawFilledQuad(gizmoTransform * quadTransform, quadLength, quadLength, float4(quadColors[2], 0.5f), vertices, commands);
        quadTransform = rotationX((float)M_PI / 2.0f);
        drawFilledQuad(gizmoTransform * quadTransform, quadLength, quadLength, float4(quadColors[1], 0.5f), vertices, commands);
        quadTransform = rotationY(-(float)M_PI / 2.0f);
        drawFilledQuad(gizmoTransform * quadTransform, quadLength, quadLength, float4(quadColors[0], 0.5f), vertices, commands);
    }
    else if (state.mode == TransformModeRotation) {
        float4x4 circleTransform;
        drawWireCircle(gizmoTransform * circleTransform, 32, axisLength, float4(quadColors[2], 1.0f), vertices, commands);
        circleTransform = rotationX((float)M_PI / 2.0f);
        drawWireCircle(gizmoTransform * circleTransform, 32, axisLength, float4(quadColors[1], 1.0f), vertices, commands);
        circleTransform = rotationY(-(float)M_PI / 2.0f);
        drawWireCircle(gizmoTransform * circleTransform, 32, axisLength, float4(quadColors[0], 1.0f), vertices, commands);

        for (int i = 0; i < 3; i++) {
            SolidDrawCmd & cmd = commands[commands.size() - 1 - i];

            cmd.clip = true;
            //cmd.clipPlane = float4()
        }
    }
}