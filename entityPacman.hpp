#pragma once

#include "entity.hpp"

class entityPacman : public entity
{
  public:
    entityPacman(game& gameRef);

    void spawn() override;
    void run() override;
    void draw() override;

    uint32_t getFrame() const { return mFrame; }
    int32_t  getDead() const { return mDead; }
    void     setDead(int32_t dead) { mDead = dead; }
    uint32_t getEatGhost() const { return mEatGhost; }
    void     setEatGhost(uint32_t val) { mEatGhost = val; }
    uint32_t getGhostCounter() const { return mGhostCounter; }
    void     setGhostCounter(uint32_t val) { mGhostCounter = val; }

    void resetPosition();

  private:
    uint32_t mFrame;
    int32_t  mDead;
    uint32_t mEatGhost;
    uint32_t mGhostCounter;
    uint32_t mAiDesiredDir;   // attract-mode AI: desired next direction

    void updateAiDir();       // greedy dot-seeking AI
};
