#ifndef GAMETYPES_H
#define GAMETYPES_H

// Game window dimensions
const int WINDOW_WIDTH = 50;
const int WINDOW_HEIGHT = 40;

// Game States
enum class GameState {
    START_MENU,
    DIFFICULTY_SELECT,
    PLAYING,
    WIN,
    GAME_OVER,
    STATS_VIEW
};

// Difficulty Levels
enum class Difficulty {
    EASY,
    MEDIUM,
    HARD
};

// Difficulty configuration
struct DifficultyConfig {
    int minSpawnInterval;   // min frames between spawn waves
    int maxSpawnInterval;   // max frames between spawn waves
    int maxEnemies;         // max enemies on screen
    int spawnCount;         // enemies per spawn wave
    int enemyMoveInterval;  // enemy move interval (frames)
    int playerLives;        // starting lives
    int shootCooldownBase;  // auto-shoot cooldown base (frames)
    int scorePerKill;       // score per enemy killed
};

struct RenderOffsets {
    int x;
    int y;
};

extern RenderOffsets gRenderOffsets;

/*
 * Purpose: Convert a local X coordinate inside the game window to an absolute ncurses coordinate.
 * Inputs: localX - column inside the logical game window.
 * Returns: Absolute screen column relative to the terminal origin.
 */
inline int translateX(int localX) {
    return gRenderOffsets.x + localX;
}

/*
 * Purpose: Convert a local Y coordinate inside the game window to an absolute ncurses coordinate.
 * Inputs: localY - row inside the logical game window.
 * Returns: Absolute screen row relative to the terminal origin.
 */
inline int translateY(int localY) {
    return gRenderOffsets.y + localY;
}

#endif

