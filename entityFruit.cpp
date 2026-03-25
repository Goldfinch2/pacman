#include "entityFruit.hpp"
#include "game.h"

entityFruit::entityFruit(game& gameRef)
    : entity(gameRef)
    , mActive(false)
    , mScoreTimer(0)
{
    mType = ENTITY_TYPE_FRUIT;
    mX = 112;
    mY = 140;
    setState(ENTITY_STATE_RUNNING);
}

void entityFruit::activate()
{
    mActive = true;
}

void entityFruit::run()
{
    if (mScoreTimer > 0)
        mScoreTimer--;
}

void entityFruit::draw()
{
    if (mActive) {
        int32_t index = mGame.levelToFruitIndex(mGame.mLevel);
        if (index != -1)
            mGame.drawBitmap(108, 134, game::sFruitImg.bmp[game::sFruitTable[index].fruit],
                             game::sFruitImg.width, game::sFruitImg.height);
    }
    if (mScoreTimer > 0) {
        int32_t index = mGame.levelToFruitIndex(mGame.mLevel);
        if (index != -1)
            mGame.drawBitmap(106, 137, game::sFruitTable[index].score_bmp,
                             game::sFruitTable[index].width, game::sFruitTable[index].height);
    }
}
