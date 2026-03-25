#pragma once

#include "defines.h"

class game;

class entity
{
  public:
    enum EntityType
    {
        ENTITY_TYPE_UNDEF = 0,
        ENTITY_TYPE_PACMAN,
        ENTITY_TYPE_BLINKY,
        ENTITY_TYPE_PINKY,
        ENTITY_TYPE_INKY,
        ENTITY_TYPE_CLYDE,
        ENTITY_TYPE_FRUIT,
        ENTITY_NUM_TYPES
    };

    enum EntityState
    {
        ENTITY_STATE_INACTIVE = 0,
        ENTITY_STATE_SPAWNING,
        ENTITY_STATE_RUNNING,
        ENTITY_STATE_DYING,
        ENTITY_STATE_DESTROYED
    };

    entity(game& gameRef);
    virtual ~entity() = default;

    static entity* createEntity(EntityType type, game& gameRef);

    EntityType getType() const { return mType; }

    EntityState getState() const { return mState; }
    void setState(EntityState state) { mState = state; }

    bool getEnabled() const { return mState != ENTITY_STATE_INACTIVE; }
    void setEnabled(bool enabled);

    int32_t getX() const { return mX; }
    int32_t getY() const { return mY; }
    void setPos(int32_t x, int32_t y) { mX = x; mY = y; }

    uint32_t getDir() const { return mDir; }
    void setDir(uint32_t dir) { mDir = dir; }

    virtual void spawn();
    virtual void run();
    virtual void draw();
    virtual void destroy();

  protected:
    EntityType  mType;
    EntityState mState;
    int32_t     mX, mY;
    uint32_t    mDir;
    float       mStateTimer;
    game&       mGame;
};
