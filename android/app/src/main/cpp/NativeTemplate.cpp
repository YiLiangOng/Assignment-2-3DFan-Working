/**
 * NativeTemplate.cpp
 *
 * JNI bridge between the Java/Android layer and the shared C++ rendering
 * engine. The rendering code lives in the root Scene/ directory and is
 * shared with the Desktop (GLFW) and WebGL (Emscripten) builds.
 *
 * Native method signatures must match those declared in MainActivity.java:
 *   package com.example.fan3d
 *   class   MainActivity
 *
 * Symbol-name rule: Java_ + package (dots -> underscores) + class + method.
 */

#define LOG_TAG "Fan3D"
// PLATFORM_ANDROID must be defined before including any Scene headers
#ifndef PLATFORM_ANDROID
#define PLATFORM_ANDROID
#endif

#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

// Shared Scene header (include path set by CMakeLists.txt)
#include "Renderer.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// ---------------------------------------------------------------------------
// JNI entry points - must match the package/class in MainActivity.java
// Package: com.example.fan3d  Class: MainActivity
// ---------------------------------------------------------------------------

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_fan3d_MainActivity_nativeInit(
        JNIEnv* env, jobject /* thiz */, jobject assetManager)
{
    LOGI("nativeInit called");

    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    if (!mgr) {
        LOGE("Failed to obtain AAssetManager");
        return JNI_FALSE;
    }

    Renderer::Instance().setAssetManager(mgr);
    bool ok = Renderer::Instance().initializeRenderer();
    return ok ? JNI_TRUE : JNI_FALSE;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_fan3d_MainActivity_nativeResize(
        JNIEnv* /* env */, jobject /* thiz */, jint width, jint height)
{
    LOGI("nativeResize: %d x %d", width, height);
    Renderer::Instance().resize(width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_fan3d_MainActivity_nativeRender(
        JNIEnv* /* env */, jobject /* thiz */)
{
    Renderer::Instance().render();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_fan3d_MainActivity_nativeTouchDown(
        JNIEnv* /* env */, jobject /* thiz */, jfloat x, jfloat y)
{
    Renderer::Instance().TouchEventDown(x, y);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_fan3d_MainActivity_nativeTouchMove(
        JNIEnv* /* env */, jobject /* thiz */, jfloat x, jfloat y)
{
    Renderer::Instance().TouchEventMove(x, y);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_fan3d_MainActivity_nativeTouchRelease(
        JNIEnv* /* env */, jobject /* thiz */, jfloat x, jfloat y)
{
    Renderer::Instance().TouchEventRelease(x, y);
}
