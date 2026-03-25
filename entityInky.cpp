#include "entityInky.hpp"
#include "game.h"
#include "entityPacman.hpp"
#include "entityBlinky.hpp"

entityInky::entityInky(game& gameRef)
    : entityGhost(gameRef, 2)
{
    mType = ENTITY_TYPE_INKY;
}

void entityInky::getChaseTarget(int32_t& targetX, int32_t& targetY)
{
    // Inky: complex algorithm using Blinky's position
    auto* pac = mGame.getPacman();
    auto* blinky = mGame.getBlinky();

    targetX = (pac->getX() >> 3) + (mGame.xDir(pac->getDir()) << 1);
    targetY = (pac->getY() >> 3) + (mGame.yDir(pac->getDir()) << 1);

    int32_t bx = blinky->getX() >> 3;
    int32_t by = blinky->getY() >> 3;
    int32_t dx = bx - targetX;
    int32_t dy = by - targetY;
    targetX -= dx << 1;
    targetY -= dy << 1;
}

void entityInky::getScatterTarget(int32_t& targetX, int32_t& targetY)
{
    targetX = 28;
    targetY = 31;
}

void entityInky::getTrapTarget(int32_t& targetX, int32_t& targetY)
{
    targetX = 11;
    targetY = 14;
}
