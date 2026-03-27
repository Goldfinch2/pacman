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
    , mAiDesiredDir(DIR_RIGHT)
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
    mDir = DIR_NONE;
    mFrame = 0;
    mAiDesiredDir = DIR_RIGHT;
}

// BFS pathfinder: find the nearest target tile and return the first-step direction.
// Prioritises energizers; falls back to any food.
void entityPacman::updateAiDir()
{
    // Only recalculate at tile centers
    if ((mX % 8) != 0 || (mY % 8) != 0) return;

    const int COLS = MAZE_COLS;
    const int ROWS = MAZE_ROWS;

    const int startCol = mX >> 3;
    const int startRow = mY >> 3;

    // Reverse of current direction — used to deprioritise going backward
    int rev = DIR_NONE;
    if      (mDir == DIR_RIGHT) rev = DIR_LEFT;
    else if (mDir == DIR_LEFT)  rev = DIR_RIGHT;
    else if (mDir == DIR_UP)    rev = DIR_DOWN;
    else if (mDir == DIR_DOWN)  rev = DIR_UP;

    const int dcol[4] = { 1, -1,  0,  0 };
    const int drow[4] = { 0,  0, -1,  1 };
    const int ddirs[4] = { DIR_RIGHT, DIR_LEFT, DIR_UP, DIR_DOWN };

    // BFS state
    // visited[row][col] = first-step direction taken from start to reach this tile
    static int8_t visited[MAZE_ROWS][MAZE_COLS];
    memset(visited, -1, sizeof(visited));

    // Queue: encode as (col | row<<5 | firstDir<<10)
    static int queue[MAZE_ROWS * MAZE_COLS];
    int head = 0, tail = 0;

    // Seed neighbours of start (skip reverse to prefer forward momentum)
    for (int di = 0; di < 4; di++) {
        int d = ddirs[di];
        int nc = mGame.wrapMapX(startCol + dcol[di]);
        int nr = mGame.wrapMapY(startRow + drow[di]);
        if (mGame.mMap[nr][nc] == TILE_WALL) continue;
        if (visited[nr][nc] != -1) continue;
        // Push reverse last so forward directions are explored first
        if (d == rev) continue;
        visited[nr][nc] = (int8_t)d;
        queue[tail++] = nc | (nr << 5) | (d << 10);
    }
    // Now push reverse direction (lowest priority)
    for (int di = 0; di < 4; di++) {
        int d = ddirs[di];
        if (d != rev) continue;
        int nc = mGame.wrapMapX(startCol + dcol[di]);
        int nr = mGame.wrapMapY(startRow + drow[di]);
        if (mGame.mMap[nr][nc] == TILE_WALL) continue;
        if (visited[nr][nc] != -1) continue;
        visited[nr][nc] = (int8_t)d;
        queue[tail++] = nc | (nr << 5) | (d << 10);
    }

    int foundDir = -1;
    bool foundEnergizer = false;

    while (head < tail) {
        int item  = queue[head++];
        int col   = item & 0x1f;
        int row   = (item >> 5) & 0x1f;
        int first = (item >> 10) & 0xf;

        uint8_t tile = mGame.mMap[row][col];

        if (tile == TILE_ENERGIZER) {
            foundDir      = first;
            foundEnergizer = true;
            break;  // energizer found — stop immediately
        }
        if (tile == TILE_FOOD && !foundEnergizer && foundDir == -1)
            foundDir = first;  // record first food hit, keep searching for energizer

        // Expand neighbours
        for (int di = 0; di < 4; di++) {
            int nc = mGame.wrapMapX(col + dcol[di]);
            int nr = mGame.wrapMapY(row + drow[di]);
            if (mGame.mMap[nr][nc] == TILE_WALL) continue;
            if (visited[nr][nc] != -1) continue;
            visited[nr][nc] = (int8_t)first;
            queue[tail++] = nc | (nr << 5) | (first << 10);
        }
    }

    if (foundDir != -1)
        mAiDesiredDir = (uint32_t)foundDir;
}

void entityPacman::run()
{
    if (getState() != ENTITY_STATE_RUNNING) return;

    bool inAttract = (game::mGameMode == game::GAMEMODE_ATTRACT);

    if (inAttract)
        updateAiDir();

    controls* ctrl = mGame.mControls;

    // Input: AI in attract mode, player controls otherwise
    bool wantRight = inAttract ? (mAiDesiredDir == DIR_RIGHT) : ctrl->dirRight();
    bool wantLeft  = inAttract ? (mAiDesiredDir == DIR_LEFT)  : ctrl->dirLeft();
    bool wantUp    = inAttract ? (mAiDesiredDir == DIR_UP)    : ctrl->dirUp();
    bool wantDown  = inAttract ? (mAiDesiredDir == DIR_DOWN)  : ctrl->dirDown();

    // Movement
    bool tunnel = mGame.inTunnel(mX, mY);
    int32_t a, b;
    uint8_t tmod = (mY + 4) % 8;
    bool corner = (tmod < mGame.mCornering) || (tmod > 8 - mGame.mCornering);

    if ((wantRight && corner) || (mDir == DIR_RIGHT)) {
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
    if ((wantLeft && corner) || (mDir == DIR_LEFT)) {
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
    if ((wantDown && corner && !tunnel) || (mDir == DIR_DOWN)) {
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
    if ((wantUp && corner && !tunnel) || (mDir == DIR_UP)) {
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
    uint32_t drawDir = (mDir == DIR_NONE) ? DIR_RIGHT : mDir;
    mGame.drawBitmap(mX - 6, mY - 6,
                     game::sPacmanImg.bmp[drawDir][mFrame / 3],
                     game::sPacmanImg.width, game::sPacmanImg.height);
}
