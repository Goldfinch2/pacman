#pragma once

#include "entity.hpp"

class entityFruit : public entity
{
  public:
    entityFruit(game& gameRef);

    void run() override;
    void draw() override;

    void activate();
    bool isActive() const { return mActive; }

    uint32_t getScoreTimer() const { return mScoreTimer; }
    void setScoreTimer(uint32_t val) { mScoreTimer = val; }

  private:
    bool     mActive;
    uint32_t mScoreTimer;
};
