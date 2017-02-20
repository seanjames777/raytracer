#ifndef __TRANSFORMGIZMO_H
#define __TRANSFORMGIZMO_H

#include <preview/solidgeom.h>
#include <math/vector.h>
#include <math/matrix.h>

// TODO: Make sure transform mode and local do not change while dragging the tool
// TODO: Camera relative transform mode?
// TODO: Snap to grid, discrete transform
// TODO: Make pretty and consistent
// TODO: Uniform scale

enum TransformMode {
    TransformModeTranslation,
    TransformModeRotation,
    TransformModeScale
};

struct TransformGizmoState {
    // Public read/write state
    float pixelSize;
    bool local;
    TransformMode mode;

    // Public read only state
    bool hovered;
    bool active;

    // Private state
    int selectedPlane;
    float3 selectionOffset;
    int selectedAxes;

    TransformGizmoState()
        : pixelSize(100.0f),
          local(false),
          mode(TransformModeTranslation),
          hovered(false),
          active(false)
    {
    }
};

void transformGizmo(
    float4x4 & transform,
    const float4x4 & viewProjection,
    bool mouseDown,
    const float2 & mousePos, // mouseX, mouseY
    const float2 & viewPos,
    const float2 & viewSize,
    TransformGizmoState & state,
    std::vector<SolidVertex> & vertices,
    std::vector<SolidDrawCmd> & commands);

#endif
