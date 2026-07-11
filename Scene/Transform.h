// Transform.h -- Self-contained matrix stack for OpenGL / OpenGL ES 3.0
//
// Original source: OpenGL ES 3.0 Cookbook (GLPIFramework)
// Modified by   : Parminder Singh
// Modifications : Removed glutils.h and constant.h dependencies.
//                 All required constants are inlined below so this header
//                 compiles standalone with GLM on every Assignment 2 target.
//                 GL headers and the LOGI macro now come from Platform.h so
//                 the same file builds on Android / Windows / Emscripten.
//
// MIT License
//
// Copyright (c) 2026 Parminder Singh
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef TRANSFORM_H
#define TRANSFORM_H

#pragma once

// ---- OpenGL + logging via the project platform layer -----------------------
// Platform.h selects <GLES3/gl3.h> (Android / Emscripten) or GLEW (Windows)
// and defines LOGI / LOGE / LOGD for each platform.
#include "Platform.h"

// ---- GLM ------------------------------------------------------------------
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ---- Standard library (used in Transform.cpp) -----------------------------
#include <cstring>   // memset, memcpy
#include <cstdio>    // printf

// ---- Constants (inlined from constant.h) ----------------------------------
//! Maximum character buffer size used in error reporting.
#define MAX_CHAR               64

//! Matrix stack depths.
#define MAX_MODEL_MATRIX        8
#define MAX_VIEW_MATRIX         8
#define MAX_PROJECTION_MATRIX   2
#define MAX_TEXTURE_MATRIX      2

//! Matrix mode identifiers -- passed to TransformSetMatrixMode().
enum
{
    MODEL_MATRIX      = 0,   //!< Operates on the model matrix stack.
    VIEW_MATRIX       = 1,   //!< Operates on the view matrix stack.
    PROJECTION_MATRIX = 2,   //!< Operates on the projection matrix stack.
    TEXTURE_MATRIX    = 3    //!< Operates on the texture matrix stack.
};

// ---------------------------------------------------------------------------
//! TransformData -- internal storage for all four matrix stacks and derived
//! matrices (modelview, modelview-projection, normal matrix).
// ---------------------------------------------------------------------------
typedef struct
{
    //! Active matrix mode (MODEL_MATRIX, VIEW_MATRIX, PROJECTION_MATRIX, TEXTURE_MATRIX).
    unsigned char  matrix_mode;

    //! Stack index for each matrix type (0 = bottom of stack).
    unsigned char  modelMatrixIndex;
    unsigned char  viewMatrixIndex;
    unsigned char  projectionMatrixIndex;
    unsigned char  textureMatrixIndex;

    //! The four matrix stacks.
    glm::mat4  model_matrix      [ MAX_MODEL_MATRIX      ];
    glm::mat4  view_matrix       [ MAX_VIEW_MATRIX       ];
    glm::mat4  projection_matrix [ MAX_PROJECTION_MATRIX ];
    glm::mat4  texture_matrix    [ MAX_TEXTURE_MATRIX    ];

    //! Cached P * V * M result (computed by TransformGetModelViewProjectionMatrix).
    glm::mat4  modelview_projection_matrix;

    //! Cached V * M result (computed by TransformGetModelViewMatrix).
    glm::mat4  modelview_matrix;

    //! Cached inverse-transpose of the upper 3x3 of V*M (for lighting normals).
    glm::mat3  normal_matrix;

} TransformData;

// ---------------------------------------------------------------------------
//! Transform -- OpenGL-style matrix stack manager.
//
//  Usage pattern (2D windmill example, MODEL_MATRIX only):
//
//    transform.TransformSetMatrixMode(MODEL_MATRIX);
//    transform.TransformLoadIdentity();
//
//    // Static part (pole):
//    transform.TransformPushMatrix();
//      transform.TransformTranslate(0.0f, -0.565f, 0.0f);
//      transform.TransformScale(0.044f, 0.67f, 1.0f);
//      quad.render(*transform.TransformGetModelMatrix());
//    transform.TransformPopMatrix();
//
//    // Animated child (blade i), pivot = hub centre:
//    transform.TransformPushMatrix();
//      transform.TransformTranslate(0.0f, -0.22f, 0.0f);  // move to hub
//      transform.TransformRotate(spinAngle + i*PI/2, 0,0,1);
//      transform.TransformTranslate(0.0f,  0.175f, 0.0f); // offset from hub
//      transform.TransformScale(0.065f, 0.24f, 1.0f);
//      quad.render(*transform.TransformGetModelMatrix());
//    transform.TransformPopMatrix();
// ---------------------------------------------------------------------------
class Transform
{
public:
     Transform(void);
    ~Transform(void);

    //! Initialise all stacks to identity and set default GL state.
    void TransformInit( void );

    //! Print any pending GL errors to the console.
    void TransformError( void );

    //! Select which stack subsequent operations target.
    void TransformSetMatrixMode( unsigned int mode );

    //! Replace the top of the active stack with the identity matrix.
    void TransformLoadIdentity( void );

    //! Duplicate the top of the active stack (push).
    void TransformPushMatrix( void );

    //! Discard the top of the active stack (pop).
    void TransformPopMatrix( void );

    //! Replace the top of the active stack with *m.
    void TransformLoadMatrix( glm::mat4 *m );

    //! Post-multiply the top of the active stack by *m.
    void TransformMultiplyMatrix( glm::mat4 *m );

    //! Post-multiply the top of the active stack by a translation matrix.
    void TransformTranslate( float x, float y, float z );

    //! Post-multiply the top of the active stack by a rotation matrix.
    //  angle is in RADIANS.  axis is (x, y, z).
    void TransformRotate( float angle, float x, float y, float z );

    //! Post-multiply the top of the active stack by a scale matrix.
    void TransformScale( float x, float y, float z );

    // ---- Matrix accessors (return pointer to top of the named stack) ------
    glm::mat4 *TransformGetModelMatrix( void );
    glm::mat4 *TransformGetViewMatrix( void );
    glm::mat4 *TransformGetProjectionMatrix( void );
    glm::mat4 *TransformGetTextureMatrix( void );

    //! Returns P * V * M (computes and caches the result).
    glm::mat4 *TransformGetModelViewProjectionMatrix( void );

    //! Returns V * M (computes and caches the result).
    glm::mat4 *TransformGetModelViewMatrix( void );

    //! Fills *mat3Obj with the inverse-transpose of the upper 3x3 of V*M.
    void TransformGetNormalMatrix( glm::mat3 *mat3Obj );

    // ---- Projection helpers -----------------------------------------------
    void TransformOrtho( float left, float right, float bottom, float top,
                         float clip_start, float clip_end );

    void TransformOrthoGrahpic( float screen_ratio, float scale,
                                float aspect_ratio, float clip_start,
                                float clip_end, float screen_orientation );

    void TransformSetPerspective( float fovy, float aspect_ratio,
                                  float clip_start, float clip_end,
                                  float screen_orientation );

    // ---- View helpers -----------------------------------------------------
    void TransformLookAt( glm::vec3 *eye, glm::vec3 *center, glm::vec3 *up );
    void TransformSetView( glm::mat4 mat );

    // ---- Projection / Unprojection ----------------------------------------
    int TransformProject( float objx, float objy, float objz,
                          glm::mat4 *modelview_matrix,
                          glm::mat4 *projection_matrix,
                          int *viewport_matrix,
                          float *winx, float *winy, float *winz );

    int TransformUnproject( float winx, float winy, float winz,
                            glm::mat4 *modelview_matrix,
                            glm::mat4 *projection_matrix,
                            int *viewport_matrix,
                            float *objx, float *objy, float *objz );

    //! Multiply a vec4 by a mat4: dst = v * m.
    void Vec4MultiplyMat4( glm::vec4 *dst, glm::vec4 *v0, glm::mat4 *v1 );

    //! All matrix data lives here (public for legacy compatibility).
    TransformData TransformMemData;
};

#endif // TRANSFORM_H
