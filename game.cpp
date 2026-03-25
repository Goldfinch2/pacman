#include "game.h"
#include "controls.hpp"
#include "players.hpp"
#include "enemies.hpp"
#include "entityPacman.hpp"
#include "entityGhost.hpp"
#include "entityBlinky.hpp"
#include "entityPinky.hpp"
#include "entityInky.hpp"
#include "entityClyde.hpp"
#include "entityFruit.hpp"
#include "renderer.h"
#include "pacdefs.h"
#include "pac_char_set.h"
#include "SDL.h"

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

// Static game mode
game::GameMode game::mGameMode = game::GAMEMODE_ATTRACT;

// char_set is local only — not needed by entities
typedef struct {
    uint32_t width, height;
    uint32_t *bmp[128];
} char_set_img_t;

// ============================================================
// Fruit table
// ============================================================

FruitInfo game::sFruitTable[] = {
    {1, 1,    CHERRY,     100, 16, 7, s_cnt_100_img},
    {2, 2,    STRAWBERRY, 300, 16, 7, s_cnt_300_img},
    {3, 4,    PEAR,       500, 16, 7, s_cnt_500_img},
    {5, 6,    APPLE,      700, 16, 7, s_cnt_700_img},
    {7, 8,    JACK,       1000, 20, 7, s_cnt_1000_img},
    {9, 10,   EAGLE,      2000, 20, 7, s_cnt_2000_img},
    {11, 12,  BELL,       3000, 20, 7, s_cnt_3000_img},
    {13, 255, KEY,        5000, 20, 7, s_cnt_5000_img}
};

// ============================================================
// Image tables
// ============================================================

fruit_img_t game::sFruitImg = {
    12, 12,
    { s_cherry_img, s_straw_img, s_pear_img, s_apple_img,
      s_jack_img, s_eagle_img, s_bell_img, s_key_img }
};

pacman_img_t game::sPacmanImg = {
    14, 14,
    {
        { s_pac_right1_img, s_pac_right2_img, s_pac_right3_img },
        { s_pac_left1_img,  s_pac_left2_img,  s_pac_left3_img },
        { s_pac_up1_img,    s_pac_up2_img,    s_pac_up3_img },
        { s_pac_down1_img,  s_pac_down2_img,  s_pac_down3_img }
    }
};

ghost_img_t game::sGhostImg = {
    14, 14,
    {
        { s_blinky_right_img, s_blinky_left_img, s_blinky_up_img, s_blinky_down_img },
        { s_pinky_right_img,  s_pinky_left_img,  s_pinky_up_img,  s_pinky_down_img },
        { s_inky_right_img,   s_inky_left_img,   s_inky_up_img,   s_inky_down_img },
        { s_clyde_right_img,  s_clyde_left_img,  s_clyde_up_img,  s_clyde_down_img }
    }
};

ghost_img_t game::sGhostDeadImg = {
    14, 14,
    {
        { s_ghost_dead_right_img, s_ghost_dead_left_img, s_ghost_dead_up_img, s_ghost_dead_down_img },
        {}, {}, {}
    }
};

ghost_img_t game::sGhostScaredImg = {
    14, 14,
    {
        { s_ghost_scared1_img, s_ghost_scared1_img, s_ghost_scared2_img, s_ghost_scared2_img },
        {}, {}, {}
    }
};

pacdead_img_t game::sPacDeadImg = {
    14, 14,
    {
        s_pac_right3_img,
        s_pac_dead1_img, s_pac_dead2_img, s_pac_dead3_img,
        s_pac_dead4_img, s_pac_dead5_img, s_pac_dead6_img,
        s_pac_dead7_img, s_pac_dead8_img, s_pac_dead9_img,
        s_pac_dead10_img, s_pac_dead11_img, s_pac_dead12_img
    }
};

ghost_counter_img_t game::sGhostCounterImg = {
    16, 7,
    { s_ghost_counter1_img, s_ghost_counter2_img, s_ghost_counter3_img, s_ghost_counter4_img }
};

static char_set_img_t s_char_set_img = {
    8, 8,
    {
        // ASCII 0-15: all blank
        s_char_set_47, s_char_set_47, s_char_set_47, s_char_set_47,
        s_char_set_47, s_char_set_47, s_char_set_47, s_char_set_47,
        s_char_set_47, s_char_set_47, s_char_set_47, s_char_set_47,
        s_char_set_47, s_char_set_47, s_char_set_47, s_char_set_47,
        // ASCII 16-31: all blank
        s_char_set_47, s_char_set_47, s_char_set_47, s_char_set_47,
        s_char_set_47, s_char_set_47, s_char_set_47, s_char_set_47,
        s_char_set_47, s_char_set_47, s_char_set_47, s_char_set_47,
        s_char_set_47, s_char_set_47, s_char_set_47, s_char_set_47,
        // ASCII 32-47: space ! " # $ % & ' ( ) * + , - . /
        s_char_set_47, s_char_set_13, s_char_set_43, s_char_set_47,
        s_char_set_47, s_char_set_47, s_char_set_47, s_char_set_11,
        s_char_set_47, s_char_set_47, s_char_set_47, s_char_set_47,
        s_char_set_47, s_char_set_44, s_char_set_47, s_char_set_47,
        // ASCII 48-63: 0-9 : ; < = > ?
        s_char_set_0, s_char_set_1, s_char_set_2, s_char_set_3,
        s_char_set_4, s_char_set_5, s_char_set_6, s_char_set_7,
        s_char_set_8, s_char_set_9, s_char_set_10, s_char_set_11,
        s_char_set_12, s_char_set_13, s_char_set_14, s_char_set_15,
        // ASCII 64-79: @ A-O
        s_char_set_16, s_char_set_17, s_char_set_18, s_char_set_19,
        s_char_set_20, s_char_set_21, s_char_set_22, s_char_set_23,
        s_char_set_24, s_char_set_25, s_char_set_26, s_char_set_27,
        s_char_set_28, s_char_set_29, s_char_set_30, s_char_set_31,
        // ASCII 80-95: P-Z [ \ ] ^ _
        s_char_set_32, s_char_set_33, s_char_set_34, s_char_set_35,
        s_char_set_36, s_char_set_37, s_char_set_38, s_char_set_39,
        s_char_set_40, s_char_set_41, s_char_set_42, s_char_set_43,
        s_char_set_44, s_char_set_45, s_char_set_46, s_char_set_47,
        // ASCII 96-111: blank
        s_char_set_47, s_char_set_47, s_char_set_47, s_char_set_47,
        s_char_set_47, s_char_set_47, s_char_set_47, s_char_set_47,
        s_char_set_47, s_char_set_47, s_char_set_47, s_char_set_47,
        s_char_set_47, s_char_set_47, s_char_set_47, s_char_set_47,
        // ASCII 112-127: blank
        s_char_set_47, s_char_set_47, s_char_set_47, s_char_set_47,
        s_char_set_47, s_char_set_47, s_char_set_47, s_char_set_47,
        s_char_set_47, s_char_set_47, s_char_set_47, s_char_set_47,
        s_char_set_47, s_char_set_47, s_char_set_47, s_char_set_47
    }
};

// ============================================================
// Original maze tile map
// ============================================================

static uint8_t s_org_map[31][28] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1},
    {1,3,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,3,1},
    {1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,2,1},
    {1,2,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,2,1},
    {1,2,2,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,2,2,1},
    {1,1,1,1,1,1,2,1,1,1,1,1,0,1,1,0,1,1,1,1,1,2,1,1,1,1,1,1},
    {1,1,1,1,1,1,2,1,1,1,1,1,0,1,1,0,1,1,1,1,1,2,1,1,1,1,1,1},
    {0,0,0,0,1,1,2,1,1,0,0,0,0,0,0,0,0,0,0,1,1,2,1,1,0,0,0,0},
    {1,1,1,1,1,1,2,1,1,0,1,1,1,1,1,1,1,1,0,1,1,2,1,1,1,1,1,1},
    {1,1,1,1,1,1,2,1,1,0,1,1,0,0,0,0,1,1,0,1,1,2,1,1,1,1,1,1},
    {0,0,0,0,0,0,2,0,0,0,1,1,0,0,0,0,1,1,0,0,0,2,0,0,0,0,0,0},
    {1,1,1,1,1,1,2,1,1,0,1,1,1,1,1,1,1,1,0,1,1,2,1,1,1,1,1,1},
    {1,0,0,0,0,1,2,1,1,0,1,1,1,1,1,1,1,1,0,1,1,2,1,0,0,0,0,1},
    {0,0,0,0,0,1,2,1,1,0,0,0,0,0,0,0,0,0,0,1,1,2,1,0,0,0,0,0},
    {0,0,0,0,0,1,2,1,1,0,1,1,1,1,1,1,1,1,0,1,1,2,1,0,0,0,0,0},
    {1,1,1,1,1,1,2,1,1,0,1,1,1,1,1,1,1,1,0,1,1,2,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1},
    {1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1},
    {1,3,2,2,1,1,2,2,2,2,2,2,2,0,0,2,2,2,2,2,2,2,1,1,2,2,3,1},
    {1,1,1,2,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,1,2,1,1,2,1,1,1},
    {1,1,1,2,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,1,2,1,1,2,1,1,1},
    {1,2,2,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1},
    {1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// ============================================================
// Maze background polygons (vector outlines)
// ============================================================

static uint8_t s_background[] = {
24,
34,
224,104, 185,104, 184,103, 184,80, 185,79, 220,79, 221,78, 222,78, 223,77, 223,76, 224,75, 224,4, 223,3, 223,2, 222,1, 221,1, 220,0, 5,0, 4,1, 3,1, 2,2, 2,3, 1,4, 1,75, 2,76, 2,77, 3,78, 4,78, 5,79, 40,79, 41,80, 41,103, 40,104, 1,104,
34,
1,127, 40,127, 41,128, 41,151, 40,152, 5,152, 4,153, 3,153, 2,154, 2,155, 1,156, 1,243, 2,244, 2,245, 3,246, 4,246, 5,247, 220,247, 221,246, 222,246, 223,245, 223,244, 224,243, 224,156, 223,155, 223,154, 222,153, 221,153, 220,152, 185,152, 184,151, 184,128, 185,127, 224,127,
26,
224,107, 183,107, 181,105, 181,78, 183,76, 219,76, 221,74, 221,5, 219,3, 118,3, 116,5, 116,33, 114,35, 111,35, 109,33, 109,5, 107,3, 6,3, 4,5, 4,74, 6,76, 42,76, 44,78, 44,105, 42,107, 1,107,
34,
1,124, 42,124, 44,126, 44,153, 42,155, 6,155, 4,157, 4,194, 6,196, 18,196, 20,198, 20,201, 18,203, 6,203, 4,205, 4,242, 6,244, 219,244, 221,242, 221,205, 219,203, 207,203, 205,201, 205,198, 207,196, 219,196, 221,194, 221,157, 219,155, 183,155, 181,153, 181,126, 183,124, 224,124,
9,
164,126, 164,153, 162,155, 159,155, 157,153, 157,126, 159,124, 162,124, 164,126,
17,
164,54, 164,105, 162,107, 159,107, 157,105, 157,85, 155,83, 135,83, 133,81, 133,78, 135,76, 155,76, 157,74, 157,54, 159,52, 162,52, 164,54,
9,
181,57, 183,59, 202,59, 204,57, 204,54, 202,52, 183,52, 181,54, 181,57,
9,
181,33, 183,35, 202,35, 204,33, 204,22, 202,20, 183,20, 181,22, 181,33,
9,
135,35, 162,35, 164,33, 164,22, 162,20, 135,20, 133,22, 133,33, 135,35,
17,
87,52, 138,52, 140,54, 140,57, 138,59, 118,59, 116,61, 116,81, 114,83, 111,83, 109,81, 109,61, 107,59, 87,59, 85,57, 85,54, 87,52,
9,
63,35, 90,35, 92,33, 92,22, 90,20, 63,20, 61,22, 61,33, 63,35,
9,
42,35, 23,35, 21,33, 21,22, 23,20, 42,20, 44,22, 44,33, 42,35,
9,
42,52, 23,52, 21,54, 21,57, 23,59, 42,59, 44,57, 44,54, 42,52,
17,
70,76, 68,74, 68,54, 66,52, 63,52, 61,54, 61,105, 63,107, 66,107, 68,105, 68,85, 70,83, 90,83, 92,81, 92,78, 90,76, 70,76,
17,
85,100, 104,100, 105,101, 120,101, 121,103, 137,103, 137,128, 88,128, 88,103, 104,103, 104,102, 120,102, 121,100, 140,100, 140,131, 85,131, 85,100,
9,
68,126, 66,124, 63,124, 61,126, 61,153, 63,155, 66,155, 68,153, 68,126,
9,
63,172, 90,172, 92,174, 92,177, 90,179, 63,179, 61,177, 61,174, 63,172,
13,
44,174, 42,172, 23,172, 21,174, 21,177, 23,179, 35,179, 37,181, 37,201, 39,203, 42,203, 44,201, 44,174,
17,
61,198, 61,218, 59,220, 23,220, 21,222, 21,225, 23,227, 90,227, 92,225, 92,222, 90,220, 70,220, 68,218, 68,198, 66,196, 63,196, 61,198,
17,
87,196, 85,198, 85,201, 87,203, 107,203, 109,205, 109,225, 111,227, 114,227, 116,225, 116,205, 118,203, 138,203, 140,201, 140,198, 138,196, 87,196,
17,
116,177, 116,157, 118,155, 138,155, 140,153, 140,150, 138,148, 87,148, 85,150, 85,153, 87,155, 107,155, 109,157, 109,177, 111,179, 114,179, 116,177,
9,
135,179, 133,177, 133,174, 135,172, 162,172, 164,174, 164,177, 162,179, 135,179,
17,
157,198, 157,218, 155,220, 135,220, 133,222, 133,225, 135,227, 202,227, 204,225, 204,222, 202,220, 166,220, 164,218, 164,198, 162,196, 159,196, 157,198,
13,
181,174, 181,201, 183,203, 186,203, 188,201, 188,181, 190,179, 202,179, 204,177, 204,174, 202,172, 183,172, 181,174
};

// Energizer bitmap (9x9)
static uint32_t s_energizer[] = {
0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
0x000000, 0x000000, 0x000000, 0xffb897, 0xffb897, 0xffb897, 0x000000, 0x000000, 0x000000,
0x000000, 0x000000, 0xffb897, 0xffb897, 0xffb897, 0xffb897, 0xffb897, 0x000000, 0x000000,
0x000000, 0xffb897, 0xffb897, 0xffb897, 0xffb897, 0xffb897, 0xffb897, 0xffb897, 0x000000,
0x000000, 0xffb897, 0xffb897, 0xffb897, 0xffb897, 0xffb897, 0xffb897, 0xffb897, 0x000000,
0x000000, 0xffb897, 0xffb897, 0xffb897, 0xffb897, 0xffb897, 0xffb897, 0xffb897, 0x000000,
0x000000, 0x000000, 0xffb897, 0xffb897, 0xffb897, 0xffb897, 0xffb897, 0x000000, 0x000000,
0x000000, 0x000000, 0x000000, 0xffb897, 0xffb897, 0xffb897, 0x000000, 0x000000, 0x000000,
0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
};

// ============================================================
// Constructor / Destructor
// ============================================================

game::game()
    : mControls(nullptr)
    , mPlayers(nullptr)
    , mEnemies(nullptr)
    , mFruit(nullptr)
{
    mGameMode = GAMEMODE_ATTRACT;
    mScore = 0;
    mHighScore = 0;
    mLives = 0;
    mLevel = 1;
    mStateTimer = 0;
    mFrameCounter = 0;
    mGlobalMode = 1;
    mEnergizerTimer = 0;
    mFoodEaten = 0;
    mGhostFrame = 0;
    mBoundary = 5;
    mCornering = 2;
}

game::~game()
{
    delete mFruit;
    delete mEnemies;
    delete mPlayers;
    delete mControls;
}

void game::init()
{
    // Create subsystems (opengw pattern: game owns managers)
    mControls = new controls();
    mPlayers  = new players(*this);
    mEnemies  = new enemies(*this);
    mFruit    = new entityFruit(*this);

    // Load sounds
    mSound.loadTrack("sounds/start.wav",      SND_START,      0.5f, false);
    mSound.loadTrack("sounds/chomp.wav",       SND_CHOMP,      0.3f, false);
    mSound.loadTrack("sounds/eat_fruit.wav",   SND_EAT_FRUIT,  0.5f, false);
    mSound.loadTrack("sounds/eat_ghost.wav",   SND_EAT_GHOST,  0.5f, false);
    mSound.loadTrack("sounds/death.wav",       SND_DEATH,      0.5f, false);
    mSound.loadTrack("sounds/siren.wav",       SND_SIREN,      0.3f, true);
    mSound.loadTrack("sounds/frightened.wav",  SND_FRIGHTENED,  0.3f, true);
    mSound.loadTrack("sounds/ghost_eyes.wav",  SND_GHOST_EYES,  0.3f, true);
    mSound.loadTrack("sounds/extra_life.wav",  SND_EXTRA_LIFE,  0.5f, false);
    mSound.startSound();
}

// ============================================================
// Entity accessors
// ============================================================

entityPacman* game::getPacman()
{
    return mPlayers->getPacman();
}

entityBlinky* game::getBlinky()
{
    return mEnemies->getBlinky();
}

entityGhost* game::getGhost(int index)
{
    return mEnemies->getGhost(index);
}

// ============================================================
// Helper functions
// ============================================================

int32_t game::wrapMapX(int32_t x) {
    if (x < 0) x = 27;
    else if (x > 27) x = 0;
    return x;
}

int32_t game::wrapMapY(int32_t y) {
    if (y < 0) y = 30;
    else if (y > 30) y = 0;
    return y;
}

int32_t game::wrap(int32_t x) {
    if (x < 0) x = PACMAN_MAX_X;
    else if (x > PACMAN_MAX_X) x = 0;
    return x;
}

int32_t game::limit(int32_t y) {
    if (y < 12) y = 12;
    return y;
}

bool game::inTunnel(uint32_t x, uint32_t y) {
    return ((y < 126) && (y > 105) && (x < 44 || x > 180));
}

int32_t game::xDir(uint32_t a) {
    if (a == DIR_RIGHT) return 1;
    if (a == DIR_LEFT) return -1;
    return 0;
}

int32_t game::yDir(uint32_t a) {
    if (a == DIR_DOWN) return 1;
    if (a == DIR_UP) return -1;
    return 0;
}

int32_t game::levelToFruitIndex(uint32_t level) {
    for (uint32_t i = 0; i < ARRAY_SIZE(sFruitTable); i++) {
        if (level >= sFruitTable[i].min_level && level <= sFruitTable[i].max_level)
            return (int32_t)i;
    }
    return -1;
}

// ============================================================
// Food logic
// ============================================================

void game::eatFood() {
    mFoodEaten++;
    if (mFoodEaten == 70 || mFoodEaten == 170)
        mFruit->activate();
    if (mFoodEaten == 30)
        mEnemies->getInky()->setMode(entityGhost::MODE_RELEASED);
    if (mFoodEaten == 82)
        mEnemies->getClyde()->setMode(entityGhost::MODE_RELEASED);
}

// ============================================================
// Collision detection (game mediates between entities)
// ============================================================

void game::checkCollisions()
{
    entityPacman* pac = getPacman();

    for (int i = 0; i < enemies::NUM_GHOSTS; i++) {
        entityGhost* ghost = getGhost(i);
        if (!ghost->getEnabled()) continue;

        if (((pac->getX() >> 3) == (ghost->getX() + 4) >> 3) &&
            ((pac->getY() >> 3) == (ghost->getY() + 4) >> 3))
        {
            if (ghost->isScared()) {
                ghost->setMode(entityGhost::MODE_DEAD);
                ghost->setScared(false);
                ghost->setPos(ghost->getX() & 0xfffffffe, ghost->getY() & 0xfffffffe);
                uint32_t cnt = pac->getGhostCounter() + 1;
                if (cnt > 4) cnt = 1;
                pac->setGhostCounter(cnt);
                mScore += 100 << cnt;
                pac->setEatGhost(40);
                mSound.playTrack(SND_EAT_GHOST);
                mSound.stopTrack(SND_FRIGHTENED);
                mSound.playTrack(SND_GHOST_EYES);
            } else if (ghost->getMode() != entityGhost::MODE_DEAD) {
                pac->setDead(75);
                mStateTimer = 0;
                mEnemies->resetAll();
                mSound.stopAllTracks();
            }
        }
    }
}

// ============================================================
// Level init
// ============================================================

void game::initLevel(bool fullReset) {
    mEnemies->initAll();
    getPacman()->resetPosition();
    getPacman()->setDead(0);
    getPacman()->setEatGhost(0);
    getPacman()->setGhostCounter(0);
    getPacman()->setState(entity::ENTITY_STATE_RUNNING);

    mBoundary = 5;
    mCornering = 2;
    mFoodEaten = 0;
    mGlobalMode = 1; // chase
    mEnergizerTimer = 0;

    delete mFruit;
    mFruit = new entityFruit(*this);

    if (fullReset) {
        mScore = 0;
        mLives = 4;
        mLevel = 1;
    }
    mGhostFrame = 0;
    memcpy(mMap, s_org_map, sizeof(mMap));
}

// ============================================================
// Main frame logic
// ============================================================

void game::runFrame() {
    entityPacman* pac = getPacman();

    // Death sequence
    if (pac->getDead() != 0) {
        mStateTimer++;

        if (mStateTimer == 20)
            mSound.playTrack(SND_DEATH);

        if (mStateTimer <= 30)
            return;

        if (pac->getDead() > 1) {
            pac->setDead(pac->getDead() - 1);
            return;
        }

        if (mSound.isTrackPlaying(SND_DEATH))
            return;

        if (pac->getDead() == 1) {
            pac->setDead(-60);
            return;
        }
        if (pac->getDead() < -1) {
            pac->setDead(pac->getDead() + 1);
            return;
        }

        // Done — transition
        pac->setDead(0);
        if (mLives) mLives--;
        if (mLives == 0) {
            mGameMode = GAMEMODE_GAMEOVER;
            mStateTimer = 0;
        } else {
            pac->resetPosition();
            mEnergizerTimer = 0;
            mEnemies->initAll();
            mGameMode = GAMEMODE_READY;
            mStateTimer = 0;
        }
        return;
    }

    // Eating ghost pause
    if (pac->getEatGhost() > 0) {
        pac->setEatGhost(pac->getEatGhost() - 1);
        return;
    }

    // Check food/energizer at pacman position
    uint8_t temp = mMap[pac->getY() >> 3][pac->getX() >> 3];
    if (temp == TILE_FOOD) {
        mMap[pac->getY() >> 3][pac->getX() >> 3] = TILE_EMPTY;
        mScore += 10;
        mSound.playTrack(SND_CHOMP);
        eatFood();
    } else if (temp == TILE_ENERGIZER) {
        mMap[pac->getY() >> 3][pac->getX() >> 3] = TILE_EMPTY;
        mScore += 50;
        mEnemies->frightenAll();
        mEnemies->reverseAll();
        mEnergizerTimer = ENERGY_LENGTH;
        mSound.stopTrack(SND_SIREN);
        mSound.playTrack(SND_FRIGHTENED);
        eatFood();
    } else if (mFruit->isActive() && pac->getX() == 112 && pac->getY() == 140) {
        int32_t index = levelToFruitIndex(mLevel);
        if (index != -1) mScore += sFruitTable[index].score;
        // Deactivate fruit, show score
        delete mFruit;
        mFruit = new entityFruit(*this);
        mFruit->setScoreTimer(75);
        mSound.playTrack(SND_EAT_FRUIT);
    } else {
        // Movement handled by pacman entity
        mPlayers->run();
    }

    // Energizer timer
    if (!mEnergizerTimer) {
        pac->setGhostCounter(0);
    } else {
        mEnergizerTimer--;
        if (mEnergizerTimer == 0) {
            mSound.stopTrack(SND_FRIGHTENED);
            mSound.stopTrack(SND_GHOST_EYES);
            mSound.playTrack(SND_SIREN);
        }
    }

    // Process ghosts via enemy manager
    mEnemies->run();

    // Collision detection (game mediates)
    checkCollisions();

    // Update high score
    if (mScore > mHighScore) mHighScore = mScore;

    // Level complete check
    if (mFoodEaten >= TOTAL_DOTS) {
        mGameMode = GAMEMODE_LEVEL_COMPLETE;
        mStateTimer = 0;
    }

    mGhostFrame = (mGhostFrame + 1) % (8 * 2);

    // Fruit timer
    mFruit->run();
}

// ============================================================
// Game state machine
// ============================================================

void game::run() {
    mControls->update();
    mFrameCounter++;

    switch (mGameMode) {
        case GAMEMODE_ATTRACT:
            if (mControls->startButton()) {
                initLevel(true);
                mSound.stopAllTracks();
                mSound.playTrack(SND_START);
                mGameMode = GAMEMODE_READY;
                mStateTimer = 0;
            }
            break;

        case GAMEMODE_READY:
            mStateTimer++;
            if (mSound.isTrackPlaying(SND_START)) {
                mStateTimer = 0;
            } else if (mStateTimer > 60) {
                mGameMode = GAMEMODE_PLAYING;
                mSound.playTrack(SND_SIREN);
            }
            break;

        case GAMEMODE_PLAYING:
            runFrame();
            break;

        case GAMEMODE_DEAD:
            runFrame();
            break;

        case GAMEMODE_LEVEL_COMPLETE:
            mStateTimer++;
            if (mStateTimer == 1)
                mSound.stopAllTracks();
            if (mStateTimer > 180) {
                mLevel++;
                initLevel(false);
                mGameMode = GAMEMODE_READY;
                mStateTimer = 0;
            }
            break;

        case GAMEMODE_GAMEOVER:
            mStateTimer++;
            if (mStateTimer == 1)
                mSound.stopAllTracks();
            if (mControls->startButton() && mStateTimer > 60) {
                initLevel(true);
                mSound.playTrack(SND_START);
                mGameMode = GAMEMODE_READY;
                mStateTimer = 0;
            }
            break;
    }
}

// ============================================================
// Scroll
// ============================================================

// ============================================================
// Rendering: Bitmap-to-vector scan conversion
// ============================================================

void game::drawBitmap(int32_t px, int32_t py, uint32_t* list, uint32_t width, uint32_t height) {
    if (!list) return;
    if (px < 0) px = PACMAN_MAX_X + px;

    renderer::beginLines();
    for (uint32_t i = 0; i < height; i++) {
        uint32_t last_color = (uint32_t)-1;
        int32_t y_pos = py + (int32_t)i;
        float fy = (float)y_pos;
        float startX = 0;

        for (uint32_t j = 0; j < width; j++) {
            uint32_t x_pos = px + j;
            if (x_pos > PACMAN_MAX_X) break;
            uint32_t color = list[i * width + j];

            if (color != last_color) {
                if (last_color != 0 && last_color != (uint32_t)-1) {
                    float ex = (float)(px + j);
                    float cr = ((last_color >> 16) & 0xff) / 255.0f;
                    float cg = ((last_color >> 8) & 0xff) / 255.0f;
                    float cb = (last_color & 0xff) / 255.0f;
                    renderer::lineVertex(startX, fy, cr, cg, cb, 1.0f);
                    renderer::lineVertex(ex, fy, cr, cg, cb, 1.0f);
                }
                startX = (float)(px + j);
                last_color = color;
            }
        }
        if (last_color != 0 && last_color != (uint32_t)-1) {
            float ex = (float)(px + width);
            if (ex > PACMAN_MAX_X) ex = PACMAN_MAX_X;
            float cr = ((last_color >> 16) & 0xff) / 255.0f;
            float cg = ((last_color >> 8) & 0xff) / 255.0f;
            float cb = (last_color & 0xff) / 255.0f;
            renderer::lineVertex(startX, fy, cr, cg, cb, 1.0f);
            renderer::lineVertex(ex, fy, cr, cg, cb, 1.0f);
        }
    }
    renderer::endLines();
}

// ============================================================
// Draw string
// ============================================================

void game::drawStr(int32_t px, int32_t py, const char* str) {
    uint32_t len = strlen(str);
    uint32_t totalWidth = len * s_char_set_img.width;

    for (uint32_t i = 0; i < len; i++) {
        uint8_t ch = toupper(str[i]) & 0x7f;
        for (uint32_t h = 0; h < s_char_set_img.height; h++) {
            for (uint32_t w = 0; w < s_char_set_img.width; w++) {
                uint32_t srcOff = h * s_char_set_img.width + w;
                uint32_t dstOff = h * totalWidth + i * s_char_set_img.width + w;
                mBmpBuf[dstOff] = s_char_set_img.bmp[ch][srcOff];
            }
        }
    }
    drawBitmap(px + 2, py - 7, mBmpBuf, totalWidth, s_char_set_img.height);
}

void game::drawColorStr(int32_t px, int32_t py, const char* str, uint32_t color) {
    uint32_t len = strlen(str);
    uint32_t totalWidth = len * s_char_set_img.width;

    for (uint32_t i = 0; i < len; i++) {
        uint8_t ch = toupper(str[i]) & 0x7f;
        for (uint32_t h = 0; h < s_char_set_img.height; h++) {
            for (uint32_t w = 0; w < s_char_set_img.width; w++) {
                uint32_t srcOff = h * s_char_set_img.width + w;
                uint32_t dstOff = h * totalWidth + i * s_char_set_img.width + w;
                uint32_t px = s_char_set_img.bmp[ch][srcOff];
                mBmpBuf[dstOff] = (px != 0) ? color : 0;
            }
        }
    }
    drawBitmap(px + 2, py - 7, mBmpBuf, totalWidth, s_char_set_img.height);
}

void game::drawReadyText() {
    drawColorStr(90, 143, "READY!", 0xffff00);
}

// ============================================================
// Rendering: Maze background
// ============================================================

void game::drawBackground() {
    drawScore();

    uint32_t cnt = 0;
    uint32_t nbrPoly = s_background[cnt++];

    renderer::setLineWidth(2.0f);
    renderer::enableLineSmooth(true);

    for (uint32_t i = 0; i < nbrPoly; i++) {
        uint32_t nbrVertex = s_background[cnt++];
        renderer::beginLineStrip();
        for (uint32_t j = 0; j < nbrVertex; j++) {
            float x = (float)s_background[cnt++];
            float y = (float)s_background[cnt++];
            renderer::stripVertex(x, y, 0.0f, 0.0f, 0.75f, 1.0f);
        }
        renderer::endLineStrip();
    }

    // Ghost house door
    renderer::beginLineStrip();
    renderer::stripVertex(104.0f, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    renderer::stripVertex(105.0f, 101.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    renderer::stripVertex(120.0f, 101.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    renderer::stripVertex(121.0f, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    renderer::endLineStrip();
    renderer::beginLineStrip();
    renderer::stripVertex(104.0f, 103.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    renderer::stripVertex(104.0f, 102.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    renderer::stripVertex(120.0f, 102.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    renderer::stripVertex(121.0f, 103.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    renderer::endLineStrip();

    renderer::enableLineSmooth(false);
    renderer::setLineWidth(1.0f);
}

// ============================================================
// Rendering: Food dot
// ============================================================

void game::drawDot(int32_t px, int32_t py) {
    float x = (float)(px + 4);
    float y = (float)(py + 4);
    renderer::beginPoints();
    renderer::pointVertex(x, y, 1.0f, 0.72f, 0.59f, 1.0f);
    renderer::endPoints();
}


// ============================================================
// Rendering: Score
// ============================================================

void game::drawScore() {
    char buf[64];

    if (mGameMode == GAMEMODE_PLAYING) {
        if ((mFrameCounter / 15) & 1)
            drawStr(16, -18, "1UP");
    } else {
        drawStr(16, -18, "1UP");
    }
    snprintf(buf, sizeof(buf), "%u", mScore);
    drawStr(32, -8, buf);

    drawStr(72, -18, "HIGH SCORE");
    snprintf(buf, sizeof(buf), "%u", mHighScore);
    uint32_t len = strlen(buf);
    int32_t hx = 126 - (int32_t)(len * s_char_set_img.width);
    drawStr(hx, -8, buf);
}

// ============================================================
// Rendering: Lives
// ============================================================

void game::drawLives() {
    int32_t x = 100;
    for (uint32_t i = 0; i < mLives && i < 5; i++) {
        drawBitmap(x, 252, sPacmanImg.bmp[DIR_LEFT][1], sPacmanImg.width, sPacmanImg.height);
        x -= sPacmanImg.width + 2;
    }
}

// ============================================================
// Rendering: Level indicator
// ============================================================

void game::drawLevel() {
    uint32_t imgs[7], cnt = 0, total = 0;
    for (uint32_t i = 1; i <= mLevel; i++) {
        int32_t index = levelToFruitIndex(i);
        if (index != -1) {
            imgs[cnt] = sFruitTable[index].fruit;
            cnt = (cnt + 1) % 7;
            if (total < 7) total++;
        }
    }
    uint32_t x = 210;
    uint32_t start = cnt % (total ? total : 1);
    for (uint32_t i = 0; i < total; i++) {
        drawBitmap(x, 250, sFruitImg.bmp[imgs[start]], sFruitImg.width, sFruitImg.height);
        start = (start + 1) % 7;
        x -= sFruitImg.width + 2;
    }
}

// ============================================================
// Draw everything
// ============================================================

void game::draw() {
    renderer::setBlendAdditive(true);
    entityPacman* pac = getPacman();

    switch (mGameMode) {
        case GAMEMODE_ATTRACT:
        {
            drawBackground();
            renderer::sortBarrier();
            drawStr(60, 140, "VECTOR PAC-MAN");
            drawStr(72, 170, "PRESS START");
            mGhostFrame = (mGhostFrame + 1) % (8 * 2);
            break;
        }

        case GAMEMODE_READY:
        {
            drawBackground();
            renderer::sortBarrier();
            drawLevel();
            drawLives();
            for (uint32_t j = 0; j < 31; j++) {
                for (uint32_t i = 0; i < 28; i++) {
                    if (mMap[j][i] == TILE_FOOD) drawDot(i << 3, j << 3);
                    if (mMap[j][i] == TILE_ENERGIZER && mGhostFrame > 5)
                        drawBitmap(i << 3, j << 3, s_energizer, 9, 9);
                }
            }
            drawReadyText();
            if (mSound.getTrackProgress(SND_START) >= 0.75f || !mSound.isTrackPlaying(SND_START)) {
                mEnemies->draw();
                mPlayers->draw();
            }
            break;
        }

        case GAMEMODE_PLAYING:
        {
            drawBackground();
            renderer::sortBarrier();
            drawLevel();
            drawLives();

            if (pac->getDead() != 0) {
                if (pac->getDead() < 0 || pac->getDead() == 1) {
                    // Post-animation: empty maze only
                } else if (mStateTimer <= 30) {
                    // Phase 1 freeze
                    mEnemies->draw();
                    mPlayers->draw();
                } else {
                    // Phase 2: death animation
                    int frame = (75 - pac->getDead()) * 13 / 75;
                    if (frame < 0) frame = 0;
                    if (frame > 12) frame = 12;
                    drawBitmap(pac->getX() - 7, pac->getY() - 7,
                               sPacDeadImg.bmp[frame], sPacDeadImg.width, sPacDeadImg.height);
                }
                break;
            } else if (pac->getEatGhost() > 0) {
                if (pac->getGhostCounter())
                    drawBitmap(pac->getX() - 8, pac->getY() - 4,
                               sGhostCounterImg.bmp[pac->getGhostCounter() - 1],
                               sGhostCounterImg.width, sGhostCounterImg.height);
            } else {
                // Draw dots
                renderer::setPointSize(3.0f);
                for (uint32_t j = 0; j < 31; j++) {
                    for (uint32_t i = 0; i < 28; i++) {
                        if (mMap[j][i] == TILE_FOOD) drawDot(i << 3, j << 3);
                        if (mMap[j][i] == TILE_ENERGIZER && mGhostFrame > 5)
                            drawBitmap(i << 3, j << 3, s_energizer, 9, 9);
                    }
                }
                renderer::setPointSize(1.0f);

                // Fruit
                mFruit->draw();

                // Ghosts
                mEnemies->draw();

                // Pac-Man
                mPlayers->draw();
            }
            break;
        }

        case GAMEMODE_LEVEL_COMPLETE:
        {
            if ((mStateTimer >> 4) & 1) {
                uint32_t cnt = 0;
                uint32_t nbrPoly = s_background[cnt++];
                renderer::setLineWidth(2.0f);
                for (uint32_t i = 0; i < nbrPoly; i++) {
                    uint32_t nbrVertex = s_background[cnt++];
                    renderer::beginLineStrip();
                    for (uint32_t j = 0; j < nbrVertex; j++) {
                        float x = (float)s_background[cnt++];
                        float y = (float)s_background[cnt++];
                        renderer::stripVertex(x, y, 0.75f, 0.75f, 0.75f, 1.0f);
                    }
                    renderer::endLineStrip();
                }
                renderer::setLineWidth(1.0f);
            } else {
                drawBackground();
                renderer::sortBarrier();
            }
            break;
        }

        case GAMEMODE_DEAD:
        {
            drawBackground();
            renderer::sortBarrier();
            drawLevel();
            drawLives();
            if (pac->getDead() > 15) {
                int frame = 15 - pac->getDead() / 5;
                if (frame >= 0 && frame < 13)
                    drawBitmap(pac->getX() - 7, pac->getY() - 7,
                               sPacDeadImg.bmp[frame], sPacDeadImg.width, sPacDeadImg.height);
            }
            break;
        }

        case GAMEMODE_GAMEOVER:
        {
            drawBackground();
            renderer::sortBarrier();
            for (uint32_t j = 0; j < 31; j++) {
                for (uint32_t i = 0; i < 28; i++) {
                    if (mMap[j][i] == TILE_FOOD) drawDot(i << 3, j << 3);
                    if (mMap[j][i] == TILE_ENERGIZER && mGhostFrame > 5)
                        drawBitmap(i << 3, j << 3, s_energizer, 9, 9);
                }
            }
            drawLives();
            // Show ghosts in attract poses using entity positions
            drawBitmap(122, 110, sGhostImg.bmp[3][DIR_UP], sGhostImg.width, sGhostImg.height);
            drawBitmap(106, 110, sGhostImg.bmp[1][DIR_DOWN], sGhostImg.width, sGhostImg.height);
            drawBitmap(90, 110, sGhostImg.bmp[2][DIR_UP], sGhostImg.width, sGhostImg.height);
            drawBitmap(106, 85, sGhostImg.bmp[0][DIR_LEFT], sGhostImg.width, sGhostImg.height);
            drawStr(74, 143, "GAME OVER");
            mGhostFrame = (mGhostFrame + 1) % (8 * 2);
            break;
        }
    }

}
