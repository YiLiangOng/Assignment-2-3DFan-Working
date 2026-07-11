#pragma once

/**
 * Renderer.h
 *
 * Singleton controller for the rendering pipeline.
 *
 * Manages a collection of Model pointers. Initializes, resizes, and
 * renders all shapes in the scene, and fans touch/mouse events out to
 * every registered model (Chapter 2 tutorial 4 pattern).
 *
 * On Android, holds the AAssetManager* received from the Java layer
 * and passes it to models during construction for file loading.
 */

#include "Model.h"
#include "Platform.h"
#include <vector>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Renderer {
public:
    static Renderer& Instance();

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

#ifdef PLATFORM_ANDROID
    void setAssetManager(AAssetManager* mgr);
#endif

    bool initializeRenderer();
    void resize(int w, int h);
    void render();

    // Touch / mouse delegation -- forwarded to every registered model.
    void TouchEventDown(float x, float y);
    void TouchEventMove(float x, float y);
    void TouchEventRelease(float x, float y);

private:
    Renderer()  = default;
    ~Renderer();

    void createModels();
    void initializeModels();
    void clearModels();

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    std::vector<Model*> models;
};
