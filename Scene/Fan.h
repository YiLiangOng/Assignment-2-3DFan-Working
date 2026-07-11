#pragma once

/**
 * Fan.h
 *
 * Assignment 2 -- "3D Fan"
 *
 * Renders a 3D table fan using a single shared 8-vertex cube.
 * Parts are positioned using a hierarchical matrix stack (Transform.h) 
 * and colored via the PARTCOLOR uniform.
 *
 * Part hierarchy:
 *              [B3] (or 3-5 evenly spaced blades)
 *               |
 *      [B2]--(hub)--[B0]
 *               |
 *              [B1]
 *               |
 *              pole
 *               |
 *          ___base___
 *
 * Gestures:
 *   - TAP: Toggles fan on/off.
 *   - LONG PRESS: Cycles through 3, 4, and 5 blades.
 *   - SWIPE/DRAG: Boosts spin speed based on drag velocity; the added speed
 *     decays smoothly after release for a brief fling effect.
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
    int   bladeCount = 4;

    static constexpr float kBaseSpeed    = 1.5f;   // deg/frame when ON
    static constexpr float kMaxBoost     = 20.0f;  // drag boost clamp
    static constexpr float kBoostScale   = 8.0f;   // px/ms -> deg/frame
    static constexpr float kTapThreshold = 12.0f;  // px threshold for tap vs swipe
    static constexpr float kBoostDecay   = 0.98f;  // per-frame fling decay
    static constexpr int   kMinBladeCount = 3;
    static constexpr int   kMaxBladeCount = 5;
    static constexpr long long kBladeCyclePressMs = 550;

    float lastX = 0.0f, lastY = 0.0f;
    float movedDistance = 0.0f;
    std::chrono::steady_clock::time_point lastMoveTime;
    std::chrono::steady_clock::time_point touchDownTime;
};
