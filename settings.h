#ifndef SETTINGS_H
#define SETTINGS_H

namespace settings
{
    void load(const char* filename = "pacman.cfg");

    extern char serialPort[256];
    extern char sounds[256];
    extern char marquee[256];
    extern bool sortVectors;
    extern int  dvgRefreshRate;
    extern bool mirror;
}

#endif
