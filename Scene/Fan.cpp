/**
 * Fan.cpp
 * 
 * Implements the 3D Fan model geometry, animation, and touch gestures.
 */

#include "Fan.h"
#include "ShaderHelper.h"
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <algorithm>

namespace {

// 8 unique unit-cube corners (front and back)
const GLfloat kPositions[8][3] = {
    {-0.5f,-0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f},
    { 0.5f, 0.5f, 0.5f}, { 0.5f,-0.5f, 0.5f},  // front
    {-0.5f,-0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f},
    { 0.5f, 0.5f,-0.5f}, { 0.5f,-0.5f,-0.5f},  // back
};

// Per-corner grayscale shade for basic depth
const GLfloat kShades[8] = {
    1.0f, 1.0f, 1.0f, 1.0f,   // front corners
    0.6f, 0.6f, 0.6f, 0.6f,   // back corners
};

const GLushort kIndices[36] = {
    0,3,1,  3,2,1,   // front
    7,4,6,  4,5,6,   // back
    4,0,5,  0,1,5,   // left
    3,7,2,  7,6,2,   // right
    1,2,5,  2,6,5,   // top
    3,0,7,  0,4,7    // bottom
};

constexpr GLuint ATTRIB_POSITION = 0;
constexpr GLuint ATTRIB_SHADE    = 1;

} // namespace

#ifdef PLATFORM_ANDROID
Fan::Fan(AAssetManager* assetMgr) : mgr(assetMgr) {}
#else
Fan::Fan() {}
#endif

Fan::~Fan()
{
    if (vbo)     glDeleteBuffers(1, &vbo);
    if (ibo)     glDeleteBuffers(1, &ibo);
    if (vao)     glDeleteVertexArrays(1, &vao);
    if (program) glDeleteProgram(program);
}

// --- Geometry Setup ----
void Fan::createCube()
{
    const GLsizeiptr posSize   = sizeof(kPositions); 
    const GLsizeiptr shadeSize = sizeof(kShades);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, posSize + shadeSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,       posSize,   kPositions);  // region 0
    glBufferSubData(GL_ARRAY_BUFFER, posSize, shadeSize, kShades);     // region 1

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao); // start recording

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(ATTRIB_SHADE);
    glVertexAttribPointer(ATTRIB_SHADE, 1, GL_FLOAT, GL_FALSE, 0, (void*)posSize);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); // IBO binding is VAO state

    glBindVertexArray(0); // seal VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Fan::InitModel()
{
#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(mgr,
        "shader/FanVertex.glsl", "shader/FanFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile("FanVertex.glsl", "FanFragment.glsl");
#endif
    if (!program) { LOGE("Fan: failed to build shader program"); return; }

    uMVP       = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    uPartColor = glGetUniformLocation(program, "PARTCOLOR");

    createCube();

    transform.TransformSetMatrixMode(VIEW_MATRIX);
    transform.TransformLoadIdentity();

    lastMoveTime = std::chrono::steady_clock::now();
    touchDownTime = lastMoveTime;
}

void Fan::Resize(int w, int h)
{
    if (h <= 0) h = 1;
    aspectRatio = static_cast<float>(w) / static_cast<float>(h);

    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformSetPerspective(glm::radians(60.0f), aspectRatio, 0.01f, 1000.0f, 0.0f);
}

void Fan::drawPart(const glm::vec3& color)
{
    glm::mat4* mvp = transform.TransformGetModelViewProjectionMatrix();
    glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(*mvp));
    glUniform3fv(uPartColor, 1, glm::value_ptr(color));
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
}

// --- Render & Animation ---
void Fan::Render()
{
    if (!program || !vao) return;

    if (fanOn) spinAngle += kBaseSpeed + dragBoost;
    if (spinAngle >= 360.0f) spinAngle -= 360.0f;

    // Bonus: preserve a swipe's momentum briefly after release, then coast
    // back to the base speed instead of dropping instantly to it
    dragBoost *= kBoostDecay;
    if (dragBoost < 0.01f) dragBoost = 0.0f;

    glEnable(GL_DEPTH_TEST);
    glUseProgram(program);
    glBindVertexArray(vao);

    static const glm::vec3 baseColor(0.45f, 0.28f, 0.12f);
    static const glm::vec3 poleColor(0.55f, 0.55f, 0.58f);
    static const glm::vec3 hubColor (0.20f, 0.20f, 0.22f);
    static const glm::vec3 bladeColor[kMaxBladeCount] = {
        glm::vec3(0.85f, 0.10f, 0.10f),  // 0: red
        glm::vec3(0.15f, 0.35f, 0.85f),  // 1: blue
        glm::vec3(0.90f, 0.50f, 0.05f),  // 2: orange
        glm::vec3(0.15f, 0.70f, 0.20f),  // 3: green
        glm::vec3(0.65f, 0.20f, 0.80f),  // 4: purple
    };

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // Pull the fan back into the scene and tilt it
    transform.TransformTranslate(0.0f, 0.8f, -8.0f);
    transform.TransformRotate(glm::radians(20.0f), 0.0f, 1.0f, 0.0f);

    // --- Base ----
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, -2.6f, 0.0f);
        transform.TransformScale(1.6f, 0.25f, 0.8f);
        drawPart(baseColor);
    transform.TransformPopMatrix();

    // --- Pole -------
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, -1.21f, 0.0f);
        transform.TransformScale(0.15f, 2.53f, 0.15f);
        drawPart(poleColor);
    transform.TransformPopMatrix();

    // --- Hub --
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, 0.2f, 0.15f);
        transform.TransformScale(0.22f, 0.22f, 0.08f);
        drawPart(hubColor);
    transform.TransformPopMatrix();

    // --- 3-5 Blades (evenly spaced around the hub) ---
    const float bladeSpacing = 360.0f / static_cast<float>(bladeCount);
    for (int i = 0; i < bladeCount; ++i) {
        transform.TransformPushMatrix();
            transform.TransformTranslate(0.0f, 0.2f, 0.15f);
            transform.TransformRotate(glm::radians(spinAngle + i * bladeSpacing), 0.0f, 0.0f, 1.0f);
            transform.TransformTranslate(0.0f, 0.55f, 0.0f);
            transform.TransformScale(0.22f, 0.8f, 0.05f);
            drawPart(bladeColor[i]);
        transform.TransformPopMatrix();
    }

    glBindVertexArray(0);
}

// --- Gestures -----------
void Fan::TouchEventDown(float x, float y)
{
    lastX = x;
    lastY = y;
    movedDistance = 0.0f;
    touchDownTime = std::chrono::steady_clock::now();
    lastMoveTime = touchDownTime;
}

void Fan::TouchEventMove(float x, float y)
{
    const float dx   = x - lastX;
    const float dy   = y - lastY;
    const float dist = std::sqrt(dx * dx + dy * dy);   // pixels event
    movedDistance += dist;

    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    float dtMs = std::chrono::duration<float, std::milli>(now - lastMoveTime).count();
    if (dtMs < 1.0f) dtMs = 1.0f;

    const float velocity = dist / dtMs;   // pixels per millisecond
    dragBoost = std::min(velocity * kBoostScale, kMaxBoost);

    lastX = x;
    lastY = y;
    lastMoveTime = now;
}

void Fan::TouchEventRelease(float /*x*/, float /*y*/)
{
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    const long long pressMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - touchDownTime).count();

    if (movedDistance < kTapThreshold) {
        if (pressMs >= kBladeCyclePressMs) {
            bladeCount = (bladeCount == kMaxBladeCount)
                ? kMinBladeCount
                : bladeCount + 1;
            LOGI("Blade count: %d", bladeCount);
        } else {
            fanOn = !fanOn;
            LOGI("Fan %s", fanOn ? "ON" : "OFF");
        }
    }
    // Bonus: dragBoost is intentionally not cleared here so that the speed decays slowly instead
}
