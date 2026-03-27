#ifdef USE_DVG_RENDERER

// DVG (Digital Vector Generator) backend for the renderer abstraction.
// Uses the shared dvg_utils library (DvgCmd) for serial protocol, sorting,
// and frame building — same as bzone2 and opengw.
//
// Game coordinates (0..225 X, -28..270 Y) are mapped to DVG (0..4095).

#include "renderer.h"
#include "dvg_cmd.h"
#include "settings.h"

#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

// ─── Static state ────────────────────────────────────────────────────────────

static DvgCmd      g_dvg;
static SDL_Window* g_window       = NULL;
static int         g_screenWidth  = 0;
static int         g_screenHeight = 0;

// Display orientation (queried from DVG hardware)
static bool g_verticalDisplay = false;

// Line-vertex pair accumulator
static int   g_lineVertIdx = 0;
static float g_lineX0, g_lineY0;
static float g_lineR0, g_lineG0, g_lineB0, g_lineA0;

// Line-strip accumulator
static bool  g_stripHasPrev = false;
static float g_stripPrevX, g_stripPrevY;
static float g_stripPrevR, g_stripPrevG, g_stripPrevB, g_stripPrevA;

// Line-loop accumulator
static bool  g_loopHasPrev = false;
static bool  g_loopHasFirst = false;
static float g_loopFirstX, g_loopFirstY;
static float g_loopFirstR, g_loopFirstG, g_loopFirstB, g_loopFirstA;
static float g_loopPrevX, g_loopPrevY;
static float g_loopPrevR, g_loopPrevG, g_loopPrevB, g_loopPrevA;

static uint32_t g_targetFps = 40;

// ─── Helpers ─────────────────────────────────────────────────────────────────

//
// Map game coordinates to DVG coordinates (0..4095).
// Game field: X=0..225, Y=-28..270 (Y increases downward in game)
// DVG: X=0..4095, Y=0..4095 (Y increases upward)
//
static void gameToDvg(float gx, float gy, int32_t* outX, int32_t* outY)
{
    // Preserve aspect ratio: scale uniformly, center the shorter axis
    static const float gameW = 225.0f;
    static const float gameH = 298.0f;
    static const float scale = 4095.0f / gameH;  // fit the taller axis
    static const float offsetX = (4095.0f - gameW * scale) * 0.5f;

    *outX = (int32_t)(gx * scale + offsetX);
    *outY = (int32_t)((270.0f - gy) * scale);

    if (*outX < 0) *outX = 0;
    if (*outX > 4095) *outX = 4095;
    if (*outY < 0) *outY = 0;
    if (*outY > 4095) *outY = 4095;
}

static void colorToRgb(float r, float g, float b, float a,
                        uint8_t* outR, uint8_t* outG, uint8_t* outB)
{
    *outR = (uint8_t)std::min(std::max(r * a * 255.0f, 0.0f), 255.0f);
    *outG = (uint8_t)std::min(std::max(g * a * 255.0f, 0.0f), 255.0f);
    *outB = (uint8_t)std::min(std::max(b * a * 255.0f, 0.0f), 255.0f);
}

static void getDvgInfo()
{
    char jsonBuf[DVG_MAX_JSON_SIZE + 1];
    if (!g_dvg.queryDvgInfo(jsonBuf, sizeof(jsonBuf)))
    {
        fprintf(stderr, "getDvgInfo: query failed\n");
        return;
    }

    const char* p = strstr(jsonBuf, "\"vertical\"");
    if (p)
    {
        p += strlen("\"vertical\"");
        while (*p == ' ' || *p == ':') p++;
        g_verticalDisplay = (strncmp(p, "true", 4) == 0);
    }

    fprintf(stderr, "DVG JSON: %s\n", jsonBuf);
    fprintf(stderr, "DVG orientation: vertical=%d\n", g_verticalDisplay);
}

static void serialSend()
{
    g_dvg.buildFrameCommands();
    g_dvg.sendFrame();
}

// ─── Renderer interface ──────────────────────────────────────────────────────

namespace renderer
{
    void init(SDL_Window* window, int width, int height)
    {
        g_window       = window;
        g_screenWidth  = width;
        g_screenHeight = height;

        if (!g_dvg.init())
        {
            fprintf(stderr, "DVG renderer: allocation failed, exiting\n");
            exit(1);
        }

        if (!settings::serialPort[0])
        {
            fprintf(stderr, "DVG renderer: no serial port configured, exiting\n");
            exit(1);
        }

        if (!g_dvg.openSerial(settings::serialPort))
        {
            fprintf(stderr, "DVG renderer: failed to open %s, exiting\n", settings::serialPort);
            exit(1);
        }

        getDvgInfo();

        int cfgFps = settings::dvgRefreshRate;
        if (cfgFps < 5)  cfgFps = 5;
        if (cfgFps > 60) cfgFps = 60;
        g_targetFps = (uint32_t)cfgFps;

        fprintf(stderr, "DVG renderer initialized — %u fps\n", g_targetFps);
    }

    void shutdown()
    {
        if (g_dvg.isSerialOpen())
        {
            SDL_Delay(100);
            g_dvg.sendExitSequence();
            SDL_Delay(50);
            g_dvg.closeSerial();
        }

        g_dvg.shutdown();
        fprintf(stderr, "DVG renderer shutdown\n");
    }

    void beginFrame()
    {
        g_dvg.beginFrame();
    }

    void present()
    {
        if (g_dvg.isSerialOpen() && g_dvg.vecCount() > 0)
        {
            static Uint32 lastSendTime = 0;
            Uint32 now = SDL_GetTicks();
            Uint32 interval = 1000 / g_targetFps;
            if (now - lastSendTime >= interval)
            {
                lastSendTime = now;
                serialSend();
            }
        }

        g_dvg.beginFrame();
    }

    void enable2D(int screenW, int screenH)
    {
        g_screenWidth  = screenW;
        g_screenHeight = screenH;
    }

    void disable2D() {}
    void setColor(float r, float g, float b, float a) {}
    void setLineWidth(float w) {}
    void setPointSize(float s) {}
    void setBlendAdditive(bool enabled) {}
    void enableLineSmooth(bool enabled) {}

    // ── Lines ────────────────────────────────────────────────────────────────

    void beginLines()
    {
        g_lineVertIdx = 0;
    }

    void endLines() {}

    void lineVertex(float x, float y, float r, float g, float b, float a)
    {
        if (g_lineVertIdx == 0)
        {
            g_lineX0 = x;  g_lineY0 = y;
            g_lineR0 = r;  g_lineG0 = g;  g_lineB0 = b;  g_lineA0 = a;
            g_lineVertIdx = 1;
        }
        else
        {
            int32_t dvgX0, dvgY0, dvgX1, dvgY1;
            gameToDvg(g_lineX0, g_lineY0, &dvgX0, &dvgY0);
            gameToDvg(x, y, &dvgX1, &dvgY1);

            uint8_t cr, cg, cb;
            colorToRgb(r, g, b, a, &cr, &cg, &cb);

            g_dvg.addVec(dvgX0, dvgY0, dvgX1, dvgY1, cr, cg, cb);
            g_lineVertIdx = 0;
        }
    }

    // ── Line Strip ───────────────────────────────────────────────────────────

    void beginLineStrip()
    {
        g_stripHasPrev = false;
    }

    void endLineStrip()
    {
        g_stripHasPrev = false;
    }

    void stripVertex(float x, float y, float r, float g, float b, float a)
    {
        if (!g_stripHasPrev)
        {
            g_stripPrevX = x;  g_stripPrevY = y;
            g_stripPrevR = r;  g_stripPrevG = g;  g_stripPrevB = b;  g_stripPrevA = a;
            g_stripHasPrev = true;
        }
        else
        {
            int32_t dvgX0, dvgY0, dvgX1, dvgY1;
            gameToDvg(g_stripPrevX, g_stripPrevY, &dvgX0, &dvgY0);
            gameToDvg(x, y, &dvgX1, &dvgY1);

            uint8_t cr, cg, cb;
            colorToRgb(r, g, b, a, &cr, &cg, &cb);

            g_dvg.addVec(dvgX0, dvgY0, dvgX1, dvgY1, cr, cg, cb);

            g_stripPrevX = x;  g_stripPrevY = y;
            g_stripPrevR = r;  g_stripPrevG = g;  g_stripPrevB = b;  g_stripPrevA = a;
        }
    }

    // ── Line Loop ────────────────────────────────────────────────────────────

    void beginLineLoop()
    {
        g_loopHasPrev  = false;
        g_loopHasFirst = false;
    }

    void endLineLoop()
    {
        if (g_loopHasPrev && g_loopHasFirst)
        {
            int32_t dvgX0, dvgY0, dvgX1, dvgY1;
            gameToDvg(g_loopPrevX, g_loopPrevY, &dvgX0, &dvgY0);
            gameToDvg(g_loopFirstX, g_loopFirstY, &dvgX1, &dvgY1);

            uint8_t cr, cg, cb;
            colorToRgb(g_loopFirstR, g_loopFirstG, g_loopFirstB, g_loopFirstA, &cr, &cg, &cb);

            g_dvg.addVec(dvgX0, dvgY0, dvgX1, dvgY1, cr, cg, cb);
        }

        g_loopHasPrev  = false;
        g_loopHasFirst = false;
    }

    void loopVertex(float x, float y, float r, float g, float b, float a)
    {
        if (!g_loopHasFirst)
        {
            g_loopFirstX = x;  g_loopFirstY = y;
            g_loopFirstR = r;  g_loopFirstG = g;  g_loopFirstB = b;  g_loopFirstA = a;
            g_loopHasFirst = true;
        }

        if (g_loopHasPrev)
        {
            int32_t dvgX0, dvgY0, dvgX1, dvgY1;
            gameToDvg(g_loopPrevX, g_loopPrevY, &dvgX0, &dvgY0);
            gameToDvg(x, y, &dvgX1, &dvgY1);

            uint8_t cr, cg, cb;
            colorToRgb(r, g, b, a, &cr, &cg, &cb);

            g_dvg.addVec(dvgX0, dvgY0, dvgX1, dvgY1, cr, cg, cb);
        }

        g_loopPrevX = x;  g_loopPrevY = y;
        g_loopPrevR = r;  g_loopPrevG = g;  g_loopPrevB = b;  g_loopPrevA = a;
        g_loopHasPrev = true;
    }

    // ── Points ───────────────────────────────────────────────────────────────

    void beginPoints() {}
    void endPoints() {}

    void pointVertex(float x, float y, float r, float g, float b, float a)
    {
        int32_t dvgX, dvgY;
        gameToDvg(x, y, &dvgX, &dvgY);

        uint8_t cr, cg, cb;
        colorToRgb(r, g, b, a, &cr, &cg, &cb);

        g_dvg.addVec(dvgX, dvgY, dvgX + 1, dvgY, cr, cg, cb);
    }

    // ── Convenience ──────────────────────────────────────────────────────────

    void drawLine(float x1, float y1, float x2, float y2,
                  float r, float g, float b, float a)
    {
        int32_t dvgX0, dvgY0, dvgX1, dvgY1;
        gameToDvg(x1, y1, &dvgX0, &dvgY0);
        gameToDvg(x2, y2, &dvgX1, &dvgY1);

        uint8_t cr, cg, cb;
        colorToRgb(r, g, b, a, &cr, &cg, &cb);

        g_dvg.addVec(dvgX0, dvgY0, dvgX1, dvgY1, cr, cg, cb);
    }
}

#endif // USE_DVG_RENDERER
