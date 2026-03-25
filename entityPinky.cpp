#include "entityPinky.hpp"
#include "game.h"
#include "entityPacman.hpp"

entityPinky::entityPinky(game& gameRef)
    : entityGhost(gameRef, 1)
{
    mType = ENTITY_TYPE_PINKY;
}

void entityPinky::getChaseTarget(int32_t& targetX, int32_t& targetY)
{
    // Pinky: target 4 tiles ahead of pacman
    auto* pac = mGame.getPacman();
    targetX = (pac->getX() >> 3) + (mGame.xDir(pac->getDir()) << 2);
    targetY = (pac->getY() >> 3) + (mGame.yDir(pac->getDir()) << 2);
}

void entityPinky::getScatterTarget(int32_t& targetX, int32_t& targetY)
{
    targetX = 3;
    targetY = 0;
}
