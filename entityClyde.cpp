#include "entityClyde.hpp"
#include "game.h"
#include "entityPacman.hpp"

entityClyde::entityClyde(game& gameRef)
    : entityGhost(gameRef, 3)
{
    mType = ENTITY_TYPE_CLYDE;
}

void entityClyde::getChaseTarget(int32_t& targetX, int32_t& targetY)
{
    // Clyde: chase if >8 tiles away, else scatter
    auto* pac = mGame.getPacman();
    targetX = pac->getX() >> 3;
    targetY = pac->getY() >> 3;

    uint32_t distance = (mY/8 - targetY)*(mY/8 - targetY) +
                        (mX/8 - 1 - targetX)*(mX/8 - 1 - targetX);
    if (distance < 64) {
        getScatterTarget(targetX, targetY);
    }
}

void entityClyde::getScatterTarget(int32_t& targetX, int32_t& targetY)
{
    targetX = 0;
    targetY = 31;
}

void entityClyde::getTrapTarget(int32_t& targetX, int32_t& targetY)
{
    targetX = 15;
    targetY = 14;
}
