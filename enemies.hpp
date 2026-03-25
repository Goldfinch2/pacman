#pragma once

#include "entityGhost.hpp"
#include "entityBlinky.hpp"
#include "entityPinky.hpp"
#include "entityInky.hpp"
#include "entityClyde.hpp"

class game;

// Ghost manager — owns and dispatches run/draw for all 4 ghosts.
// Modeled after opengw's enemies class.

class enemies
{
  public:
    enemies(game& gameRef);
    ~enemies();

    void run();
    void draw();

    entityBlinky* getBlinky() { return mBlinky; }
    entityPinky*  getPinky()  { return mPinky; }
    entityInky*   getInky()   { return mInky; }
    entityClyde*  getClyde()  { return mClyde; }

    entityGhost* getGhost(int index);

    void initAll();
    void resetAll();
    void frightenAll();
    void reverseAll();

    static constexpr int NUM_GHOSTS = 4;

  private:
    entityBlinky* mBlinky;
    entityPinky*  mPinky;
    entityInky*   mInky;
    entityClyde*  mClyde;

    game& mGame;
};
