#ifndef SETTINGS_H
#define SETTINGS_H

#include "advmame_rc.h"

namespace settings
{
    void load(const char* filename = "pacman.cfg");
    void loadAdvmameRC();

    extern char serialPort[256];
    extern char sounds[256];
    extern char marquee[256];
    extern char advmamerc[256];
    extern int  dvgRefreshRate;
    extern bool mirror;

    // Input actions (loaded from advmame.rc, keyboard defaults set at startup)
    extern AdvInputAction upAction;
    extern AdvInputAction downAction;
    extern AdvInputAction leftAction;
    extern AdvInputAction rightAction;
    extern AdvInputAction startAction;
    extern AdvInputAction coinAction;
    extern AdvInputAction exitAction;
}

#endif
