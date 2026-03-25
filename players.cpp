#include "players.hpp"
#include "game.h"

players::players(game& gameRef)
    : mGame(gameRef)
{
    mPacman = new entityPacman(gameRef);
}

players::~players()
{
    delete mPacman;
}

void players::run()
{
    mPacman->run();
}

void players::draw()
{
    mPacman->draw();
}
