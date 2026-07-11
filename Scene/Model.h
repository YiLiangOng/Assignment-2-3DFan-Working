#pragma once

/**
 * Model.h
 *
 * Abstract base class for all renderable shapes.
 *
 * Any subclass that fails to implement both InitModel() / Render() will not compile.
 * Resize() and the three touch hooks are optional and provide empty default
 * implementations (Chapter 2 tutorial 4 pattern) -- a model only overrides
 * the ones it actually needs. Renderer fans touch events out to every
 * registered model; Fan is the only model in this project that uses them.
 */

class Model {
public:
    Model() {}
    virtual ~Model() {}

    virtual void InitModel() = 0;          // called before the first frame
    virtual void Render()    = 0;          // called once per frame
    virtual void Resize(int w, int h) { (void)w; (void)h; }   // called whenever the surface/window resizes

    // Touch / mouse event interface (screen pixels, origin top-left).
    virtual void TouchEventDown(float x, float y)    { (void)x; (void)y; }
    virtual void TouchEventMove(float x, float y)    { (void)x; (void)y; }
    virtual void TouchEventRelease(float x, float y) { (void)x; (void)y; }
};
