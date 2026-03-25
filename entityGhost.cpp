#include "entityGhost.hpp"
#include "game.h"
#include "renderer.h"

entityGhost::entityGhost(game& gameRef, uint32_t ghostIndex)
    : entity(gameRef)
    , mGhostIndex(ghostIndex)
    , mMode(MODE_NORMAL)
    , mScared(false)
    , mSpeedAccum(0)
    , mTrappedTimer(0)
{
}

void entityGhost::init(int32_t x, int32_t y, uint32_t dir, uint32_t mode, uint32_t speed)
{
    mX = x;
    mY = y;
    mDir = dir;
    mSpeedAccum = speed;
    mScared = false;

    if (mode == 0)
        mMode = MODE_NORMAL;
    else if (mode == MODE_RELEASED)
        mMode = MODE_RELEASED;
    else if (mode >= 4) {
        mMode = MODE_TRAPPED;
        mTrappedTimer = mode;
    } else {
        mMode = (GhostMode)mode;
    }

    setState(ENTITY_STATE_RUNNING);
}

void entityGhost::reset(uint32_t time)
{
    if (mMode < MODE_TRAPPED) {
        mMode = MODE_TRAPPED;
        mTrappedTimer = 4 + time;
        mX = 112;
        mY = 96;
    }
}

void entityGhost::frighten()
{
    mScared = true;
}

void entityGhost::reverseDirection()
{
    if (mDir == DIR_RIGHT) mDir = DIR_LEFT;
    else if (mDir == DIR_LEFT) mDir = DIR_RIGHT;
    else if (mDir == DIR_UP) mDir = DIR_DOWN;
    else mDir = DIR_UP;
}

void entityGhost::getTrapTarget(int32_t& targetX, int32_t& targetY)
{
    targetX = 13;
    targetY = 14;
}

void entityGhost::run()
{
    if (getState() != ENTITY_STATE_RUNNING) return;

    uint32_t targetX = 0, targetY = 0;
    bool tunnel = mGame.inTunnel(mX, mY);

    if (mGame.mEnergizerTimer == 0 && mScared)
        mScared = false;

    // Collision with pacman is handled by game class (mediates between entities)

    // Direction logic — only recalculate at grid-aligned positions
    bool skipDirection = false;
    if (mX % 8 == 0 && mY % 8 == 0 && !tunnel) {
        if (mScared) {
            targetX = rand() % 28;
            targetY = rand() % 31;
        } else if (mMode == MODE_NORMAL) {
            if (mGame.mGlobalMode == 0) {
                getScatterTarget((int32_t&)targetX, (int32_t&)targetY);
            } else {
                getChaseTarget((int32_t&)targetX, (int32_t&)targetY);
            }
        }

        if (mMode == MODE_DEAD) {
            targetX = 14; targetY = 11;
            if (mX >> 3 == 13 && mY >> 3 == 11) {
                mMode = MODE_TRAPPED;
                mTrappedTimer = 30;
                mY++;
                mDir = DIR_DOWN;
                skipDirection = true;  // original used goto to skip direction-finding here
            }
        }

        if (mMode == MODE_RELEASED) {
            targetX = 13; targetY = 11;
            if (mY >> 3 == 13 && mX >> 3 == 13) {
                mMode = MODE_NORMAL;
                mDir = DIR_UP;
                mY--;
            }
        }

        if (mMode == MODE_TRAPPED) {
            if (mTrappedTimer > 0) mTrappedTimer--;
            if (mTrappedTimer == 0) mMode = MODE_RELEASED;
            getTrapTarget((int32_t&)targetX, (int32_t&)targetY);
        }

        // Find best direction (shortest euclidean distance to target)
        if (!skipDirection) {
            uint32_t distR = 10000, distL = 10000, distU = 10000, distD = 10000, minimum = 10000;
            int32_t y, x;

            y = mGame.wrapMapY(mY >> 3);
            x = mGame.wrapMapX((mX >> 3) + 1);
            if (mDir != DIR_LEFT && mGame.mMap[y][x] != TILE_WALL) {
                distR = (mY/8 - targetY)*(mY/8 - targetY) + (mX/8 + 1 - targetX)*(mX/8 + 1 - targetX);
                if (distR < minimum) minimum = distR;
            }
            y = mGame.wrapMapY(mY >> 3);
            x = mGame.wrapMapX((mX >> 3) - 1);
            if (mDir != DIR_RIGHT && mGame.mMap[y][x] != TILE_WALL) {
                distL = (mY/8 - targetY)*(mY/8 - targetY) + (mX/8 - 1 - targetX)*(mX/8 - 1 - targetX);
                if (distL < minimum) minimum = distL;
            }
            y = mGame.wrapMapY((mY >> 3) - 1);
            x = mGame.wrapMapX(mX >> 3);
            if (mDir != DIR_DOWN && mGame.mMap[y][x] != TILE_WALL) {
                distU = (mY/8 - 1 - targetY)*(mY/8 - 1 - targetY) + (mX/8 - targetX)*(mX/8 - targetX);
                if (distU < minimum) minimum = distU;
            }
            y = mGame.wrapMapY((mY >> 3) + 1);
            x = mGame.wrapMapX(mX >> 3);
            if (mDir != DIR_UP && mGame.mMap[y][x] != TILE_WALL) {
                distD = (mY/8 + 1 - targetY)*(mY/8 + 1 - targetY) + (mX/8 - targetX)*(mX/8 - targetX);
                if (distD < minimum) minimum = distD;
            }

            if (distU == minimum) mDir = DIR_UP;
            else if (distR == minimum) mDir = DIR_RIGHT;
            else if (distD == minimum) mDir = DIR_DOWN;
            else if (distL == minimum) mDir = DIR_LEFT;
        }
    }

    // Speed control
    uint32_t doubleSpeed = 1, speed = 9;
    if (tunnel) speed = 5;
    if (mScared) speed = 6;
    if ((mX % 8) == 0 && (mY % 8) == 0) speed = 10;
    if (mMode == MODE_DEAD) { doubleSpeed = 2; speed = 10; }

    while (doubleSpeed--) {
        mSpeedAccum += speed;
        if (mSpeedAccum > 9) {
            if (mDir == DIR_RIGHT) mX++;
            if (mDir == DIR_LEFT)  mX--;
            if (mDir == DIR_UP)    mY--;
            if (mDir == DIR_DOWN)  mY++;
            mSpeedAccum -= 10;
        }
        mX = mGame.wrap(mX);
    }
}

void entityGhost::draw()
{
    uint32_t* bmp = game::sGhostImg.bmp[mGhostIndex][mDir];
    if (mScared) {
        bmp = game::sGhostScaredImg.bmp[0][mGame.mGhostFrame / 8 +
              (mGame.mEnergizerTimer < 128 && (mGame.mEnergizerTimer % 32) < 16 ? 2 : 0)];
    }
    if (mMode == MODE_DEAD) {
        bmp = game::sGhostDeadImg.bmp[0][mDir];
    }
    mGame.drawBitmap(mX - 2, mY - 2, bmp, 14, 14);
}
