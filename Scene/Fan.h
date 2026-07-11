#pragma once

/**
 * Fan.h
 *
 * Assignment 2 -- "3D Fan"
 *
 * Renders a hierarchical 3D table fan (base, pole, hub, blades) built
 * entirely from ONE shared 8-vertex / 36-index color cube (Chapter 2
 * tutorial 3 style indexed geometry, uploaded once and captured into a
 * single VAO -- Chapter 3 tutorials 1 and 4). Every part is positioned
 * with its own model matrix composed on the provided Transform push/pop
 * matrix stack (Scene/Transform.h) and tinted through the PARTCOLOR
 * fragment uniform -- there is no per-part vertex data and no manual
 * matrix math.
 *
 * Part hierarchy (see Fan::Render() in Fan.cpp for the exact Transform
 * calls -- this is the classic "windmill" hierarchical-transform exercise):
 *
 *              [B3]
 *               |
 *      [B2]--(hub)--[B0]     blades rotate around the hub's Z axis,
 *               |             360/bladeCount degrees apart, driven by
 *              [B1]           spinAngle. The hub itself stays still.
 *               |
 *              pole           pole connects hub to base
 *               |
 *          ___base___         base sits on the "table"
 *
 * Core gesture behaviour (Model touch hooks, fed by Renderer <- platform
 * layer):
 *   - TAP   (touch down + release, < kTapThreshold px of movement, held
 *           for less than kLongPressMs) toggles fanOn and logs
 *           "Fan ON" / "Fan OFF".
 *   - SWIPE / DRAG accumulates a velocity-based dragBoost (measured in
 *           pixels/millisecond via std::chrono, scaled to degrees/frame
 *           and clamped to kMaxBoost) that is added on top of kBaseSpeed
 *           while the finger/mouse is held down.
 *
 * Stretch goals implemented on top of the core spec:
 *   - Fling with decay: releasing after a genuine drag no longer snaps
 *           dragBoost back to 0 -- it persists and decays by kDragDecay
 *           (0.98) every frame in Render() until it settles near zero,
 *           like a real fan winding down. (A stationary tap/long-press
 *           release still zeroes it immediately, since there is no fling
 *           to preserve.)
 *   - Blade count toggle: a LONG PRESS (touch down + release with
 *           < kTapThreshold px of movement, held for >= kLongPressMs)
 *           cycles bladeCount through 3 -> 4 -> 5 -> 3 and logs the new
 *           count. The blades re-space themselves evenly at
 *           360/bladeCount degrees apart every frame, so the change is
 *           immediate and needs no extra geometry -- it just changes the
 *           loop bound and rotation spacing in Render().
 */

#include "Model.h"
#include "Platform.h"
#include "Transform.h"
#include <chrono>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Fan : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Fan(AAssetManager* assetMgr);
#else
    Fan();
#endif
    ~Fan() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    void TouchEventDown(float x, float y)    override;
    void TouchEventMove(float x, float y)    override;
    void TouchEventRelease(float x, float y) override;

private:
    // Setup geometry and VAO
    void createCube();

    // Bind uniforms and draw
    void drawPart(const glm::vec3& color);

#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    // --- GPU objects ---
    GLuint program = 0;
    GLuint vao     = 0;
    GLuint vbo     = 0;
    GLuint ibo     = 0;

    GLint uMVP       = -1;
    GLint uPartColor = -1;

    // --- Matrix stack ---
    Transform transform;
    float aspectRatio = 1.0f;

    // --- Animation & Interaction ---
    float spinAngle = 0.0f;
    bool  fanOn     = true;
    float dragBoost = 0.0f;

    static constexpr float kBaseSpeed    = 1.5f;   // deg/frame when ON
    static constexpr float kMaxBoost     = 20.0f;  // drag boost clamp
    static constexpr float kBoostScale   = 8.0f;   // px/ms -> deg/frame
    static constexpr float kTapThreshold = 12.0f;  // px threshold for tap vs swipe

    float lastX = 0.0f, lastY = 0.0f;
    float movedDistance = 0.0f;
    std::chrono::steady_clock::time_point lastMoveTime;

    // --- Stretch goal: fling with decay -------------------------------
    // True from TouchEventDown until TouchEventRelease; Render() only
    // decays dragBoost while this is false, so a live drag is never
    // fought by the decay.
    bool isDragging = false;
    static constexpr float kDragDecay = 0.98f;   // per-frame decay once released

    // --- Stretch goal: blade count toggle (long press) ------------------
    int bladeCount = 4;
    static constexpr int   kMinBladeCount = 3;
    static constexpr int   kMaxBladeCount = 5;
    static constexpr float kLongPressMs   = 500.0f;   // hold time to count as a long press
    std::chrono::steady_clock::time_point touchDownTime;
};
