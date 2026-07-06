#ifndef GAME_H
#define GAME_H

#include "Player.h"
#include "Bullet.h"
#include "Enemy.h"
#include "Display.h"
#include "GameTypes.h"
#include "Boss.h"
#include <vector>
#include <memory>
#include <fstream>
#include <string>

// Main game class
class Game {
private:
    Display display;
    Player player;
    std::vector<std::unique_ptr<Bullet>> bullets;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::unique_ptr<Boss> boss;

    // Game State
    GameState currentState;
    Difficulty currentDifficulty;
    int selectedButton; // For menu navigation

    int score;
    int enemySpawnTimer;
    bool autoShoot;      // Auto shoot flag
    int shootCooldown;   // Shoot cooldown time

    // Difficulty Parameters (derived from config)
    int minSpawnInterval;   // min frames between spawn waves
    int maxSpawnInterval;   // max frames between spawn waves
    int maxEnemies;         // max concurrent enemies
    int spawnCount;         // enemies per wave
    int enemyMoveInterval;  // enemy move interval (frames)

    // Player stats & difficulty config
    int playerLives;        // remaining lives in current run
    int shootCooldownBase;  // auto-shoot cooldown base
    int scorePerKill;       // score per kill
    int bulletDamage;       // current bullet damage

    // Boss battle control
    bool bossActive;
    int frameCounter;       // frames since game start
    int bossSpawnFrame;     // when to spawn boss (in frames)

    DifficultyConfig currentConfig;

    // Stats (Per Difficulty)
    std::vector<int> leaderboard; // Stores all historical scores
    int enemiesDefeated;          // Current game
    int bossDamageDealtForScore;  // Cumulative damage dealt to boss for scoring
    
    // Private methods
    void resetGame();
    void spawnEnemy();
    void checkCollisions();
    void clearGameStats(Difficulty diff); // Add this method declaration
    int getDifficultyHealthBonus() const; // Calculate health bonus based on damage
    
    // Input Handling per state
    void handleInput();
    void handleInputMenu();
    void handleInputDifficulty();
    void handleInputPlaying();
    void handleInputGameOver();
    void handleInputStats();
    void handleInputWin();

    // Update per state
    void update();
    
    // Draw
    void draw();

    void loadGameStats(Difficulty diff);
    void saveGameStats(Difficulty diff);
    std::string getStatsFileName(Difficulty diff);
    DifficultyConfig getConfigForDifficulty(Difficulty diff) const;
    
public:
    // Constructor
    Game();
    
    // Destructor
    ~Game();
    
    // Initialize game
    void init();
    
    // Main game loop
    void gameLoop();
    
    bool isRunning() const { return true; } // Controlled by internal state logic
};

#endif
