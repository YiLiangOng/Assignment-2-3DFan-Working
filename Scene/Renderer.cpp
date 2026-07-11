/**
 * Renderer.cpp
 *
 * Implementation of the Renderer singleton. This is the only .cpp file in
 * the project permitted to #include concrete shape headers -- Renderer.h
 * itself only ever speaks Model*.
 *
 * Assignment 2: the scene contains exactly one model, the Fan.
 */

#include "Renderer.h"
#include "Fan.h"

Renderer& Renderer::Instance()
{
    static Renderer instance;
    return instance;
}

Renderer::~Renderer()
{
    clearModels();
}

#ifdef PLATFORM_ANDROID
void Renderer::setAssetManager(AAssetManager* mgr)
{
    assetMgr = mgr;
}
#endif

void Renderer::createModels()
{
    clearModels();

#ifdef PLATFORM_ANDROID
    models.push_back(new Fan(assetMgr));
#else
    models.push_back(new Fan());
#endif
}

void Renderer::initializeModels()
{
    for (Model* model : models) {
        model->InitModel();
    }
}

void Renderer::clearModels()
{
    for (Model* model : models) {
        delete model;
    }
    models.clear();
}

bool Renderer::initializeRenderer()
{
    createModels();
    initializeModels();
    return true;
}

void Renderer::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    for (Model* model : models) {
        model->Resize(w, h);
    }
}

void Renderer::render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (Model* model : models) {
        model->Render();
    }
}

void Renderer::TouchEventDown(float x, float y)
{
    for (Model* model : models) model->TouchEventDown(x, y);
}

void Renderer::TouchEventMove(float x, float y)
{
    for (Model* model : models) model->TouchEventMove(x, y);
}

void Renderer::TouchEventRelease(float x, float y)
{
    for (Model* model : models) model->TouchEventRelease(x, y);
}
