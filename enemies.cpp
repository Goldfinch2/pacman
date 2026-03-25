#include "enemies.hpp"
#include "game.h"

enemies::enemies(game& gameRef)
    : mGame(gameRef)
{
    mBlinky = new entityBlinky(gameRef);
    mPinky  = new entityPinky(gameRef);
    mInky   = new entityInky(gameRef);
    mClyde  = new entityClyde(gameRef);
}

enemies::~enemies()
{
    delete mBlinky;
    delete mPinky;
    delete mInky;
    delete mClyde;
}

entityGhost* enemies::getGhost(int index)
{
    switch (index) {
    case 0: return mBlinky;
    case 1: return mPinky;
    case 2: return mInky;
    case 3: return mClyde;
    }
    return nullptr;
}

void enemies::initAll()
{
    mBlinky->init(112, 88, 0, 0, 0);
    mPinky->init(112, 96, 0, 4, 0);
    mInky->init(112, 96, 0, 255, 0);
    mClyde->init(112, 96, 0, 255, 0);
}

void enemies::resetAll()
{
    mBlinky->reset(0);
    mPinky->reset(10);
    mInky->reset(20);
    mClyde->reset(30);
}

void enemies::frightenAll()
{
    mBlinky->frighten();
    mPinky->frighten();
    mInky->frighten();
    mClyde->frighten();
}

void enemies::reverseAll()
{
    mBlinky->reverseDirection();
    mPinky->reverseDirection();
    mInky->reverseDirection();
    mClyde->reverseDirection();
}

void enemies::run()
{
    mBlinky->run();
    mPinky->run();
    mInky->run();
    mClyde->run();
}

void enemies::draw()
{
    mBlinky->draw();
    mPinky->draw();
    mInky->draw();
    mClyde->draw();
}
