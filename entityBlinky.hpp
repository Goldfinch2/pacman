#pragma once

#include "entityGhost.hpp"

class entityBlinky : public entityGhost
{
  public:
    entityBlinky(game& gameRef);

  protected:
    void getChaseTarget(int32_t& targetX, int32_t& targetY) override;
    void getScatterTarget(int32_t& targetX, int32_t& targetY) override;
};
