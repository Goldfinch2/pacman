#include "entityPacman.hpp"
#include "game.h"
#include "controls.hpp"
#include "renderer.h"

entityPacman::entityPacman(game& gameRef)
    : entity(gameRef)
    , mFrame(6)
    , mDead(0)
    , mEatGhost(0)
    , mGhostCounter(0)
{
    mType = ENTITY_TYPE_PACMAN;
}

void entityPacman::spawn()
{
    resetPosition();
    setState(ENTITY_STATE_RUNNING);
}

void entityPacman::resetPosition()
{
    mX = 112;
    mY = 188;
    mDir = DIR_LEFT;
    mFrame = 6;
}

void entityPacman::run()
{
    if (getState() != ENTITY_STATE_RUNNING) return;

    controls* ctrl = mGame.mControls;

    // Movement
    bool tunnel = mGame.inTunnel(mX, mY);
    int32_t a, b;
    uint8_t tmod = (mY + 4) % 8;
    bool corner = (tmod < mGame.mCornering) || (tmod > 8 - mGame.mCornering);

    if ((ctrl->dirRight() && corner) || (mDir == DIR_RIGHT)) {
        a = mGame.wrapMapY(mY >> 3);
        b = mGame.wrapMapX((mX + mGame.mBoundary) >> 3);
        if (mGame.mMap[a][b] != TILE_WALL) {
            mX++; mDir = DIR_RIGHT; mFrame++;
            // Snap Y
            a = mGame.wrapMapY((mY + 3) >> 3);
            b = mGame.wrapMapX(mX >> 3);
            if (mGame.mMap[a][b] == TILE_WALL) mY--;
            a = mGame.wrapMapY((mY - 4) >> 3);
            b = mGame.wrapMapX(mX >> 3);
            if (mGame.mMap[a][b] == TILE_WALL) mY++;
        }
    }
    if ((ctrl->dirLeft() && corner) || (mDir == DIR_LEFT)) {
        a = mGame.wrapMapY(mY >> 3);
        b = mGame.wrapMapX((mX - mGame.mBoundary) >> 3);
        if (mGame.mMap[a][b] != TILE_WALL) {
            mX--; mDir = DIR_LEFT; mFrame++;
            // Snap Y
            a = mGame.wrapMapY((mY + 3) >> 3);
            b = mGame.wrapMapX(mX >> 3);
            if (mGame.mMap[a][b] == TILE_WALL) mY--;
            a = mGame.wrapMapY((mY - 4) >> 3);
            b = mGame.wrapMapX(mX >> 3);
            if (mGame.mMap[a][b] == TILE_WALL) mY++;
        }
    }
    tmod = (mX + 4) % 8;
    corner = (tmod < mGame.mCornering) || (tmod > 8 - mGame.mCornering);
    if ((ctrl->dirDown() && corner && !tunnel) || (mDir == DIR_DOWN)) {
        a = mGame.wrapMapY((mY + mGame.mBoundary) >> 3);
        b = mGame.wrapMapX(mX >> 3);
        if (mGame.mMap[a][b] != TILE_WALL) {
            mY++; mDir = DIR_DOWN; mFrame++;
            // Snap X
            a = mGame.wrapMapY(mY >> 3);
            b = mGame.wrapMapX((mX + 3) >> 3);
            if (mGame.mMap[a][b] == TILE_WALL) mX--;
            a = mGame.wrapMapY(mY >> 3);
            b = mGame.wrapMapX((mX - 4) >> 3);
            if (mGame.mMap[a][b] == TILE_WALL) mX++;
        }
    }
    if ((ctrl->dirUp() && corner && !tunnel) || (mDir == DIR_UP)) {
        a = mGame.wrapMapY((mY - mGame.mBoundary) >> 3);
        b = mGame.wrapMapX(mX >> 3);
        if (mGame.mMap[a][b] != TILE_WALL) {
            mY--; mDir = DIR_UP; mFrame++;
            // Snap X
            a = mGame.wrapMapY(mY >> 3);
            b = mGame.wrapMapX((mX + 3) >> 3);
            if (mGame.mMap[a][b] == TILE_WALL) mX--;
            a = mGame.wrapMapY(mY >> 3);
            b = mGame.wrapMapX((mX - 4) >> 3);
            if (mGame.mMap[a][b] == TILE_WALL) mX++;
        }
    }

    mX = mGame.wrap(mX);
    mY = mGame.limit(mY);
    mFrame = mFrame % (3 * 3);
}

void entityPacman::draw()
{
    mGame.drawBitmap(mX - 6, mY - 6,
                     game::sPacmanImg.bmp[mDir][mFrame / 3],
                     game::sPacmanImg.width, game::sPacmanImg.height);
}
