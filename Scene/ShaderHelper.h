#pragma once

/**
 * ShaderHelper.h
 *
 * Utility class for compiling and linking shader programs.
 *
 * Supports two loading methods:
 * 1. String-based: Compiles directly from inline C-string source.
 * 2. File-based: Loads .glsl files from disk (Desktop/Web) or APK assets (Android).
 *
 *   all methods are static.
 */

#include "Platform.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class ShaderHelper {
public:
    // --- Core compile/link, shared by both loading methods below ---

    static GLuint compileShader(GLenum type, const char* src)
    {
        GLuint shader = glCreateShader(type);
        if (!shader) {
            LOGE("ShaderHelper: glCreateShader failed (type=0x%x)", type);
            return 0;
        }
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint ok = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLint len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            std::string log(static_cast<size_t>(len), '\0');
            glGetShaderInfoLog(shader, len, nullptr, &log[0]);
            LOGE("ShaderHelper: shader compile error:\n%s", log.c_str());
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    static GLuint linkProgram(GLuint vert, GLuint frag)
    {
        if (!vert || !frag) return 0;

        GLuint prog = glCreateProgram();
        glAttachShader(prog, vert);
        glAttachShader(prog, frag);
        glLinkProgram(prog);

        GLint ok = GL_FALSE;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok) {
            GLint len = 0;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
            std::string log(static_cast<size_t>(len), '\0');
            glGetProgramInfoLog(prog, len, nullptr, &log[0]);
            LOGE("ShaderHelper: program link error:\n%s", log.c_str());
            glDeleteProgram(prog);
            prog = 0;
        }

        glDetachShader(prog, vert);
        glDetachShader(prog, frag);
        return prog;
    }

    // --- Method 1: "stringify" — build directly from inline source strings ---
    // This is what Triangle.cpp already does today; keep it available here
    // for any future Model that prefers to embed its shader source in code.
    static GLuint buildProgram(const char* vertSrc, const char* fragSrc)
    {
        GLuint vs = compileShader(GL_VERTEX_SHADER, vertSrc);
        if (!vs) return 0;
        GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragSrc);
        if (!fs) { glDeleteShader(vs); return 0; }

        GLuint prog = linkProgram(vs, fs);

        // linkProgram() only detaches
        glDeleteShader(vs);
        glDeleteShader(fs);
        return prog;
    }

    // --- Method 2: file-based — load shader source from disk / APK assets ---
#ifdef PLATFORM_ANDROID
    static std::string loadAsset(AAssetManager* mgr, const char* path)
    {
        if (!mgr) {
            LOGE("ShaderHelper::loadAsset: null AAssetManager (path=%s)", path);
            return std::string();
        }

        AAsset* asset = AAssetManager_open(mgr, path, AASSET_MODE_BUFFER);
        if (!asset) {
            LOGE("ShaderHelper::loadAsset: could not open asset '%s'", path);
            return std::string();
        }

        off_t length = AAsset_getLength(asset);
        std::string contents(static_cast<size_t>(length), '\0');
        int readBytes = AAsset_read(asset, &contents[0], static_cast<size_t>(length));
        AAsset_close(asset);

        if (readBytes != length) {
            LOGE("ShaderHelper::loadAsset: short read on '%s' (%d of %ld bytes)",
                 path, readBytes, static_cast<long>(length));
            return std::string();
        }

        return contents;
    }

    static GLuint buildProgramFromAssets(AAssetManager* mgr, const char* vertPath, const char* fragPath)
    {
        std::string vertSrc = loadAsset(mgr, vertPath);
        std::string fragSrc = loadAsset(mgr, fragPath);
        if (vertSrc.empty() || fragSrc.empty()) {
            LOGE("ShaderHelper::buildProgramFromAssets: missing shader source (%s / %s)",
                 vertPath, fragPath);
            return 0;
        }
        return buildProgram(vertSrc.c_str(), fragSrc.c_str());
    }
#else
    static std::string loadFile(const char* filename)
    {
        // Tries candidate paths in priority order
        std::vector<std::string> candidates = {
            std::string("assets/shader/") + filename,
            std::string(filename),
            std::string("shader/") + filename,
            std::string("assets/") + filename,
        };

        for (const auto& candidate : candidates) {
            std::ifstream file(candidate, std::ios::in | std::ios::binary);
            if (file.is_open()) {
                std::ostringstream contents;
                contents << file.rdbuf();
                return contents.str();
            }
        }

        LOGE("ShaderHelper::loadFile: could not open '%s' via any candidate path", filename);
        return std::string();
    }

    static GLuint buildProgramFromFile(const char* vertFile, const char* fragFile)
    {
        std::string vertSrc = loadFile(vertFile);
        std::string fragSrc = loadFile(fragFile);
        if (vertSrc.empty() || fragSrc.empty()) {
            LOGE("ShaderHelper::buildProgramFromFile: missing shader source (%s / %s)",
                 vertFile, fragFile);
            return 0;
        }
        return buildProgram(vertSrc.c_str(), fragSrc.c_str());
    }
#endif
};
