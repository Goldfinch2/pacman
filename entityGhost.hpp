#pragma once

#include "entity.hpp"

class entityGhost : public entity
{
  public:
    enum GhostMode { MODE_NORMAL, MODE_DEAD, MODE_RELEASED, MODE_TRAPPED };

    entityGhost(game& gameRef, uint32_t ghostIndex);

    void run() override;
    void draw() override;

    uint32_t getGhostIndex() const { return mGhostIndex; }

    void init(int32_t x, int32_t y, uint32_t dir, uint32_t mode, uint32_t speed);
    void reset(uint32_t time);
    void frighten();
    void reverseDirection();

    GhostMode getMode() const { return mMode; }
    void setMode(GhostMode mode) { mMode = mode; }
    bool isScared() const { return mScared; }
    void setScared(bool scared) { mScared = scared; }
    uint32_t getSpeedAccum() const { return mSpeedAccum; }

  protected:
    // Subclasses override these to define personality
    virtual void getChaseTarget(int32_t& targetX, int32_t& targetY) = 0;
    virtual void getScatterTarget(int32_t& targetX, int32_t& targetY) = 0;
    virtual void getTrapTarget(int32_t& targetX, int32_t& targetY);

    uint32_t  mGhostIndex;
    GhostMode mMode;
    bool      mScared;
    uint32_t  mSpeedAccum;
    uint32_t  mTrappedTimer; // countdown for trapped ghosts (replaces mode >= GHOST_TRAPPED)
};
