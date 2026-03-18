#ifndef USE_DVG_RENDERER

#include "renderer.h"
#include "defines.h"

#include "SDL.h"
#include <GL/gl.h>
#include <GL/glu.h>

namespace renderer
{
    static SDL_GLContext glContext = nullptr;
    static SDL_Window*   gWindow  = nullptr;

    // ---------------------------------------------------------------
    // Lifecycle
    // ---------------------------------------------------------------

    void init(SDL_Window* window, int w, int h)
    {
        gWindow = window;

        // GL attributes
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

        glContext = SDL_GL_CreateContext(window);

        // Vsync off
        SDL_GL_SetSwapInterval(0);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

        glEnable(GL_POINT_SMOOTH);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    }

    void shutdown()
    {
        if (glContext)
        {
            SDL_GL_DeleteContext(glContext);
            glContext = nullptr;
        }
        gWindow = nullptr;
    }

    // ---------------------------------------------------------------
    // Frame
    // ---------------------------------------------------------------

    void beginFrame()
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Default to additive blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }

    void present()
    {
        SDL_GL_SwapWindow(gWindow);
    }

    // ---------------------------------------------------------------
    // 2D projection
    // ---------------------------------------------------------------

    void enable2D(int screenW, int screenH)
    {
        // Visible area: X=0..225, Y=-28..270
        // -28..0 = score text area above maze
        // 0..248 = maze
        // 248..270 = lives/fruit area below maze
        static const float TOP = -28.0f;
        static const float BOTTOM = 270.0f;
        static const float LEFT = 0.0f;
        static const float RIGHT = (float)PACMAN_MAX_X;

        float gameW = RIGHT - LEFT;
        float gameH = BOTTOM - TOP;
        float gameAspect = gameW / gameH;
        float screenAspect = (float)screenW / (float)screenH;

        int vpX, vpY, vpW, vpH;

        if (screenAspect > gameAspect)
        {
            // Screen is wider than game -- pillarbox
            vpH = screenH;
            vpW = (int)(screenH * gameAspect);
            vpX = (screenW - vpW) / 2;
            vpY = 0;
        }
        else
        {
            // Screen is taller than game -- letterbox
            vpW = screenW;
            vpH = (int)(screenW / gameAspect);
            vpX = 0;
            vpY = (screenH - vpH) / 2;
        }

        glViewport(vpX, vpY, vpW, vpH);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        // Y flipped: top = TOP, bottom = BOTTOM
        glOrtho(LEFT, RIGHT, BOTTOM, TOP, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    void disable2D()
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    void sortBarrier()
    {
        // No-op for GL renderer
    }

    // ---------------------------------------------------------------
    // State helpers
    // ---------------------------------------------------------------

    void setColor(float r, float g, float b, float a)
    {
        glColor4f(r, g, b, a);
    }

    void setLineWidth(float w)
    {
        glLineWidth(w);
    }

    void setPointSize(float s)
    {
        glPointSize(s);
    }

    void setBlendAdditive(bool additive)
    {
        if (additive)
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        else
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void enableLineSmooth(bool enable)
    {
        if (enable)
        {
            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        }
        else
        {
            glDisable(GL_LINE_SMOOTH);
        }
    }

    // ---------------------------------------------------------------
    // GL_LINES
    // ---------------------------------------------------------------

    void beginLines()
    {
        glBegin(GL_LINES);
    }

    void endLines()
    {
        glEnd();
    }

    void lineVertex(float x, float y, float r, float g, float b, float a)
    {
        glColor4f(r, g, b, a);
        glVertex2f(x, y);
    }

    void drawLine(float x1, float y1, float x2, float y2,
                  float r, float g, float b, float a)
    {
        glBegin(GL_LINES);
        glColor4f(r, g, b, a);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glEnd();
    }

    // ---------------------------------------------------------------
    // GL_POINTS
    // ---------------------------------------------------------------

    void beginPoints()
    {
        glBegin(GL_POINTS);
    }

    void endPoints()
    {
        glEnd();
    }

    void pointVertex(float x, float y, float r, float g, float b, float a)
    {
        glColor4f(r, g, b, a);
        glVertex2f(x, y);
    }

    // ---------------------------------------------------------------
    // GL_LINE_LOOP
    // ---------------------------------------------------------------

    void beginLineLoop()
    {
        glBegin(GL_LINE_LOOP);
    }

    void endLineLoop()
    {
        glEnd();
    }

    void loopVertex(float x, float y, float r, float g, float b, float a)
    {
        glColor4f(r, g, b, a);
        glVertex2f(x, y);
    }

    // ---------------------------------------------------------------
    // GL_LINE_STRIP
    // ---------------------------------------------------------------

    void beginLineStrip()
    {
        glBegin(GL_LINE_STRIP);
    }

    void endLineStrip()
    {
        glEnd();
    }

    void stripVertex(float x, float y, float r, float g, float b, float a)
    {
        glColor4f(r, g, b, a);
        glVertex2f(x, y);
    }

} // namespace renderer

#endif // !USE_DVG_RENDERER
