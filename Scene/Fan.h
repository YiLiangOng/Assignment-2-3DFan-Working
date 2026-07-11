#pragma once

/**
 * Fan.h
 *
 * Assignment 2 -- "3D Fan"
 *
 * Renders a hierarchical 3D table fan (base, pole, hub, four blades) built
 * entirely from ONE shared 8-vertex / 36-index color cube (Chapter 2 tutorial
 * 3 style indexed geometry, uploaded once and captured into a single VAO --
 * Chapter 3 tutorials 1 and 4). Every part is positioned with its own model
 * matrix composed on the provided Transform push/pop matrix stack
 * (Scene/Transform.h) and tinted through the PARTCOLOR fragment uniform --
 * there is no per-part vertex data and no manual matrix math.
 *
 * Part hierarchy (see Fan::Render() in Fan.cpp for the exact Transform
 * calls -- this is the classic "windmill" hierarchical-transform exercise):
 *
 *              [B3]
 *               |
 *      [B2]--(hub)--[B0]     blades rotate around the hub's Z axis,
 *               |             90 degrees apart, driven by spinAngle
 *              [B1]
 *               |
 *              pole           pole connects hub to base
 *               |
 *          ___base___         base sits on the "table"
 *
 * Gesture behaviour (Model touch hooks, fed by Renderer <- platform layer):
 *   - TAP   (touch down + release with < kTapThreshold px of total movement)
 *           toggles fanOn and logs "Fan ON" / "Fan OFF".
 *   - SWIPE / DRAG accumulates a velocity-based dragBoost (measured in
 *           pixels/millisecond via std::chrono, scaled to degrees/frame and
 *           clamped to kMaxBoost) that is added on top of kBaseSpeed while
 *           the finger/mouse is held down. Releasing resets dragBoost to 0,
 *           returning the fan to its base spin speed.
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
    // Uploads the single shared cube (positions + per-corner shade, plus the
    // index buffer) and records the attribute layout + IBO into one VAO.
    void createCube();

    // Uploads this part's MVP + PARTCOLOR uniforms and issues the one
    // glDrawElements call shared by every part.
    void drawPart(const glm::vec3& color);

#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    // --- GPU objects: ONE cube, shared by all seven parts -------------------
    GLuint program = 0;
    GLuint vao     = 0;
    GLuint vbo     = 0;
    GLuint ibo     = 0;

    GLint uMVP       = -1;
    GLint uPartColor = -1;

    // --- Matrix stack + projection ------------------------------------------
    Transform transform;
    float aspectRatio = 1.0f;

    // --- Core animation / interaction state (per assignment spec) ----------
    float spinAngle = 0.0f;   // current blade angle (degrees)
    bool  fanOn     = true;   // toggled by a tap
    float dragBoost = 0.0f;   // extra speed while dragging, 0 otherwise

    static constexpr float kBaseSpeed    = 1.5f;   // deg/frame when ON
    static constexpr float kMaxBoost     = 20.0f;  // clamp for the drag boost
    static constexpr float kBoostScale   = 8.0f;   // px/ms -> deg/frame
    static constexpr float kTapThreshold = 12.0f;  // px: below this it's a tap

    float lastX = 0.0f, lastY = 0.0f;   // previous touch position
    float movedDistance = 0.0f;         // total movement since touch-down
    std::chrono::steady_clock::time_point lastMoveTime;
};
