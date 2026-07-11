#pragma once

/**
 * Fan.h
 *
 * Assignment 2 -- "3D Fan"
 *
 * Renders a hierarchical 3D table fan built from a single shared color cube.
 * Parts are positioned using a matrix stack and tinted via fragment uniforms.
 *
 * Part hierarchy:
 *              [B3]
 *               |
 *      [B2]--(hub)--[B0]     Blades rotate around the hub's Z axis.
 *               |            The hub, pole, and base remain still.
 *              [B1]
 *               |
 *              pole
 *               |
 *          ___base___
 *
 * Gestures & Stretch Goals:
 *   - TAP: Toggles the fan ON/OFF.
 *   - SWIPE/DRAG: Temporarily boosts spin speed based on swipe velocity.
 *   - FLING DECAY: Upon release, the drag speed boost gradually winds down.
 *   - LONG PRESS: Holding a tap cycles the blade count (3, 4, or 5).
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
