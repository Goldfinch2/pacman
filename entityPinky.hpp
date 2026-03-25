#pragma once

#include "entityGhost.hpp"

class entityPinky : public entityGhost
{
  public:
    entityPinky(game& gameRef);

  protected:
    void getChaseTarget(int32_t& targetX, int32_t& targetY) override;
    void getScatterTarget(int32_t& targetX, int32_t& targetY) override;
};
