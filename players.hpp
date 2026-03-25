#pragma once

#include "entityPacman.hpp"

class game;

// Player manager — owns the pacman entity.
// Modeled after opengw's players class.

class players
{
  public:
    players(game& gameRef);
    ~players();

    void run();
    void draw();

    entityPacman* getPacman() { return mPacman; }

  private:
    entityPacman* mPacman;
    game& mGame;
};
