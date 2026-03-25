#pragma once

#include "SDL.h"

// Input abstraction modeled after opengw's controls system.
// Supports keyboard and joystick; optionally reads advmame.rc bindings.

class controls
{
  public:
    controls();
    ~controls();

    void update();

    bool dirRight() const { return mRight; }
    bool dirLeft()  const { return mLeft; }
    bool dirUp()    const { return mUp; }
    bool dirDown()  const { return mDown; }

    bool startButton() const { return mStart; }
    bool coinButton()  const { return mCoin; }
    bool exitButton()  const { return mExit; }

    // Hot-plugging
    void handleGamepadAdded(SDL_JoystickID id);
    void handleGamepadRemoved(SDL_JoystickID id);
    void scanForGamepads();

  private:
    bool mRight, mLeft, mUp, mDown;
    bool mStart, mCoin, mExit;

    // Gamepads
    static constexpr int MAX_GAMEPADS = 4;
    SDL_GameController* mGamepads[MAX_GAMEPADS] {};
    int mNumGamepads { 0 };

    static constexpr int DEADZONE = 8000;
};
