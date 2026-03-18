#ifdef USE_DVG_RENDERER

// DVG (Digital Vector Generator) backend for the renderer abstraction.
// Outputs real vectors to a vector CRT via DVG hardware connected as a
// CDC serial port.  Based on the USB-DVG MAME driver by Mario Montminy.
//
// Adapted for Pac-Man: game coordinates (0..225 X, -28..270 Y) are mapped
// directly to DVG coordinates (0..4095) instead of going through NDC.

#include "renderer.h"
#include "renderer_dvg.h"
#include "dvg_serial.h"
#include "settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

// ─── Internal types ──────────────────────────────────────────────────────────

struct dvg_vec_t
{
    int32_t x0, y0, x1, y1;
    uint8_t r, g, b;
};

// ─── Static state ────────────────────────────────────────────────────────────

static SDL_Window* g_window       = NULL;
static int         g_screenWidth  = 0;
static int         g_screenHeight = 0;

// Command buffer (sent to DVG each frame)
static uint8_t*  g_cmdBuf  = NULL;
static uint32_t  g_cmdOffs = 0;

// Input vector list (accumulated during the frame)
static dvg_vec_t* g_vecList = NULL;
static uint32_t   g_vecCnt  = 0;

// Last sent RGB (for change-detection optimisation)
static int32_t g_lastR = -1, g_lastG = -1, g_lastB = -1;

// Beam position as the firmware last left it — persists across frames so that
// the blank repositioning move at the start of each frame is correctly omitted
// only when the beam is already at the right position.
static int32_t g_beamEndX = 0, g_beamEndY = 0;

// Display orientation (queried from DVG hardware)
static bool g_verticalDisplay = false;

// Line-vertex pair accumulator (for beginLines/endLines)
static int   g_lineVertIdx = 0;
static float g_lineX0, g_lineY0;
static float g_lineR0, g_lineG0, g_lineB0, g_lineA0;

// Line-strip accumulator (for beginLineStrip/endLineStrip)
static bool  g_stripActive = false;
static bool  g_stripHasPrev = false;
static float g_stripPrevX, g_stripPrevY;
static float g_stripPrevR, g_stripPrevG, g_stripPrevB, g_stripPrevA;

// Line-loop accumulator (for beginLineLoop/endLineLoop)
static bool  g_loopActive = false;
static bool  g_loopHasPrev = false;
static bool  g_loopHasFirst = false;
static float g_loopFirstX, g_loopFirstY;
static float g_loopFirstR, g_loopFirstG, g_loopFirstB, g_loopFirstA;
static float g_loopPrevX, g_loopPrevY;
static float g_loopPrevR, g_loopPrevG, g_loopPrevB, g_loopPrevA;

static uint32_t g_targetFps = 40;  // DVG frame rate

// ─── Helpers ─────────────────────────────────────────────────────────────────

//
// Map game coordinates to DVG coordinates (0..4095).
// Game field: X=0..225, Y=-28..270 (Y increases downward in game)
// DVG: X=0..4095, Y=0..4095 (Y increases upward)
//
static void gameToDvg(float gx, float gy, int32_t* outX, int32_t* outY)
{
    // Y always uses full range (flip: game Y-down to DVG Y-up)
    *outY = (int32_t)((270.0f - gy) * 4095.0f / 298.0f);

    if (g_verticalDisplay)
    {
        // Vertical (3:4) display — game aspect ~3:4, fills naturally
        *outX = (int32_t)(gx * 4095.0f / 225.0f);
    }
    else
    {
        // Horizontal (4:3) display — pillarbox to maintain correct aspect ratio
        float xScale  = (3.0f / 4.0f) * (4095.0f / 298.0f);
        float xRange  = 225.0f * xScale;
        float xOffset = (4095.0f - xRange) / 2.0f;
        *outX = (int32_t)(gx * xScale + xOffset);
    }

    if (*outX < 0) *outX = 0;
    if (*outX > 4095) *outX = 4095;
    if (*outY < 0) *outY = 0;
    if (*outY > 4095) *outY = 4095;
}

//
// Cohen-Sutherland line clipping to DVG coordinate range (0..4095).
// Returns true if the (possibly shortened) line is visible.
//
enum { CS_INSIDE=0, CS_LEFT=1, CS_RIGHT=2, CS_BOTTOM=4, CS_TOP=8 };

static int csCode(int32_t x, int32_t y)
{
    int code = CS_INSIDE;
    if (x < (int32_t)DVG_RES_MIN) code |= CS_LEFT;
    else if (x > (int32_t)DVG_RES_MAX) code |= CS_RIGHT;
    if (y < (int32_t)DVG_RES_MIN) code |= CS_BOTTOM;
    else if (y > (int32_t)DVG_RES_MAX) code |= CS_TOP;
    return code;
}

static bool clipLine(int32_t* x0, int32_t* y0, int32_t* x1, int32_t* y1)
{
    int code0 = csCode(*x0, *y0);
    int code1 = csCode(*x1, *y1);

    while (true)
    {
        if (!(code0 | code1))
            return true;  // Both inside
        if (code0 & code1)
            return false; // Both outside same edge

        int code = code0 ? code0 : code1;
        int32_t x, y;

        if (code & CS_TOP) {
            x = *x0 + (int32_t)((int64_t)(*x1 - *x0) * (DVG_RES_MAX - *y0) / (*y1 - *y0));
            y = DVG_RES_MAX;
        } else if (code & CS_BOTTOM) {
            x = *x0 + (int32_t)((int64_t)(*x1 - *x0) * (DVG_RES_MIN - *y0) / (*y1 - *y0));
            y = DVG_RES_MIN;
        } else if (code & CS_RIGHT) {
            y = *y0 + (int32_t)((int64_t)(*y1 - *y0) * (DVG_RES_MAX - *x0) / (*x1 - *x0));
            x = DVG_RES_MAX;
        } else {
            y = *y0 + (int32_t)((int64_t)(*y1 - *y0) * (DVG_RES_MIN - *x0) / (*x1 - *x0));
            x = DVG_RES_MIN;
        }

        if (code == code0) {
            *x0 = x; *y0 = y;
            code0 = csCode(*x0, *y0);
        } else {
            *x1 = x; *y1 = y;
            code1 = csCode(*x1, *y1);
        }
    }
}

//
// Convert float RGBA (0..1) to uint8 RGB, pre-multiplied by alpha.
//
static void colorToRgb(float r, float g, float b, float a,
                        uint8_t* outR, uint8_t* outG, uint8_t* outB)
{
    *outR = (uint8_t)std::min(std::max(r * a * 255.0f, 0.0f), 255.0f);
    *outG = (uint8_t)std::min(std::max(g * a * 255.0f, 0.0f), 255.0f);
    *outB = (uint8_t)std::min(std::max(b * a * 255.0f, 0.0f), 255.0f);
}

//
// Append a 4-byte big-endian command word to the command buffer.
//
static void cmdAppend(uint32_t cmd)
{
    if (g_cmdOffs <= (DVG_CMD_BUF_SIZE - 4))
    {
        g_cmdBuf[g_cmdOffs++] = (uint8_t)(cmd >> 24);
        g_cmdBuf[g_cmdOffs++] = (uint8_t)(cmd >> 16);
        g_cmdBuf[g_cmdOffs++] = (uint8_t)(cmd >>  8);
        g_cmdBuf[g_cmdOffs++] = (uint8_t)(cmd >>  0);
    }
}

//
// Emit DVG draw-to command for one point.
// Sends an RGB command first if the color has changed.
// Sets the blank flag in the XY command when color is black.
//
static void cmdAddPoint(int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t blank = (r == 0) && (g == 0) && (b == 0);

    if (!blank)
    {
        if (r != g_lastR || g != g_lastG || b != g_lastB)
        {
            g_lastR = r;
            g_lastG = g;
            g_lastB = b;
            cmdAppend((DVG_FLAG_RGB << 29) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff));
        }
    }
    cmdAppend((DVG_FLAG_XY << 29) | ((blank & 0x1) << 28) | ((x & 0x3fff) << 14) | (y & 0x3fff));
}

//
// Add a vector to the input list.
//
static void addVec(int32_t x0, int32_t y0, int32_t x1, int32_t y1,
                   uint8_t r, uint8_t g, uint8_t b)
{
    // Skip zero-length vectors
    if (x0 == x1 && y0 == y1)
        return;

    // Clip line to DVG bounds
    if (!clipLine(&x0, &y0, &x1, &y1))
        return;

    if (g_vecCnt < DVG_MAX_VECTORS)
    {
        g_vecList[g_vecCnt].x0 = x0;
        g_vecList[g_vecCnt].y0 = y0;
        g_vecList[g_vecCnt].x1 = x1;
        g_vecList[g_vecCnt].y1 = y1;
        g_vecList[g_vecCnt].r  = r;
        g_vecList[g_vecCnt].g  = g;
        g_vecList[g_vecCnt].b  = b;
        g_vecCnt++;
    }
}

// ─── DVG info query ──────────────────────────────────────────────────────────

//
// Query the DVG hardware for its configuration JSON.
// Parses the "vertical" field to determine display orientation.
//
static void getDvgInfo()
{
    uint32_t cmd = (DVG_FLAG_CMD << 29) | DVG_FLAG_CMD_GET_DVG_INFO;
    uint8_t cmd_buf[4];
    cmd_buf[0] = (uint8_t)(cmd >> 24);
    cmd_buf[1] = (uint8_t)(cmd >> 16);
    cmd_buf[2] = (uint8_t)(cmd >>  8);
    cmd_buf[3] = (uint8_t)(cmd >>  0);

    if (dvg_serial::write(cmd_buf, 4) != 4)
    {
        fprintf(stderr, "getDvgInfo: write failed\n");
        return;
    }

    // Read 4-byte ack
    uint32_t ack;
    int ackRead = dvg_serial::read(&ack, 4);
    if (ackRead != 4)
    {
        fprintf(stderr, "getDvgInfo: ack read failed (got %d bytes)\n", ackRead);
        return;
    }

    // Read 4-byte JSON length
    uint32_t jsonLen;
    int lenRead = dvg_serial::read(&jsonLen, 4);
    if (lenRead != 4)
    {
        fprintf(stderr, "getDvgInfo: length read failed (got %d bytes)\n", lenRead);
        return;
    }
    if (jsonLen == 0 || jsonLen >= DVG_MAX_JSON_SIZE)
    {
        fprintf(stderr, "getDvgInfo: bad length %u\n", jsonLen);
        return;
    }

    // Read JSON payload
    char jsonBuf[DVG_MAX_JSON_SIZE];
    int jsonRead = dvg_serial::read(jsonBuf, jsonLen);
    if (jsonRead != (int)jsonLen)
    {
        fprintf(stderr, "getDvgInfo: payload read failed (got %d of %u bytes)\n", jsonRead, jsonLen);
        return;
    }
    jsonBuf[jsonLen] = '\0';

    // Parse "vertical" field
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

// ─── Serial I/O ──────────────────────────────────────────────────────────────

//
// Write the command buffer to the DVG in 512-byte chunks.
//
static int serialWrite(uint8_t* buf, int size)
{
    int total = size;
    while (size > 0)
    {
        int chunk = std::min(size, 512);
        int written = dvg_serial::write(buf, chunk);
        if (written != chunk)
            return -1;
        buf  += chunk;
        size -= chunk;
    }
    return total;
}

//
// Sort vectors using chain-based grid sort.
//
// 1. Detect connected chains: consecutive vectors where endpoint == next start.
// 2. Assign each chain to a grid cell based on its midpoint.
// 3. Traverse the grid in a snake pattern (stable frame-to-frame order).
// 4. Within each cell, pick chains by nearest-neighbor.
// 5. Emit each chain's vectors in order, preserving connectivity.
//
#define GRID_COLS 8
#define GRID_ROWS 8
#define GRID_NUM_CELLS (GRID_COLS * GRID_ROWS)

struct dvg_chain_t
{
    uint16_t start;
    uint16_t count;
    int32_t  x0, y0;
    int32_t  x1, y1;
};

static dvg_chain_t g_chains[DVG_MAX_VECTORS];
static uint16_t    g_chainCnt;
static dvg_vec_t   g_sortTemp[DVG_MAX_VECTORS];
static uint16_t    g_cellCnt[GRID_NUM_CELLS];
static uint16_t    g_cellOffset[GRID_NUM_CELLS];
static uint16_t    g_cellBins[DVG_MAX_VECTORS];
static uint8_t     g_sortUsed[DVG_MAX_VECTORS];

static void sortVectors()
{
    if (g_vecCnt <= 1)
        return;

    // Pass 1: detect connected chains
    g_chainCnt = 0;
    g_chains[0].start = 0;
    g_chains[0].count = 1;
    g_chains[0].x0 = g_vecList[0].x0;
    g_chains[0].y0 = g_vecList[0].y0;

    for (uint32_t i = 1; i < g_vecCnt; i++)
    {
        if (g_vecList[i].x0 == g_vecList[i - 1].x1 &&
            g_vecList[i].y0 == g_vecList[i - 1].y1)
        {
            g_chains[g_chainCnt].count++;
        }
        else
        {
            g_chains[g_chainCnt].x1 = g_vecList[i - 1].x1;
            g_chains[g_chainCnt].y1 = g_vecList[i - 1].y1;
            g_chainCnt++;
            g_chains[g_chainCnt].start = (uint16_t)i;
            g_chains[g_chainCnt].count = 1;
            g_chains[g_chainCnt].x0 = g_vecList[i].x0;
            g_chains[g_chainCnt].y0 = g_vecList[i].y0;
        }
    }
    g_chains[g_chainCnt].x1 = g_vecList[g_vecCnt - 1].x1;
    g_chains[g_chainCnt].y1 = g_vecList[g_vecCnt - 1].y1;
    g_chainCnt++;

    // Pass 2: assign each chain to a grid cell
    memset(g_cellCnt, 0, sizeof(g_cellCnt));
    for (uint32_t i = 0; i < g_chainCnt; i++)
    {
        int32_t mx = (g_chains[i].x0 + g_chains[i].x1) / 2;
        int32_t my = (g_chains[i].y0 + g_chains[i].y1) / 2;
        int col = mx * GRID_COLS / (DVG_RES_MAX + 1);
        int row = my * GRID_ROWS / (DVG_RES_MAX + 1);
        if (col < 0) col = 0;
        if (col >= GRID_COLS) col = GRID_COLS - 1;
        if (row < 0) row = 0;
        if (row >= GRID_ROWS) row = GRID_ROWS - 1;
        g_sortUsed[i] = (uint8_t)(row * GRID_COLS + col);
        g_cellCnt[row * GRID_COLS + col]++;
    }

    g_cellOffset[0] = 0;
    for (int i = 1; i < GRID_NUM_CELLS; i++)
        g_cellOffset[i] = g_cellOffset[i - 1] + g_cellCnt[i - 1];

    memset(g_cellCnt, 0, sizeof(g_cellCnt));
    for (uint32_t i = 0; i < g_chainCnt; i++)
    {
        uint8_t c = g_sortUsed[i];
        g_cellBins[g_cellOffset[c] + g_cellCnt[c]++] = (uint16_t)i;
    }

    memset(g_sortUsed, 0, g_chainCnt);

    // Pass 3: traverse grid in snake pattern, nearest-neighbor on chains
    uint32_t outCnt = 0;
    int32_t lastX = 0, lastY = 0;

    for (int row = 0; row < GRID_ROWS; row++)
    {
        for (int i = 0; i < GRID_COLS; i++)
        {
            int col = (row & 1) ? (GRID_COLS - 1 - i) : i;
            int cell = row * GRID_COLS + col;
            uint16_t cnt = g_cellCnt[cell];
            if (cnt == 0) continue;
            uint16_t* bin = &g_cellBins[g_cellOffset[cell]];

            uint16_t remaining = cnt;
            while (remaining > 0)
            {
                int64_t dmin = INT64_MAX;
                uint16_t bestIdx = 0;
                bool bestReverse = false;

                for (uint16_t j = 0; j < cnt; j++)
                {
                    if (g_sortUsed[bin[j]]) continue;
                    const dvg_chain_t& ch = g_chains[bin[j]];
                    int64_t dx0 = (int64_t)(ch.x0 - lastX);
                    int64_t dy0 = (int64_t)(ch.y0 - lastY);
                    int64_t dx1 = (int64_t)(ch.x1 - lastX);
                    int64_t dy1 = (int64_t)(ch.y1 - lastY);
                    int64_t d0 = dx0 * dx0 + dy0 * dy0;
                    int64_t d1 = dx1 * dx1 + dy1 * dy1;
                    int64_t d = (d0 < d1) ? d0 : d1;
                    if (d < dmin) { bestIdx = j; dmin = d; bestReverse = false; }
                    if (dmin == 0) break;
                }

                g_sortUsed[bin[bestIdx]] = true;
                remaining--;
                const dvg_chain_t& ch = g_chains[bin[bestIdx]];

                // Emit chain vectors (forward or reversed)
                if (bestReverse)
                {
                    for (int32_t k = ch.start + ch.count - 1; k >= (int32_t)ch.start; k--)
                    {
                        g_sortTemp[outCnt].x0 = g_vecList[k].x1;
                        g_sortTemp[outCnt].y0 = g_vecList[k].y1;
                        g_sortTemp[outCnt].x1 = g_vecList[k].x0;
                        g_sortTemp[outCnt].y1 = g_vecList[k].y0;
                        g_sortTemp[outCnt].r = g_vecList[k].r;
                        g_sortTemp[outCnt].g = g_vecList[k].g;
                        g_sortTemp[outCnt].b = g_vecList[k].b;
                        outCnt++;
                    }
                    lastX = ch.x0;
                    lastY = ch.y0;
                }
                else
                {
                    for (uint32_t k = ch.start; k < (uint32_t)(ch.start + ch.count); k++)
                    {
                        g_sortTemp[outCnt] = g_vecList[k];
                        outCnt++;
                    }
                    lastX = ch.x1;
                    lastY = ch.y1;
                }
            }
        }
    }

    memcpy(g_vecList, g_sortTemp, outCnt * sizeof(dvg_vec_t));
    g_vecCnt = outCnt;
}

//
// Serialise the current g_vecList into g_cmdBuf (resets buffer first).
// Inserts blank repositioning moves where the beam needs to jump.
//
static void buildFrameCommands()
{
    g_cmdOffs = 0;
    g_lastR = g_lastG = g_lastB = -1;

    // Start from where the firmware's beam actually is, not from assumed (0,0).
    int32_t beamX = g_beamEndX, beamY = g_beamEndY;

    for (uint32_t i = 0; i < g_vecCnt; i++)
    {
        int32_t x0 = g_vecList[i].x0, y0 = g_vecList[i].y0;
        int32_t x1 = g_vecList[i].x1, y1 = g_vecList[i].y1;

        if (beamX != x0 || beamY != y0)
            cmdAddPoint(x0, y0, 0, 0, 0);

        cmdAddPoint(x1, y1, g_vecList[i].r, g_vecList[i].g, g_vecList[i].b);
        beamX = x1;
        beamY = y1;
    }
    g_beamEndX = beamX;
    g_beamEndY = beamY;

    cmdAppend(DVG_FLAG_COMPLETE << 29);
}

// ─── Main send path ──────────────────────────────────────────────────────────

//
// Serialise the input vector list to the command buffer and send to DVG.
//
static void serialSend()
{
    // ── Input handling ────────────────────────────────────────────────────
    SDL_Event e;
    while (SDL_PollEvent(&e))
        if (e.type == SDL_QUIT) exit(0);

    SDL_PumpEvents();
    const uint8_t* keys = SDL_GetKeyboardState(NULL);

    static bool prevUp = false, prevDown = false;
    bool curUp   = keys[SDL_SCANCODE_UP]   != 0;
    bool curDown = keys[SDL_SCANCODE_DOWN] != 0;

    if (curUp   && !prevUp   && g_targetFps < 60) g_targetFps++;
    if (curDown && !prevDown && g_targetFps > 5)  g_targetFps--;

    prevUp   = curUp;
    prevDown = curDown;

    // ── Build and send (sorting done via sortBarrier calls) ────────────────
    buildFrameCommands();
    serialWrite(g_cmdBuf, g_cmdOffs);
}

// ─── Renderer interface ──────────────────────────────────────────────────────

namespace renderer
{
    void init(SDL_Window* window, int width, int height)
    {
        g_window       = window;
        g_screenWidth  = width;
        g_screenHeight = height;

        g_cmdBuf  = (uint8_t*)malloc(DVG_CMD_BUF_SIZE);
        g_vecList = (dvg_vec_t*)malloc(DVG_MAX_VECTORS * sizeof(dvg_vec_t));

        if (!settings::serialPort[0])
        {
            fprintf(stderr, "DVG renderer: no serial port configured, exiting\n");
            exit(1);
        }

        if (!dvg_serial::open(settings::serialPort))
        {
            fprintf(stderr, "DVG renderer: failed to open %s, exiting\n", settings::serialPort);
            exit(1);
        }

        getDvgInfo();

        // Initialise send rate from settings (clamped to valid range).
        int cfgFps = settings::dvgRefreshRate;
        if (cfgFps < 5)  cfgFps = 5;
        if (cfgFps > 60) cfgFps = 60;
        g_targetFps = (uint32_t)cfgFps;

        fprintf(stderr, "DVG renderer initialized — %u fps\n", g_targetFps);
    }

    void shutdown()
    {
        if (dvg_serial::isOpen())
        {
            // Drain any pending game frame data from the transmit buffer
            // so the EXIT command doesn't get queued behind it.
            dvg_serial::flush();
            SDL_Delay(100);

            // Send several COMPLETEs to flush any in-progress frame the
            // firmware may be waiting on, then EXIT to return to test pattern.
            g_cmdOffs = 0;
            for (int i = 0; i < 16; i++)
                cmdAppend(DVG_FLAG_COMPLETE << 29);
            cmdAppend(DVG_FLAG_EXIT << 29);
            serialWrite(g_cmdBuf, g_cmdOffs);
            dvg_serial::flush();

            // Brief delay for USB to deliver the bytes, then close immediately.
            SDL_Delay(50);
            dvg_serial::close();
        }

        free(g_cmdBuf);  g_cmdBuf  = NULL;
        free(g_vecList); g_vecList = NULL;

        fprintf(stderr, "DVG renderer shutdown\n");
    }

    void beginFrame()
    {
        g_cmdOffs = 0;
        g_vecCnt  = 0;
        g_lastR = g_lastG = g_lastB = -1;
    }

    void present()
    {
        if (dvg_serial::isOpen() && g_vecCnt > 0)
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

        // Reset for next frame
        g_cmdOffs = 0;
        g_vecCnt  = 0;
    }

    void sortBarrier()
    {
        if (settings::sortVectors && g_vecCnt > 1)
            sortVectors();
    }

    void enable2D(int screenW, int screenH)
    {
        g_screenWidth  = screenW;
        g_screenHeight = screenH;
    }

    void disable2D()
    {
        // No-op
    }

    void setColor(float r, float g, float b, float a)
    {
        // Color is per-vertex in this renderer, not global state
    }

    void setLineWidth(float w)
    {
        // Vector CRT beam width is fixed by hardware
    }

    void setPointSize(float s)
    {
        // Vector CRT dot size is fixed by hardware
    }

    void setBlendAdditive(bool enabled)
    {
        // Vectors are naturally additive on a CRT
    }

    void enableLineSmooth(bool enabled)
    {
        // Vectors are inherently smooth on a CRT
    }

    // ── Lines ────────────────────────────────────────────────────────────────

    void beginLines()
    {
        g_lineVertIdx = 0;
    }

    void endLines()
    {
        // Nothing to flush — pairs are emitted immediately
    }

    void lineVertex(float x, float y, float r, float g, float b, float a)
    {
        if (g_lineVertIdx == 0)
        {
            // First vertex of the pair — just store it
            g_lineX0 = x;  g_lineY0 = y;
            g_lineR0 = r;  g_lineG0 = g;  g_lineB0 = b;  g_lineA0 = a;
            g_lineVertIdx = 1;
        }
        else
        {
            // Second vertex — emit a line segment
            int32_t dvgX0, dvgY0, dvgX1, dvgY1;
            gameToDvg(g_lineX0, g_lineY0, &dvgX0, &dvgY0);
            gameToDvg(x, y, &dvgX1, &dvgY1);

            // Use the endpoint color (matches GL which sets color per-vertex)
            uint8_t cr, cg, cb;
            colorToRgb(r, g, b, a, &cr, &cg, &cb);

            addVec(dvgX0, dvgY0, dvgX1, dvgY1, cr, cg, cb);
            g_lineVertIdx = 0;
        }
    }

    // ── Line Strip ───────────────────────────────────────────────────────────

    void beginLineStrip()
    {
        g_stripActive  = true;
        g_stripHasPrev = false;
    }

    void endLineStrip()
    {
        g_stripActive  = false;
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

            addVec(dvgX0, dvgY0, dvgX1, dvgY1, cr, cg, cb);

            g_stripPrevX = x;  g_stripPrevY = y;
            g_stripPrevR = r;  g_stripPrevG = g;  g_stripPrevB = b;  g_stripPrevA = a;
        }
    }

    // ── Line Loop ────────────────────────────────────────────────────────────

    void beginLineLoop()
    {
        g_loopActive   = true;
        g_loopHasPrev  = false;
        g_loopHasFirst = false;
    }

    void endLineLoop()
    {
        // Close the loop: connect last vertex back to first
        if (g_loopHasPrev && g_loopHasFirst)
        {
            int32_t dvgX0, dvgY0, dvgX1, dvgY1;
            gameToDvg(g_loopPrevX, g_loopPrevY, &dvgX0, &dvgY0);
            gameToDvg(g_loopFirstX, g_loopFirstY, &dvgX1, &dvgY1);

            uint8_t cr, cg, cb;
            colorToRgb(g_loopFirstR, g_loopFirstG, g_loopFirstB, g_loopFirstA, &cr, &cg, &cb);

            addVec(dvgX0, dvgY0, dvgX1, dvgY1, cr, cg, cb);
        }

        g_loopActive   = false;
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

            addVec(dvgX0, dvgY0, dvgX1, dvgY1, cr, cg, cb);
        }

        g_loopPrevX = x;  g_loopPrevY = y;
        g_loopPrevR = r;  g_loopPrevG = g;  g_loopPrevB = b;  g_loopPrevA = a;
        g_loopHasPrev = true;
    }

    // ── Points ───────────────────────────────────────────────────────────────

    void beginPoints()
    {
        // No-op
    }

    void endPoints()
    {
        // No-op
    }

    void pointVertex(float x, float y, float r, float g, float b, float a)
    {
        int32_t dvgX, dvgY;
        gameToDvg(x, y, &dvgX, &dvgY);

        uint8_t cr, cg, cb;
        colorToRgb(r, g, b, a, &cr, &cg, &cb);

        // Short horizontal vector to create a visible dot on the CRT
        addVec(dvgX, dvgY, dvgX + 1, dvgY, cr, cg, cb);
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

        addVec(dvgX0, dvgY0, dvgX1, dvgY1, cr, cg, cb);
    }
}

#endif // USE_DVG_RENDERER
