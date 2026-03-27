#ifndef GAME_H
#define GAME_H

#include "defines.h"
#include "sound.h"

// Sound IDs
enum {
    SND_START = 0,
    SND_CHOMP,
    SND_EAT_FRUIT,
    SND_EAT_GHOST,
    SND_DEATH,
    SND_SIREN,
    SND_FRIGHTENED,
    SND_GHOST_EYES,
    SND_EXTRA_LIFE,
    SND_LAST
};

// Directions
enum { DIR_RIGHT, DIR_LEFT, DIR_UP, DIR_DOWN, DIR_NONE };

// Tile types
enum { TILE_EMPTY, TILE_WALL, TILE_FOOD, TILE_ENERGIZER };

// Fruit types
enum { CHERRY, STRAWBERRY, PEAR, APPLE, JACK, EAGLE, BELL, KEY };

struct FruitInfo {
    uint32_t min_level, max_level;
    uint32_t fruit;
    uint32_t score;
    uint32_t width, height;
    uint32_t* score_bmp;
};

// Image structure types (used by entities for self-drawing)
struct pacman_img_t {
    uint32_t width, height;
    uint32_t *bmp[4][3];
};

struct pacdead_img_t {
    uint32_t width, height;
    uint32_t *bmp[13];
};

struct fruit_img_t {
    uint32_t width, height;
    uint32_t *bmp[8];
};

struct ghost_img_t {
    uint32_t width, height;
    uint32_t *bmp[4][4];
};

struct ghost_counter_img_t {
    uint32_t width, height;
    uint32_t *bmp[4];
};

// Forward declarations
class controls;
class players;
class enemies;
class entityPacman;
class entityGhost;
class entityBlinky;
class entityFruit;

class game
{
public:
    enum GameMode {
        GAMEMODE_ATTRACT = 0,
        GAMEMODE_READY,
        GAMEMODE_PLAYING,
        GAMEMODE_DEAD,
        GAMEMODE_LEVEL_COMPLETE,
        GAMEMODE_GAMEOVER
    };

    game();
    ~game();

    void init();
    void run();
    void draw();

    // Subsystems (public for entity access, matching opengw pattern)
    sound      mSound;
    controls*  mControls;
    players*   mPlayers;
    enemies*   mEnemies;
    entityFruit* mFruit;

    // Accessors for entities
    entityPacman* getPacman();
    entityBlinky* getBlinky();
    entityGhost*  getGhost(int index);

    // Public game state (entities need read/write access)
    uint8_t  mMap[MAZE_ROWS][MAZE_COLS];
    uint32_t mGlobalMode;       // 0=scatter, 1=chase
    uint32_t mEnergizerTimer;
    uint32_t mScore;
    uint32_t mHighScore;
    uint32_t mLevel;
    uint32_t mLives;
    uint32_t mFoodEaten;
    uint32_t mGhostFrame;
    uint32_t mBoundary;
    uint32_t mCornering;
    uint32_t mFrameCounter;
    uint32_t mStateTimer;

    static GameMode mGameMode;

    // Sprite tables (public static for entity self-drawing)
    static ghost_img_t         sGhostImg;
    static ghost_img_t         sGhostDeadImg;
    static ghost_img_t         sGhostScaredImg;
    static pacman_img_t        sPacmanImg;
    static pacdead_img_t       sPacDeadImg;
    static fruit_img_t         sFruitImg;
    static ghost_counter_img_t sGhostCounterImg;

    // Map / coordinate helpers (public for entity use)
    int32_t wrapMapX(int32_t x);
    int32_t wrapMapY(int32_t y);
    int32_t wrap(int32_t x);
    int32_t limit(int32_t y);
    bool    inTunnel(uint32_t x, uint32_t y);
    int32_t xDir(uint32_t a);
    int32_t yDir(uint32_t a);

    // Drawing utilities (public for potential entity use)
    void drawBitmap(int32_t px, int32_t py, uint32_t* list, uint32_t width, uint32_t height);
    void drawStr(int32_t px, int32_t py, const char* str);
    void drawColorStr(int32_t px, int32_t py, const char* str, uint32_t color);
    int32_t levelToFruitIndex(uint32_t level);

    static FruitInfo sFruitTable[];

private:
    void initLevel(bool fullReset);
    void runFrame();
    void eatFood();
    void checkCollisions();

    // Rendering
    void drawBackground();
    void drawDot(int32_t px, int32_t py);
    void drawDotsAndEnergizers();
    void drawScore();
    void drawLives();
    void drawLevel();
    void drawReadyText();

    // Bitmap compositing buffer
    uint32_t mBmpBuf[32 * 8 * 8];
};

#endif // GAME_H
