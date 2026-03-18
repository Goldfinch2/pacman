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
enum { DIR_RIGHT, DIR_LEFT, DIR_UP, DIR_DOWN };

// Ghost IDs
enum { BLINKY, PINKY, INKY, CLYDE, MAX_GHOSTS };

// Tile types
enum { TILE_EMPTY, TILE_WALL, TILE_FOOD, TILE_ENERGIZER };

// Fruit types
enum { CHERRY, STRAWBERRY, PEAR, APPLE, JACK, EAGLE, BELL, KEY };

// Ghost modes
enum { GHOST_NORMAL, GHOST_DEAD, GHOST_RELEASED, GHOST_TRAPPED };

struct Ghost {
    int32_t x, y;
    uint32_t dir;
    uint32_t mode;
    bool scared;
    uint32_t speed;
};

struct FruitInfo {
    uint32_t min_level, max_level;
    uint32_t fruit;
    uint32_t score;
    uint32_t width, height;
    uint32_t* score_bmp;
};

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

    sound mSound;

    static GameMode mGameMode;

private:
    // Game state
    Ghost mGhosts[MAX_GHOSTS];
    uint8_t mMap[MAZE_ROWS][MAZE_COLS];
    int32_t mPacX, mPacY;
    uint32_t mPacDir;
    uint32_t mPacFrame;
    uint32_t mFoodEaten;
    int32_t  mPacDead;
    uint32_t mPacEatGhost;
    uint32_t mPacGhostCounter;
    uint32_t mGlobalMode;
    bool mCherry;
    uint32_t mCherryTimer;
    uint32_t mEnergizerTimer;
    uint32_t mScore;
    uint32_t mHighScore;
    uint32_t mLevel;
    uint32_t mLives;
    uint32_t mGhostFrame;
    uint32_t mBoundary;
    uint32_t mCornering;
    uint32_t mStateTimer;
    uint32_t mFrameCounter;

    // Bitmap compositing buffer
    uint32_t mBmpBuf[32 * 8 * 8];

    // Private methods
    void initLevel(bool fullReset);
    void initGhost(uint32_t ghost, uint32_t x, uint32_t y, uint32_t dir, uint32_t mode, bool scared, uint32_t speed);
    void resetGhost(uint32_t ghost, uint32_t time);

    void runFrame();
    void processGhost(uint32_t ghost);
    void frighten(uint32_t ghost);
    void reverseDirection(uint32_t ghost);
    void eatFood();

    // Rendering
    void drawBitmap(int32_t px, int32_t py, uint32_t* list, uint32_t width, uint32_t height);
    void drawBackground();
    void drawDot(int32_t px, int32_t py);
    void drawGhost(uint32_t ghost);
    void drawScore();
    void drawLives();
    void drawLevel();
    void drawStr(int32_t px, int32_t py, const char* str);
    void drawColorStr(int32_t px, int32_t py, const char* str, uint32_t color);
    void drawReadyText();

    // Helpers
    int32_t wrapMapX(int32_t x);
    int32_t wrapMapY(int32_t y);
    int32_t wrap(int32_t x);
    int32_t limit(int32_t y);
    bool inTunnel(uint32_t x, uint32_t y);
    void snapX();
    void snapY();
    int32_t xDir(uint32_t a);
    int32_t yDir(uint32_t a);
    int32_t levelToFruitIndex(uint32_t level);

    // Input
    bool mKeyRight, mKeyLeft, mKeyUp, mKeyDown;
    bool mKeyStart, mKeyCoin;
    void readInput();
};

extern game theGame;

#endif // GAME_H
