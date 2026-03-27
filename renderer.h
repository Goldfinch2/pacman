#ifndef RENDERER_H
#define RENDERER_H

#include "SDL.h"
#include "defines.h"

namespace renderer
{
    // Lifecycle
    void init(SDL_Window* window, int w, int h);
    void shutdown();

    // Frame
    void beginFrame();
    void present();

    // 2D projection: maps game coords (0..PACMAN_MAX_X, 0..FIELD_H) with Y=0 at top
    void enable2D(int screenW, int screenH);
    void disable2D();

    // State
    void setColor(float r, float g, float b, float a = 1.0f);
    void setLineWidth(float w);
    void setPointSize(float s);
    void setBlendAdditive(bool additive);
    void enableLineSmooth(bool enable);

    // GL_LINES
    void beginLines();
    void endLines();
    void lineVertex(float x, float y, float r, float g, float b, float a = 1.0f);

    // Convenience: draw a single line segment
    void drawLine(float x1, float y1, float x2, float y2,
                  float r, float g, float b, float a = 1.0f);

    // GL_POINTS
    void beginPoints();
    void endPoints();
    void pointVertex(float x, float y, float r, float g, float b, float a = 1.0f);

    // GL_LINE_LOOP
    void beginLineLoop();
    void endLineLoop();
    void loopVertex(float x, float y, float r, float g, float b, float a = 1.0f);

    // GL_LINE_STRIP
    void beginLineStrip();
    void endLineStrip();
    void stripVertex(float x, float y, float r, float g, float b, float a = 1.0f);
}

#endif
