/*
 * File: Display.h
 * Purpose: Declares the Display class that encapsulates all ncurses drawing logic for states, HUD, and entities.
 */
#ifndef DISPLAY_H
#define DISPLAY_H

#include <ncurses.h>
#include <vector>
#include <string>
#include <memory>
#include "GameTypes.h"
#include "Player.h"
#include "Bullet.h"
#include "Enemy.h"
#include "Boss.h"

class Display {
public:
    Display();
    ~Display();

    // Initialize ncurses
    void init();
    
    // Cleanup ncurses
    void cleanup();

    // Main draw function that dispatches based on state
    void draw(GameState currentState, 
              const Player& player, 
              const std::vector<std::unique_ptr<Bullet>>& bullets,
              const std::vector<std::unique_ptr<Enemy>>& enemies,
              const Boss* boss,
              int score, 
              int enemiesDefeated, 
              int playerLives, 
              int bulletDamage, 
              Difficulty currentDifficulty, 
              bool autoShoot,
              int selectedButton,
              const std::vector<int>& leaderboard);

private:
    int borderTop = 0;
    int borderLeft = 0;
    int borderBottom = 0;
    int borderRight = 0;

    void updateLayout();
    void drawGameBorder() const;

    void drawMenu();
    void drawDifficultySelect(int selectedButton);
    void drawPlaying(const Player& player, 
                     const std::vector<std::unique_ptr<Bullet>>& bullets,
                     const std::vector<std::unique_ptr<Enemy>>& enemies,
                     const Boss* boss,
                     int score, 
                     int enemiesDefeated, 
                     int playerLives, 
                     int bulletDamage, 
                     Difficulty currentDifficulty, 
                     bool autoShoot);
    void drawGameOver();
    void drawWin();
    void drawStats(Difficulty currentDifficulty, 
                   const std::vector<int>& leaderboard, 
                   int score, 
                   int selectedButton);
};

#endif

