#ifndef DEFINES_H
#define DEFINES_H

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <cctype>

#define MAZE_COLS 28
#define MAZE_ROWS 31
#define TILE_SIZE 8
#define FIELD_W (MAZE_COLS * TILE_SIZE)
#define FIELD_H 288
#define TOTAL_DOTS 244
#define ENERGY_LENGTH 500
#define PACMAN_MAX_X 225
#define PACMAN_MAX_Y 288

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#endif
