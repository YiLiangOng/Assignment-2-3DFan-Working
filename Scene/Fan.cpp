/**
 * Fan.cpp
 *
 * See Fan.h for the part hierarchy diagram and gesture summary.
 *
 * PART 1 -- geometry & static scene graph
 * ----------------------------------------------------------------------
 * The cube is 8 unique corners / 36 indices (Chapter 2 tutorial 3 style).
 * InitModel() uploads it ONCE, following the Chapter 3 tutorial 1
 * sub-region VBO pattern -- a single glBufferData(nullptr) allocation
 * followed by two glBufferSubData calls (24 position floats, then 8
 * per-corner grayscale "shade" floats) -- and records the attribute
 * layout plus the index buffer into one VAO (Chapter 3 tutorial 4), so
 * Render() only needs a single glBindVertexArray() per part.
 *
 * Render() draws that one cube seven times -- base, pole, hub, and four
 * blades -- each with its own model matrix built on the provided
 * Transform push/pop stack (parent-first, post-multiplying, exactly like
 * legacy glTranslatef/glRotatef) and its own PARTCOLOR uniform. No
 * per-part vertex data and no manual matrix math exist anywhere here.
 *
 * PART 2 -- animation
 * ----------------------------------------------------------------------
 * spinAngle advances once per frame at the top of Render() and feeds the
 * TransformRotate() call inside the blade push/pop block only, so the
 * base/pole/hub stay motionless while the four blades spin together.
 *
 * PARTS 3 & 4 -- gestures
 * ----------------------------------------------------------------------
 * TouchEventDown/Move/Release implement tap-vs-swipe detection and the
 * velocity-based speed boost described in Fan.h.
 */

#include "Fan.h"
#include "ShaderHelper.h"
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <algorithm>

namespace {

// 8 unique cube corners: V0..V3 front face, V4..V7 back face -- the same
// indexed color-cube convention used throughout Chapters 2 and 3.
const GLfloat kPositions[8][3] = {
    {-1,-1, 1}, {-1, 1, 1}, { 1, 1, 1}, { 1,-1, 1},  // front
    {-1,-1,-1}, {-1, 1,-1}, { 1, 1,-1}, { 1,-1,-1},  // back
};

// Per-corner grayscale "shade" factor -- front corners brighter than back
// (~1.0 vs ~0.6), giving every flat-tinted part a visible sense of depth.
// The part's actual color comes from the PARTCOLOR uniform, not this VBO.
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

// ---------------------------------------------------------------------------
// Part 1 (geometry): one VBO (positions + shade as two sub-regions), one
// IBO, captured into one VAO. Mirrors Chapter 3 tutorials 1 and 4.
// ---------------------------------------------------------------------------
void Fan::createCube()
{
    const GLsizeiptr posSize   = sizeof(kPositions);  // 8*3 floats = 96 bytes
    const GLsizeiptr shadeSize = sizeof(kShades);      // 8 floats   = 32 bytes

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, posSize + shadeSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,       posSize,   kPositions);  // region 0
    glBufferSubData(GL_ARRAY_BUFFER, posSize, shadeSize, kShades);     // region 1

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);                                   // start recording

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(ATTRIB_SHADE);
    glVertexAttribPointer(ATTRIB_SHADE, 1, GL_FLOAT, GL_FALSE, 0, (void*)posSize);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);   // IBO binding is VAO state

    glBindVertexArray(0);                                     // seal the VAO
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

    // View stays identity for this assignment; only model + projection move.
    transform.TransformSetMatrixMode(VIEW_MATRIX);
    transform.TransformLoadIdentity();

    lastMoveTime = std::chrono::steady_clock::now();
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

// ---------------------------------------------------------------------------
// Part 2 (animation) lives in the one line below: spinAngle only advances
// when fanOn is true, and only the blade push/pop block reads it.
// ---------------------------------------------------------------------------
void Fan::Render()
{
    if (!program || !vao) return;

    if (fanOn) spinAngle += kBaseSpeed + dragBoost;
    if (spinAngle >= 360.0f) spinAngle -= 360.0f;

    glEnable(GL_DEPTH_TEST);
    glUseProgram(program);
    glBindVertexArray(vao);

    static const glm::vec3 baseColor(0.45f, 0.28f, 0.12f);
    static const glm::vec3 poleColor(0.55f, 0.55f, 0.58f);
    static const glm::vec3 hubColor (0.20f, 0.20f, 0.22f);
    static const glm::vec3 bladeColor[4] = {
        glm::vec3(0.85f, 0.10f, 0.10f),  // 0: red
        glm::vec3(0.15f, 0.35f, 0.85f),  // 1: blue
        glm::vec3(0.90f, 0.50f, 0.05f),  // 2: orange
        glm::vec3(0.15f, 0.70f, 0.20f),  // 3: green
    };

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // Shared world placement: pull the fan back into the scene and tilt it
    // for a 3D look. Every part below starts from this same matrix.
    transform.TransformTranslate(0.0f, 0.8f, -8.0f);
    transform.TransformRotate(glm::radians(20.0f), 0.0f, 1.0f, 0.0f);

    // --- Base ----------------------------------------------------------
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, -2.6f, 0.0f);
        transform.TransformScale(1.6f, 0.25f, 0.8f);
        drawPart(baseColor);
    transform.TransformPopMatrix();

    // --- Pole ------------------------------------------------------------
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, -1.21f, 0.0f);
        transform.TransformScale(0.15f, 2.53f, 0.15f);
        drawPart(poleColor);
    transform.TransformPopMatrix();

    // --- Hub -------------------------------------------------------------
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, 0.2f, 0.15f);
        transform.TransformRotate(glm::radians(spinAngle), 0.0f, 0.0f, 1.0f);
        transform.TransformScale(0.22f, 0.22f, 0.08f);
        drawPart(hubColor);
    transform.TransformPopMatrix();

    // --- Four blades, 90 degrees apart around the hub's Z axis ------------
    for (int i = 0; i < 4; ++i) {
        transform.TransformPushMatrix();
            transform.TransformTranslate(0.0f, 0.2f, 0.15f);
            transform.TransformRotate(glm::radians(spinAngle + i * 90.0f), 0.0f, 0.0f, 1.0f);
            transform.TransformTranslate(0.0f, 0.55f, 0.0f);
            transform.TransformScale(0.22f, 0.8f, 0.05f);
            drawPart(bladeColor[i]);
        transform.TransformPopMatrix();
    }

    glBindVertexArray(0);
}

// ---------------------------------------------------------------------------
// Parts 3 & 4 (gestures): a tap toggles the fan; a drag's measured velocity
// drives a temporary speed boost on top of the base rotation speed.
// ---------------------------------------------------------------------------

void Fan::TouchEventDown(float x, float y)
{
    lastX = x;
    lastY = y;
    movedDistance = 0.0f;
    lastMoveTime = std::chrono::steady_clock::now();
}

void Fan::TouchEventMove(float x, float y)
{
    const float dx   = x - lastX;
    const float dy   = y - lastY;
    const float dist = std::sqrt(dx * dx + dy * dy);   // pixels this event
    movedDistance += dist;

    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    float dtMs = std::chrono::duration<float, std::milli>(now - lastMoveTime).count();
    if (dtMs < 1.0f) dtMs = 1.0f;   // guard divide-by-~0 on back-to-back callbacks

    const float velocity = dist / dtMs;   // pixels per millisecond
    dragBoost = std::min(velocity * kBoostScale, kMaxBoost);

    lastX = x;
    lastY = y;
    lastMoveTime = now;
}

void Fan::TouchEventRelease(float /*x*/, float /*y*/)
{
    if (movedDistance < kTapThreshold) {
        fanOn = !fanOn;
        LOGI("Fan %s", fanOn ? "ON" : "OFF");
    }
    dragBoost = 0.0f;   // boost applies only while dragging
}
