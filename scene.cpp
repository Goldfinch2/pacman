#include "scene.hpp"
#include "game.h"

scene g_scene;

scene::scene()
{
}

scene::~scene()
{
    shutdown();
}

void scene::init()
{
    mGame = std::make_unique<game>();
    mGame->init();
}

void scene::shutdown()
{
    mGame.reset();
}

void scene::run()
{
    if (mGame)
        mGame->run();
}

void scene::draw()
{
    if (mGame)
        mGame->draw();
}
