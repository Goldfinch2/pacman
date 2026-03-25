#include "entityBlinky.hpp"
#include "game.h"
#include "entityPacman.hpp"

entityBlinky::entityBlinky(game& gameRef)
    : entityGhost(gameRef, 0)
{
    mType = ENTITY_TYPE_BLINKY;
}

void entityBlinky::getChaseTarget(int32_t& targetX, int32_t& targetY)
{
    // Blinky: direct chase to pacman
    targetX = mGame.getPacman()->getX() >> 3;
    targetY = mGame.getPacman()->getY() >> 3;
}

void entityBlinky::getScatterTarget(int32_t& targetX, int32_t& targetY)
{
    targetX = 25;
    targetY = 0;
}
