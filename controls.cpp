#include "controls.hpp"
#include "settings.h"
#include "advmame_rc.h"
#include <cstring>

controls::controls()
    : mRight(false), mLeft(false), mUp(false), mDown(false)
    , mStart(false), mCoin(false), mExit(false)
{
    memset(mGamepads, 0, sizeof(mGamepads));
    scanForGamepads();
}

controls::~controls()
{
    for (int i = 0; i < MAX_GAMEPADS; i++) {
        if (mGamepads[i]) {
            SDL_GameControllerClose(mGamepads[i]);
            mGamepads[i] = nullptr;
        }
    }
}

void controls::scanForGamepads()
{
    mNumGamepads = 0;
    int numJoysticks = SDL_NumJoysticks();
    for (int i = 0; i < numJoysticks && mNumGamepads < MAX_GAMEPADS; i++) {
        if (SDL_IsGameController(i)) {
            mGamepads[mNumGamepads] = SDL_GameControllerOpen(i);
            if (mGamepads[mNumGamepads])
                mNumGamepads++;
        }
    }
}

void controls::handleGamepadAdded(SDL_JoystickID id)
{
    if (mNumGamepads >= MAX_GAMEPADS) return;
    int deviceIndex = -1;
    int numJoysticks = SDL_NumJoysticks();
    for (int i = 0; i < numJoysticks; i++) {
        if (SDL_JoystickGetDeviceInstanceID(i) == id) {
            deviceIndex = i;
            break;
        }
    }
    if (deviceIndex >= 0 && SDL_IsGameController(deviceIndex)) {
        mGamepads[mNumGamepads] = SDL_GameControllerOpen(deviceIndex);
        if (mGamepads[mNumGamepads])
            mNumGamepads++;
    }
}

void controls::handleGamepadRemoved(SDL_JoystickID id)
{
    for (int i = 0; i < MAX_GAMEPADS; i++) {
        if (mGamepads[i] && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(mGamepads[i])) == id) {
            SDL_GameControllerClose(mGamepads[i]);
            mGamepads[i] = nullptr;
            // Compact the array
            for (int j = i; j < MAX_GAMEPADS - 1; j++)
                mGamepads[j] = mGamepads[j + 1];
            mGamepads[MAX_GAMEPADS - 1] = nullptr;
            mNumGamepads--;
            break;
        }
    }
}

void controls::update()
{
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    // Build joystick array from open game controllers for advmameInputIsActive
    SDL_Joystick* joysticks[MAX_GAMEPADS] = {};
    int numJoy = 0;
    for (int i = 0; i < mNumGamepads; i++) {
        if (mGamepads[i])
            joysticks[numJoy++] = SDL_GameControllerGetJoystick(mGamepads[i]);
    }

    // Check advmame.rc bindings (keyboard + joystick axis/button)
    mRight = advmameInputIsActive(settings::rightAction, keys, 0, joysticks, numJoy);
    mLeft  = advmameInputIsActive(settings::leftAction,  keys, 0, joysticks, numJoy);
    mUp    = advmameInputIsActive(settings::upAction,    keys, 0, joysticks, numJoy);
    mDown  = advmameInputIsActive(settings::downAction,  keys, 0, joysticks, numJoy);
    mStart = advmameInputIsActive(settings::startAction, keys, 0, joysticks, numJoy);
    mCoin  = advmameInputIsActive(settings::coinAction,  keys, 0, joysticks, numJoy);
    mExit  = advmameInputIsActive(settings::exitAction,  keys, 0, joysticks, numJoy);

    // D-pad fallback via GameController API (not covered by advmame.rc joystick bindings)
    for (int i = 0; i < mNumGamepads; i++) {
        if (!mGamepads[i]) continue;
        if (SDL_GameControllerGetButton(mGamepads[i], SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) mRight = true;
        if (SDL_GameControllerGetButton(mGamepads[i], SDL_CONTROLLER_BUTTON_DPAD_LEFT))  mLeft  = true;
        if (SDL_GameControllerGetButton(mGamepads[i], SDL_CONTROLLER_BUTTON_DPAD_UP))    mUp    = true;
        if (SDL_GameControllerGetButton(mGamepads[i], SDL_CONTROLLER_BUTTON_DPAD_DOWN))  mDown  = true;
        if (SDL_GameControllerGetButton(mGamepads[i], SDL_CONTROLLER_BUTTON_START))      mStart = true;
        if (SDL_GameControllerGetButton(mGamepads[i], SDL_CONTROLLER_BUTTON_BACK))       mCoin  = true;
    }
}
