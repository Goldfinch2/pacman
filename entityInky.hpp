#pragma once

#include "entityGhost.hpp"

class entityInky : public entityGhost
{
  public:
    entityInky(game& gameRef);

  protected:
    void getChaseTarget(int32_t& targetX, int32_t& targetY) override;
    void getScatterTarget(int32_t& targetX, int32_t& targetY) override;
    void getTrapTarget(int32_t& targetX, int32_t& targetY) override;
};
