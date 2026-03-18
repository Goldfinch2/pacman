#include "game.h"
#include "renderer.h"
#include "pacdefs.h"
#include "pac_char_set.h"
#include "SDL.h"

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

// Static game mode
game::GameMode game::mGameMode = game::GAMEMODE_ATTRACT;

// ============================================================
// Image structure types (same as original)
// ============================================================

typedef struct {
    uint32_t width, height;
    uint32_t *bmp[4][3];
} pacman_img_t;

typedef struct {
    uint32_t width, height;
    uint32_t *bmp[13];
} pacdead_img_t;

typedef struct {
    uint32_t width, height;
    uint32_t *bmp[8];
} fruit_img_t;

typedef struct {
    uint32_t width, height;
    uint32_t *bmp[4][4];
} ghost_img_t;

typedef struct {
    uint32_t width, height;
    uint32_t *bmp[4];
} ghost_counter_img_t;

typedef struct {
    uint32_t width, height;
    uint32_t *bmp[128];
} char_set_img_t;

// ============================================================
// Fruit table
// ============================================================

static FruitInfo s_fruit[] = {
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
// Image tables (same as original)
// ============================================================

static fruit_img_t s_fruit_img = {
    12, 12,
    { s_cherry_img, s_straw_img, s_pear_img, s_apple_img,
      s_jack_img, s_eagle_img, s_bell_img, s_key_img }
};

static pacman_img_t s_pacman_img = {
    14, 14,
    {
        { s_pac_right1_img, s_pac_right2_img, s_pac_right3_img },
        { s_pac_left1_img,  s_pac_left2_img,  s_pac_left3_img },
        { s_pac_up1_img,    s_pac_up2_img,    s_pac_up3_img },
        { s_pac_down1_img,  s_pac_down2_img,  s_pac_down3_img }
    }
};

static ghost_img_t s_ghost_img = {
    14, 14,
    {
        { s_blinky_right_img, s_blinky_left_img, s_blinky_up_img, s_blinky_down_img },
        { s_pinky_right_img,  s_pinky_left_img,  s_pinky_up_img,  s_pinky_down_img },
        { s_inky_right_img,   s_inky_left_img,   s_inky_up_img,   s_inky_down_img },
        { s_clyde_right_img,  s_clyde_left_img,  s_clyde_up_img,  s_clyde_down_img }
    }
};

static ghost_img_t s_ghost_dead_img = {
    14, 14,
    {
        { s_ghost_dead_right_img, s_ghost_dead_left_img, s_ghost_dead_up_img, s_ghost_dead_down_img },
        {}, {}, {}
    }
};

static ghost_img_t s_ghost_scared_img = {
    14, 14,
    {
        { s_ghost_scared1_img, s_ghost_scared1_img, s_ghost_scared2_img, s_ghost_scared2_img },
        {}, {}, {}
    }
};

static pacdead_img_t s_pacdead_img = {
    14, 14,
    {
        s_pac_right3_img,
        s_pac_dead1_img, s_pac_dead2_img, s_pac_dead3_img,
        s_pac_dead4_img, s_pac_dead5_img, s_pac_dead6_img,
        s_pac_dead7_img, s_pac_dead8_img, s_pac_dead9_img,
        s_pac_dead10_img, s_pac_dead11_img, s_pac_dead12_img
    }
};

static ghost_counter_img_t s_ghostcounter_img = {
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
{
    mGameMode = GAMEMODE_ATTRACT;
    mScore = 0;
    mHighScore = 0;
    mLives = 0;
    mLevel = 1;
    mStateTimer = 0;
    mFrameCounter = 0;
    mKeyRight = mKeyLeft = mKeyUp = mKeyDown = false;
    mKeyStart = mKeyCoin = false;
}

game::~game() {}

void game::init()
{
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
// Input reading from SDL keyboard state
// ============================================================

void game::readInput()
{
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    mKeyRight = keys[SDL_SCANCODE_RIGHT] != 0;
    mKeyLeft  = keys[SDL_SCANCODE_LEFT] != 0;
    mKeyUp    = keys[SDL_SCANCODE_UP] != 0;
    mKeyDown  = keys[SDL_SCANCODE_DOWN] != 0;
    mKeyStart = keys[SDL_SCANCODE_RETURN] != 0 || keys[SDL_SCANCODE_SPACE] != 0 || keys[SDL_SCANCODE_LCTRL] != 0;
    mKeyCoin  = keys[SDL_SCANCODE_5] != 0;
}

// ============================================================
// Helper functions (ported directly from original)
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
    for (uint32_t i = 0; i < ARRAY_SIZE(s_fruit); i++) {
        if (level >= s_fruit[i].min_level && level <= s_fruit[i].max_level)
            return (int32_t)i;
    }
    return -1;
}

void game::snapY() {
    int32_t a = wrapMapY((mPacY + 3) >> 3);
    int32_t b = wrapMapX(mPacX >> 3);
    if (mMap[a][b] == TILE_WALL) mPacY--;
    a = wrapMapY((mPacY - 4) >> 3);
    b = wrapMapX(mPacX >> 3);
    if (mMap[a][b] == TILE_WALL) mPacY++;
}

void game::snapX() {
    int32_t a = wrapMapY(mPacY >> 3);
    int32_t b = wrapMapX((mPacX + 3) >> 3);
    if (mMap[a][b] == TILE_WALL) mPacX--;
    a = wrapMapY(mPacY >> 3);
    b = wrapMapX((mPacX - 4) >> 3);
    if (mMap[a][b] == TILE_WALL) mPacX++;
}

// ============================================================
// Ghost logic
// ============================================================

void game::initGhost(uint32_t ghost, uint32_t x, uint32_t y, uint32_t dir, uint32_t mode, bool scared, uint32_t speed) {
    mGhosts[ghost].x = x;
    mGhosts[ghost].y = y;
    mGhosts[ghost].dir = dir;
    mGhosts[ghost].mode = mode;
    mGhosts[ghost].scared = scared;
    mGhosts[ghost].speed = speed;
}

void game::resetGhost(uint32_t ghost, uint32_t time) {
    if (mGhosts[ghost].mode < GHOST_TRAPPED) {
        mGhosts[ghost].mode = 4 + time;
        mGhosts[ghost].x = 112;
        mGhosts[ghost].y = 96;
    }
}

void game::frighten(uint32_t ghost) {
    mGhosts[ghost].scared = true;
}

void game::reverseDirection(uint32_t ghost) {
    if (mGhosts[ghost].dir == DIR_RIGHT) mGhosts[ghost].dir = DIR_LEFT;
    else if (mGhosts[ghost].dir == DIR_LEFT) mGhosts[ghost].dir = DIR_RIGHT;
    else if (mGhosts[ghost].dir == DIR_UP) mGhosts[ghost].dir = DIR_DOWN;
    else mGhosts[ghost].dir = DIR_UP;
}

void game::eatFood() {
    mFoodEaten++;
    if (mFoodEaten == 70 || mFoodEaten == 170)
        mCherry = true;
    if (mFoodEaten == 30)
        mGhosts[INKY].mode = GHOST_RELEASED;
    if (mFoodEaten == 82)
        mGhosts[CLYDE].mode = GHOST_RELEASED;
}

void game::processGhost(uint32_t ghost) {
    uint32_t targetX = 0, targetY = 0;
    bool tunnel = inTunnel(mGhosts[ghost].x, mGhosts[ghost].y);

    if (mEnergizerTimer == 0 && mGhosts[ghost].scared)
        mGhosts[ghost].scared = false;

    // Collision with pac-man
    if (((mPacX >> 3) == (mGhosts[ghost].x + 4) >> 3) && ((mPacY >> 3) == (mGhosts[ghost].y + 4) >> 3)) {
        if (mGhosts[ghost].scared) {
            mGhosts[ghost].mode = GHOST_DEAD;
            mGhosts[ghost].scared = false;
            mGhosts[ghost].x &= 0xfffffffe;
            mGhosts[ghost].y &= 0xfffffffe;
            mPacGhostCounter++;
            if (mPacGhostCounter > 4) mPacGhostCounter = 1;
            mScore += 100 << mPacGhostCounter;
            mPacEatGhost = 40;
            mSound.playTrack(SND_EAT_GHOST);
            mSound.stopTrack(SND_FRIGHTENED);
            mSound.playTrack(SND_GHOST_EYES);
        } else if (mGhosts[ghost].mode != GHOST_DEAD) {
            mPacDead = 75;  // animation length; mStateTimer drives the phases
            mStateTimer = 0;
            resetGhost(BLINKY, 0);
            resetGhost(PINKY, 10);
            resetGhost(INKY, 20);
            resetGhost(CLYDE, 30);
            mSound.stopAllTracks();
        }
    }

newDir:
    if (mGhosts[ghost].x % 8 == 0 && mGhosts[ghost].y % 8 == 0 && !tunnel) {
        if (mGhosts[ghost].scared) {
            targetX = rand() % 28;
            targetY = rand() % 31;
        } else if (mGhosts[ghost].mode == GHOST_NORMAL) {
            if (mGlobalMode == 0) {
                // Scatter
                if (ghost == PINKY)  { targetX = 3; targetY = 0; }
                if (ghost == BLINKY) { targetX = 25; targetY = 0; }
                if (ghost == INKY)   { targetX = 28; targetY = 31; }
                if (ghost == CLYDE)  { targetX = 0; targetY = 31; }
            } else {
                // Chase
                targetX = mPacX >> 3;
                targetY = mPacY >> 3;
                if (ghost == PINKY) {
                    targetX += xDir(mPacDir) << 2;
                    targetY += yDir(mPacDir) << 2;
                }
                if (ghost == CLYDE) {
                    uint32_t distance = (mGhosts[ghost].y / 8 - (mPacY >> 3)) * (mGhosts[ghost].y / 8 - (mPacY >> 3)) +
                                        (mGhosts[ghost].x / 8 - 1 - (mPacX >> 3)) * (mGhosts[ghost].x / 8 - 1 - (mPacX >> 3));
                    if (distance < 64) { targetX = 0; targetY = 31; }
                }
                if (ghost == INKY) {
                    uint32_t xd, yd;
                    targetX += xDir(mPacDir) << 1;
                    targetY += yDir(mPacDir) << 1;
                    xd = mGhosts[BLINKY].x >> 3;
                    yd = mGhosts[BLINKY].y >> 3;
                    xd -= targetX;
                    yd -= targetY;
                    xd = xd << 1;
                    yd = yd << 1;
                    targetX -= xd;
                    targetY -= yd;
                }
            }
        }
        if (mGhosts[ghost].mode == GHOST_DEAD) {
            targetX = 14; targetY = 11;
            if (mGhosts[ghost].x >> 3 == 13 && mGhosts[ghost].y >> 3 == 11) {
                mGhosts[ghost].mode = GHOST_TRAPPED + 30;
                mGhosts[ghost].y++;
                mGhosts[ghost].dir = DIR_DOWN;
                goto newDir;
            }
        }
        if (mGhosts[ghost].mode == GHOST_RELEASED) {
            targetX = 13; targetY = 11;
            if (mGhosts[ghost].y >> 3 == 13 && mGhosts[ghost].x >> 3 == 13) {
                mGhosts[ghost].mode = GHOST_NORMAL;
                mGhosts[ghost].dir = DIR_UP;
                mGhosts[ghost].y--;
            }
        }
        if (mGhosts[ghost].mode >= GHOST_TRAPPED) {
            if (mGhosts[ghost].mode < 255) mGhosts[ghost].mode--;
            targetY = 14; targetX = 13;
            if (ghost == INKY) targetX = 11;
            if (ghost == CLYDE) targetX = 15;
        }

        // Find best direction (shortest euclidean distance to target)
        uint32_t distR = 10000, distL = 10000, distU = 10000, distD = 10000, minimum = 10000;
        int32_t y, x;

        y = wrapMapY(mGhosts[ghost].y >> 3);
        x = wrapMapX((mGhosts[ghost].x >> 3) + 1);
        if (mGhosts[ghost].dir != DIR_LEFT && mMap[y][x] != TILE_WALL) {
            distR = (mGhosts[ghost].y/8 - targetY)*(mGhosts[ghost].y/8 - targetY) + (mGhosts[ghost].x/8 + 1 - targetX)*(mGhosts[ghost].x/8 + 1 - targetX);
            if (distR < minimum) minimum = distR;
        }
        y = wrapMapY(mGhosts[ghost].y >> 3);
        x = wrapMapX((mGhosts[ghost].x >> 3) - 1);
        if (mGhosts[ghost].dir != DIR_RIGHT && mMap[y][x] != TILE_WALL) {
            distL = (mGhosts[ghost].y/8 - targetY)*(mGhosts[ghost].y/8 - targetY) + (mGhosts[ghost].x/8 - 1 - targetX)*(mGhosts[ghost].x/8 - 1 - targetX);
            if (distL < minimum) minimum = distL;
        }
        y = wrapMapY((mGhosts[ghost].y >> 3) - 1);
        x = wrapMapX(mGhosts[ghost].x >> 3);
        if (mGhosts[ghost].dir != DIR_DOWN && mMap[y][x] != TILE_WALL) {
            distU = (mGhosts[ghost].y/8 - 1 - targetY)*(mGhosts[ghost].y/8 - 1 - targetY) + (mGhosts[ghost].x/8 - targetX)*(mGhosts[ghost].x/8 - targetX);
            if (distU < minimum) minimum = distU;
        }
        y = wrapMapY((mGhosts[ghost].y >> 3) + 1);
        x = wrapMapX(mGhosts[ghost].x >> 3);
        if (mGhosts[ghost].dir != DIR_UP && mMap[y][x] != TILE_WALL) {
            distD = (mGhosts[ghost].y/8 + 1 - targetY)*(mGhosts[ghost].y/8 + 1 - targetY) + (mGhosts[ghost].x/8 - targetX)*(mGhosts[ghost].x/8 - targetX);
            if (distD < minimum) minimum = distD;
        }

        if (distU == minimum) mGhosts[ghost].dir = DIR_UP;
        else if (distR == minimum) mGhosts[ghost].dir = DIR_RIGHT;
        else if (distD == minimum) mGhosts[ghost].dir = DIR_DOWN;
        else if (distL == minimum) mGhosts[ghost].dir = DIR_LEFT;
    }

    // Speed control
    uint32_t doubleSpeed = 1, speed = 9;
    if (tunnel) speed = 5;
    if (mGhosts[ghost].scared) speed = 6;
    if ((mGhosts[ghost].x % 8) == 0 && (mGhosts[ghost].y % 8) == 0) speed = 10;
    if (mGhosts[ghost].mode == GHOST_DEAD) { doubleSpeed = 2; speed = 10; }

    while (doubleSpeed--) {
        mGhosts[ghost].speed += speed;
        if (mGhosts[ghost].speed > 9) {
            if (mGhosts[ghost].dir == DIR_RIGHT) mGhosts[ghost].x++;
            if (mGhosts[ghost].dir == DIR_LEFT)  mGhosts[ghost].x--;
            if (mGhosts[ghost].dir == DIR_UP)    mGhosts[ghost].y--;
            if (mGhosts[ghost].dir == DIR_DOWN)   mGhosts[ghost].y++;
            mGhosts[ghost].speed -= 10;
        }
        mGhosts[ghost].x = wrap(mGhosts[ghost].x);
    }
}

// ============================================================
// Level init
// ============================================================

void game::initLevel(bool fullReset) {
    initGhost(BLINKY, 112, 88, 0, 0, false, 0);
    initGhost(PINKY,  112, 96, 0, 4, false, 0);
    initGhost(INKY,   112, 96, 0, 255, false, 0);
    initGhost(CLYDE,  112, 96, 0, 255, false, 0);
    mBoundary = 5;
    mCornering = 2;
    mPacX = 112;
    mPacY = 188;
    mPacDir = DIR_LEFT;
    mPacFrame = 6;  // start with mouth closed (frame index 6/3 = 2)
    mFoodEaten = 0;
    mPacDead = 0;
    mPacEatGhost = 0;
    mPacGhostCounter = 0;
    mGlobalMode = 1; // chase
    mCherry = false;
    mCherryTimer = 0;
    mEnergizerTimer = 0;
    if (fullReset) {
        mScore = 0;
        mLives = 4;
        mLevel = 1;
    }
    mGhostFrame = 0;
    memcpy(mMap, s_org_map, sizeof(mMap));
}

// ============================================================
// Rendering: Bitmap-to-vector scan conversion
// ============================================================

void game::drawBitmap(int32_t px, int32_t py, uint32_t* list, uint32_t width, uint32_t height) {
    if (!list) return;
    if (px < 0) px = PACMAN_MAX_X + px;  // X wrap for tunnel sprites

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
                // End previous run
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
        // Finish last run on this row
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
// Draw string using bitmap font
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
// Rendering: Maze background (TRUE VECTOR)
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

    // Ghost house door in white (top and bottom edges)
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
// Rendering: Food dot (TRUE VECTOR - single bright point)
// ============================================================

void game::drawDot(int32_t px, int32_t py) {
    float x = (float)(px + 4);
    float y = (float)(py + 4);
    renderer::beginPoints();
    renderer::pointVertex(x, y, 1.0f, 0.72f, 0.59f, 1.0f);
    renderer::endPoints();
}

// ============================================================
// Rendering: Ghost
// ============================================================

void game::drawGhost(uint32_t ghost) {
    uint32_t* bmp = s_ghost_img.bmp[ghost][mGhosts[ghost].dir];
    if (mGhosts[ghost].scared) {
        bmp = s_ghost_scared_img.bmp[0][mGhostFrame / 8 + (mEnergizerTimer < 128 && (mEnergizerTimer % 32) < 16 ? 2 : 0)];
    }
    if (mGhosts[ghost].mode == GHOST_DEAD) {
        bmp = s_ghost_dead_img.bmp[0][mGhosts[ghost].dir];
    }
    drawBitmap(mGhosts[ghost].x - 2, mGhosts[ghost].y - 2, bmp, 14, 14);
}

// ============================================================
// Rendering: Score
// ============================================================

void game::drawScore() {
    char buf[64];

    // "1UP" blinks when playing (~2 blinks/sec), solid otherwise
    if (mGameMode == GAMEMODE_PLAYING) {
        if ((mFrameCounter / 15) & 1)
            drawStr(16, -18, "1UP");
    } else {
        drawStr(16, -18, "1UP");
    }
    snprintf(buf, sizeof(buf), "%u", mScore);
    drawStr(32, -8, buf);

    // "HIGH SCORE" centered at top
    drawStr(72, -18, "HIGH SCORE");
    snprintf(buf, sizeof(buf), "%u", mHighScore);
    // Right-align value so last digit is under "O" of SCORE
    // "HIGH SCORE" at X=72, "O" is char index 7 → center at X = 72 + 2 + 7*8 + 4 = 134
    uint32_t len = strlen(buf);
    int32_t hx = 126 - (int32_t)(len * s_char_set_img.width);
    drawStr(hx, -8, buf);
}

// ============================================================
// Rendering: Lives
// ============================================================

void game::drawLives() {
    // Draw from center-left, expanding leftward, mouth facing left
    int32_t x = 100;
    for (uint32_t i = 0; i < mLives && i < 5; i++) {
        drawBitmap(x, 252, s_pacman_img.bmp[DIR_LEFT][1], s_pacman_img.width, s_pacman_img.height);
        x -= s_pacman_img.width + 2;
    }
}

// ============================================================
// Rendering: Level indicator (fruit icons)
// ============================================================

void game::drawLevel() {
    uint32_t imgs[7], cnt = 0, total = 0;
    for (uint32_t i = 1; i <= mLevel; i++) {
        int32_t index = levelToFruitIndex(i);
        if (index != -1) {
            imgs[cnt] = s_fruit[index].fruit;
            cnt = (cnt + 1) % 7;
            if (total < 7) total++;
        }
    }
    uint32_t x = 210;
    uint32_t start = cnt % (total ? total : 1);
    for (uint32_t i = 0; i < total; i++) {
        drawBitmap(x, 250, s_fruit_img.bmp[imgs[start]], s_fruit_img.width, s_fruit_img.height);
        start = (start + 1) % 7;
        x -= s_fruit_img.width + 2;
    }
}

// ============================================================
// Main frame logic (ported from original run_frame)
// ============================================================

void game::runFrame() {
    // Death sequence uses mStateTimer to track progress:
    //   0..59:  freeze (1 sec), everything visible
    //   60:     start death sound + animation
    //   60+:    animation plays, wait for sound to finish, then transition
    if (mPacDead != 0) {
        mStateTimer++;

        if (mStateTimer == 20) {
            // Play death sound ~170ms before animation starts
            mSound.playTrack(SND_DEATH);
        }

        if (mStateTimer <= 30) {
            // Phase 1: frozen for ~0.5 second, don't touch mPacDead
            return;
        }

        // Phase 2: run animation (mPacDead counts down from its initial value)
        if (mPacDead > 1) {
            mPacDead--;
            return;
        }

        // Phase 3: animation done, wait for sound to finish
        if (mSound.isTrackPlaying(SND_DEATH)) {
            return;
        }

        // Phase 4: 1 second pause after death before resuming
        // Use mPacDead == 1 as "waiting for post-death delay"
        // Hijack mStateTimer for the countdown (reset it once)
        if (mPacDead == 1) {
            mPacDead = -60;  // negative = post-death delay counter
            return;
        }
        if (mPacDead < -1) {
            mPacDead++;
            return;
        }

        // Done — transition (mPacDead == -1)
        mPacDead = 0;
        if (mLives) mLives--;
        if (mLives == 0) {
            mGameMode = GAMEMODE_GAMEOVER;
            mStateTimer = 0;
        } else {
            // Reset pac-man and ghosts to starting positions
            mPacX = 112; mPacY = 188; mPacDir = DIR_LEFT;
            mPacFrame = 6;  // mouth closed
            mCherry = false; mCherryTimer = 0;
            mEnergizerTimer = 0;
            initGhost(BLINKY, 112, 88, 0, 0, false, 0);
            initGhost(PINKY,  112, 96, 0, 4, false, 0);
            initGhost(INKY,   112, 96, 0, 255, false, 0);
            initGhost(CLYDE,  112, 96, 0, 255, false, 0);
            mGameMode = GAMEMODE_READY;
            mStateTimer = 0;  // no start sound; READY shows for 1 sec
        }
        return;
    }

    // Eating ghost pause
    if (mPacEatGhost > 0) {
        mPacEatGhost--;
        return;
    }

    // Check food/energizer at pacman position
    uint8_t temp = mMap[mPacY >> 3][mPacX >> 3];
    if (temp == TILE_FOOD) {
        mMap[mPacY >> 3][mPacX >> 3] = TILE_EMPTY;
        mScore += 10;
        mSound.playTrack(SND_CHOMP);
        eatFood();
    } else if (temp == TILE_ENERGIZER) {
        mMap[mPacY >> 3][mPacX >> 3] = TILE_EMPTY;
        mScore += 50;
        frighten(BLINKY); frighten(INKY); frighten(PINKY); frighten(CLYDE);
        reverseDirection(BLINKY); reverseDirection(INKY); reverseDirection(PINKY); reverseDirection(CLYDE);
        mEnergizerTimer = ENERGY_LENGTH;
        mSound.stopTrack(SND_SIREN);
        mSound.playTrack(SND_FRIGHTENED);
        eatFood();
    } else if (mCherry && mPacX == 112 && mPacY == 140) {
        int32_t index = levelToFruitIndex(mLevel);
        if (index != -1) mScore += s_fruit[index].score;
        mCherry = false;
        mCherryTimer = 75;
        mSound.playTrack(SND_EAT_FRUIT);
    } else {
        // Movement
        bool tunnel = inTunnel(mPacX, mPacY);
        int32_t a, b;
        uint8_t tmod = (mPacY + 4) % 8;
        bool corner = (tmod < mCornering) || (tmod > 8 - mCornering);

        if ((mKeyRight && corner) || (mPacDir == DIR_RIGHT)) {
            a = wrapMapY(mPacY >> 3);
            b = wrapMapX((mPacX + mBoundary) >> 3);
            if (mMap[a][b] != TILE_WALL) {
                mPacX++; mPacDir = DIR_RIGHT; mPacFrame++; snapY();
            }
        }
        if ((mKeyLeft && corner) || (mPacDir == DIR_LEFT)) {
            a = wrapMapY(mPacY >> 3);
            b = wrapMapX((mPacX - mBoundary) >> 3);
            if (mMap[a][b] != TILE_WALL) {
                mPacX--; mPacDir = DIR_LEFT; mPacFrame++; snapY();
            }
        }
        tmod = (mPacX + 4) % 8;
        if ((mKeyDown && corner && !tunnel) || (mPacDir == DIR_DOWN)) {
            a = wrapMapY((mPacY + mBoundary) >> 3);
            b = wrapMapX(mPacX >> 3);
            if (mMap[a][b] != TILE_WALL) {
                mPacY++; mPacDir = DIR_DOWN; mPacFrame++; snapX();
            }
        }
        if ((mKeyUp && corner && !tunnel) || (mPacDir == DIR_UP)) {
            a = wrapMapY((mPacY - mBoundary) >> 3);
            b = wrapMapX(mPacX >> 3);
            if (mMap[a][b] != TILE_WALL) {
                mPacY--; mPacDir = DIR_UP; mPacFrame++; snapX();
            }
        }
        mPacX = wrap(mPacX);
        mPacY = limit(mPacY);
    }

    // Energizer timer
    if (!mEnergizerTimer) {
        mPacGhostCounter = 0;
    } else {
        mEnergizerTimer--;
        if (mEnergizerTimer == 0) {
            // Fright ended — switch back to siren
            mSound.stopTrack(SND_FRIGHTENED);
            mSound.stopTrack(SND_GHOST_EYES);
            mSound.playTrack(SND_SIREN);
        }
    }

    // Process ghosts
    processGhost(BLINKY);
    processGhost(INKY);
    processGhost(PINKY);
    processGhost(CLYDE);

    // Update high score
    if (mScore > mHighScore) mHighScore = mScore;

    // Level complete check
    if (mFoodEaten >= TOTAL_DOTS) {
        mGameMode = GAMEMODE_LEVEL_COMPLETE;
        mStateTimer = 0;
    }

    mGhostFrame = (mGhostFrame + 1) % (8 * 2);
    mPacFrame = mPacFrame % (3 * 3);
}

// ============================================================
// Game state machine (run)
// ============================================================

void game::run() {
    readInput();
    mFrameCounter++;

    switch (mGameMode) {
        case GAMEMODE_ATTRACT:
            if (mKeyStart) {
                initLevel(true);
                mSound.stopAllTracks();
                mSound.playTrack(SND_START);
                mGameMode = GAMEMODE_READY;
                mStateTimer = 0;
            }
            break;

        case GAMEMODE_READY:
            mStateTimer++;
            // Phase 1: wait for start.wav to finish
            // Phase 2: wait 1 second (60 frames) after it ends
            if (mSound.isTrackPlaying(SND_START)) {
                mStateTimer = 0;  // reset delay counter while sound plays
            } else if (mStateTimer > 60) {
                mGameMode = GAMEMODE_PLAYING;
                mSound.playTrack(SND_SIREN);
            }
            break;

        case GAMEMODE_PLAYING:
            runFrame();
            break;

        case GAMEMODE_DEAD:
            // handled inside runFrame via mPacDead
            runFrame();
            break;

        case GAMEMODE_LEVEL_COMPLETE:
            mStateTimer++;
            if (mStateTimer == 1) {
                mSound.stopAllTracks();
            }
            if (mStateTimer > 180) { // 3 seconds
                mLevel++;
                initLevel(false);
                mSound.playTrack(SND_START);
                mGameMode = GAMEMODE_READY;
                mStateTimer = 0;
            }
            break;

        case GAMEMODE_GAMEOVER:
            mStateTimer++;
            if (mStateTimer == 1) {
                mSound.stopAllTracks();
            }
            if (mKeyStart && mStateTimer > 60) {
                initLevel(true);
                mSound.playTrack(SND_START);
                mGameMode = GAMEMODE_READY;
                mStateTimer = 0;
            }
            break;
    }
}

// ============================================================
// Draw everything
// ============================================================

void game::draw() {
    renderer::setBlendAdditive(true);

    switch (mGameMode) {
        case GAMEMODE_ATTRACT:
        {
            // Draw maze and "PRESS START" text
            drawBackground();
            renderer::sortBarrier();
            drawStr(60, 140, "VECTOR PAC-MAN");
            drawStr(72, 170, "PRESS START");

            // Animate ghost frame for energizer blink
            mGhostFrame = (mGhostFrame + 1) % (8 * 2);
            break;
        }

        case GAMEMODE_READY:
        {
            drawBackground();
            renderer::sortBarrier();
            drawLevel();
            drawLives();
            // Draw dots
            for (uint32_t j = 0; j < 31; j++) {
                for (uint32_t i = 0; i < 28; i++) {
                    if (mMap[j][i] == TILE_FOOD) drawDot(i << 3, j << 3);
                    if (mMap[j][i] == TILE_ENERGIZER && mGhostFrame > 5)
                        drawBitmap(i << 3, j << 3, s_energizer, 9, 9);
                }
            }
            // "READY!" in yellow from the start of the song
            drawReadyText();
            // Show pac-man and ghosts at 3/4 of start.wav
            if (mSound.getTrackProgress(SND_START) >= 0.75f || !mSound.isTrackPlaying(SND_START)) {
                drawGhost(BLINKY);
                drawGhost(PINKY);
                drawGhost(INKY);
                drawGhost(CLYDE);
                drawBitmap(mPacX - 6, mPacY - 6, s_pacman_img.bmp[mPacDir][mPacFrame / 3], s_pacman_img.width, s_pacman_img.height);
            }
            break;
        }

        case GAMEMODE_PLAYING:
        {
            drawBackground();
            renderer::sortBarrier();
            drawLevel();
            drawLives();

            if (mPacDead != 0) {
                if (mPacDead < 0 || mPacDead == 1) {
                    // Post-animation / waiting for sound: empty maze only
                } else if (mStateTimer <= 30) {
                    // Phase 1 freeze: draw pac-man + ghosts standing still
                    drawGhost(BLINKY);
                    drawGhost(PINKY);
                    drawGhost(INKY);
                    drawGhost(CLYDE);
                    drawBitmap(mPacX - 6, mPacY - 6, s_pacman_img.bmp[mPacDir][mPacFrame / 3], s_pacman_img.width, s_pacman_img.height);
                } else {
                    // Phase 2: death animation
                    int frame = (75 - mPacDead) * 13 / 75;
                    if (frame < 0) frame = 0;
                    if (frame > 12) frame = 12;
                    drawBitmap(mPacX - 7, mPacY - 7, s_pacdead_img.bmp[frame], s_pacdead_img.width, s_pacdead_img.height);
                }
                break;
            } else if (mPacEatGhost > 0) {
                if (mPacGhostCounter)
                    drawBitmap(mPacX - 8, mPacY - 4, s_ghostcounter_img.bmp[mPacGhostCounter - 1], s_ghostcounter_img.width, s_ghostcounter_img.height);
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

                // Cherry/fruit
                if (mCherry) {
                    int32_t index = levelToFruitIndex(mLevel);
                    if (index != -1)
                        drawBitmap(108, 134, s_fruit_img.bmp[s_fruit[index].fruit], s_fruit_img.width, s_fruit_img.height);
                }
                if (mCherryTimer) {
                    int32_t index = levelToFruitIndex(mLevel);
                    if (index != -1)
                        drawBitmap(106, 137, s_fruit[index].score_bmp, s_fruit[index].width, s_fruit[index].height);
                    mCherryTimer--;
                }

                // Ghosts
                drawGhost(BLINKY);
                drawGhost(PINKY);
                drawGhost(INKY);
                drawGhost(CLYDE);

                // Pac-Man
                drawBitmap(mPacX - 6, mPacY - 6, s_pacman_img.bmp[mPacDir][mPacFrame / 3], s_pacman_img.width, s_pacman_img.height);
            }
            break;
        }

        case GAMEMODE_LEVEL_COMPLETE:
        {
            // Flash maze
            if ((mStateTimer >> 4) & 1) {
                // Draw maze in white
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
            if (mPacDead > 15) {
                int frame = 15 - mPacDead / 5;
                if (frame >= 0 && frame < 13)
                    drawBitmap(mPacX - 7, mPacY - 7, s_pacdead_img.bmp[frame], s_pacdead_img.width, s_pacdead_img.height);
            }
            break;
        }

        case GAMEMODE_GAMEOVER:
        {
            drawBackground();
            renderer::sortBarrier();
            // Draw remaining dots
            for (uint32_t j = 0; j < 31; j++) {
                for (uint32_t i = 0; i < 28; i++) {
                    if (mMap[j][i] == TILE_FOOD) drawDot(i << 3, j << 3);
                    if (mMap[j][i] == TILE_ENERGIZER && mGhostFrame > 5)
                        drawBitmap(i << 3, j << 3, s_energizer, 9, 9);
                }
            }
            drawLives();
            // Show ghosts in attract poses
            drawBitmap(122, 110, s_ghost_img.bmp[CLYDE][DIR_UP], s_ghost_img.width, s_ghost_img.height);
            drawBitmap(106, 110, s_ghost_img.bmp[PINKY][DIR_DOWN], s_ghost_img.width, s_ghost_img.height);
            drawBitmap(90, 110, s_ghost_img.bmp[INKY][DIR_UP], s_ghost_img.width, s_ghost_img.height);
            drawBitmap(106, 85, s_ghost_img.bmp[BLINKY][DIR_LEFT], s_ghost_img.width, s_ghost_img.height);
            drawStr(74, 143, "GAME OVER");
            mGhostFrame = (mGhostFrame + 1) % (8 * 2);
            break;
        }
    }
}
