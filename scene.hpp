#pragma once

#include <memory>

class game;

// Scene — top-level wrapper that owns the game instance.
// Modeled after opengw's scene class.

class scene
{
  public:
    scene();
    ~scene();

    void init();
    void shutdown();
    void run();
    void draw();

  private:
    std::unique_ptr<game> mGame;
};

extern scene g_scene;
