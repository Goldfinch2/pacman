#include "entity.hpp"
#include "entityPacman.hpp"
#include "entityBlinky.hpp"
#include "entityPinky.hpp"
#include "entityInky.hpp"
#include "entityClyde.hpp"
#include "entityFruit.hpp"

entity::entity(game& gameRef)
    : mType(ENTITY_TYPE_UNDEF)
    , mState(ENTITY_STATE_INACTIVE)
    , mX(0), mY(0)
    , mDir(0)
    , mStateTimer(0)
    , mGame(gameRef)
{
}

entity* entity::createEntity(EntityType type, game& gameRef)
{
    switch (type) {
    case ENTITY_TYPE_PACMAN: return new entityPacman(gameRef);
    case ENTITY_TYPE_BLINKY: return new entityBlinky(gameRef);
    case ENTITY_TYPE_PINKY:  return new entityPinky(gameRef);
    case ENTITY_TYPE_INKY:   return new entityInky(gameRef);
    case ENTITY_TYPE_CLYDE:  return new entityClyde(gameRef);
    case ENTITY_TYPE_FRUIT:  return new entityFruit(gameRef);
    default: return new entity(gameRef);
    }
}

void entity::setEnabled(bool enabled)
{
    mState = enabled ? ENTITY_STATE_SPAWNING : ENTITY_STATE_INACTIVE;
}

void entity::spawn() { setState(ENTITY_STATE_RUNNING); }
void entity::run() {}
void entity::draw() {}
void entity::destroy() { setState(ENTITY_STATE_INACTIVE); }
