#include "defines.h"
#include "game.h"
#include "renderer.h"
#include "settings.h"
#include "SDL.h"
#include <time.h>

game theGame;

static SDL_Window* g_window = nullptr;

int main(int argc, char* argv[])
{
    settings::load();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

#ifndef USE_DVG_RENDERER
    // GL attributes
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#endif

    int screenWidth = 800, screenHeight = 1000;

    g_window = SDL_CreateWindow(
        "Vector Pac-Man",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        screenWidth, screenHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!g_window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_ShowCursor(SDL_ENABLE);

    renderer::init(g_window, screenWidth, screenHeight);

    srand((unsigned int)time(nullptr));

    theGame.init();

    // Main loop
    bool running = true;
    Uint32 lastFrameTime = SDL_GetTicks();
    int frameCount = 0;
    int fps = 0;
    Uint32 fpsTime = lastFrameTime;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                running = false;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
                screenWidth = event.window.data1;
                screenHeight = event.window.data2;
            }
        }

        Uint32 now = SDL_GetTicks();
        if (now - lastFrameTime < 16) // ~60fps game logic
            continue;
        lastFrameTime = now;

        theGame.run();

        renderer::beginFrame();
        renderer::enable2D(screenWidth, screenHeight);
        theGame.draw();
        renderer::disable2D();
        renderer::present();

        // FPS counter
        ++frameCount;
        if (now - fpsTime > 1000) {
            fpsTime = now;
            fps = frameCount;
            frameCount = 0;
            char title[64];
            snprintf(title, sizeof(title), "Vector Pac-Man - FPS: %d", fps);
            SDL_SetWindowTitle(g_window, title);
        }
    }

    SDL_PauseAudio(1);
    renderer::shutdown();
    SDL_DestroyWindow(g_window);
    SDL_Quit();
    return 0;
}
