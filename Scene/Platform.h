#pragma once

/**
 * Platform.h
 *
 * Platform detection and OpenGL header includes for WebGL, Windows, Android, and iOS.
 */

#ifdef __EMSCRIPTEN__
#ifndef PLATFORM_EMSCRIPTEN
        #define PLATFORM_EMSCRIPTEN
    #endif
#elif defined(_WIN32)
#ifndef PLATFORM_WINDOWS
        #define PLATFORM_WINDOWS
    #endif
#elif defined(__ANDROID__)
#ifndef PLATFORM_ANDROID
        #define PLATFORM_ANDROID
    #endif
#endif

#ifdef PLATFORM_EMSCRIPTEN
    #ifdef USE_GLFW
        // GLFW path – Emscripten provides GLFW when built with -s USE_GLFW=3.
        // GLFW_INCLUDE_ES3 tells glfw3.h to pull in <GLES3/gl3.h> automatically.
        #define GLFW_INCLUDE_ES3
        #include <GLFW/glfw3.h>
    #else
        // SDL2 path – built with -s USE_SDL=2
        #include <GLES3/gl3.h>
        #include <SDL2/SDL.h>
    #endif
    #include <emscripten.h>
    #include <cstdio>
    #include <cstring>

    #define LOGI(...) do { printf("[INFO] " __VA_ARGS__); printf("\n"); } while(0)
    #define LOGE(...) do { printf("[ERROR] " __VA_ARGS__); printf("\n"); } while(0)
    #define LOGD(...) do { printf("[DEBUG] " __VA_ARGS__); printf("\n"); } while(0)

#elif defined(PLATFORM_WINDOWS)
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
    #include <cstdio>
    #include <cstring>

    #define LOGI(...) do { printf("[INFO] " __VA_ARGS__); printf("\n"); } while(0)
    #define LOGE(...) do { printf("[ERROR] " __VA_ARGS__); printf("\n"); } while(0)
    #define LOGD(...) do { printf("[DEBUG] " __VA_ARGS__); printf("\n"); } while(0)

#elif defined(PLATFORM_ANDROID)
    #include <GLES3/gl3.h>
    #include <android/log.h>
    #include <cstring>

    #ifndef LOG_TAG
    #define LOG_TAG "GLPIFrameworkIntro"
    #endif

    #define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
    #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
    #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#endif
