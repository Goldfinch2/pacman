#include "controls.hpp"
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
    // Keyboard
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    mRight = keys[SDL_SCANCODE_RIGHT] != 0;
    mLeft  = keys[SDL_SCANCODE_LEFT]  != 0;
    mUp    = keys[SDL_SCANCODE_UP]    != 0;
    mDown  = keys[SDL_SCANCODE_DOWN]  != 0;
    mStart = keys[SDL_SCANCODE_RETURN] != 0 || keys[SDL_SCANCODE_SPACE] != 0 || keys[SDL_SCANCODE_LCTRL] != 0;
    mCoin  = keys[SDL_SCANCODE_5] != 0;
    mExit  = keys[SDL_SCANCODE_ESCAPE] != 0;

    // Overlay gamepad input (any connected pad)
    for (int i = 0; i < mNumGamepads; i++) {
        if (!mGamepads[i]) continue;

        // Left stick or D-pad for directions
        Sint16 lx = SDL_GameControllerGetAxis(mGamepads[i], SDL_CONTROLLER_AXIS_LEFTX);
        Sint16 ly = SDL_GameControllerGetAxis(mGamepads[i], SDL_CONTROLLER_AXIS_LEFTY);

        if (lx >  DEADZONE) mRight = true;
        if (lx < -DEADZONE) mLeft  = true;
        if (ly < -DEADZONE) mUp    = true;
        if (ly >  DEADZONE) mDown  = true;

        if (SDL_GameControllerGetButton(mGamepads[i], SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) mRight = true;
        if (SDL_GameControllerGetButton(mGamepads[i], SDL_CONTROLLER_BUTTON_DPAD_LEFT))  mLeft  = true;
        if (SDL_GameControllerGetButton(mGamepads[i], SDL_CONTROLLER_BUTTON_DPAD_UP))    mUp    = true;
        if (SDL_GameControllerGetButton(mGamepads[i], SDL_CONTROLLER_BUTTON_DPAD_DOWN))  mDown  = true;

        if (SDL_GameControllerGetButton(mGamepads[i], SDL_CONTROLLER_BUTTON_START)) mStart = true;
        if (SDL_GameControllerGetButton(mGamepads[i], SDL_CONTROLLER_BUTTON_BACK))  mCoin  = true;
    }
}
