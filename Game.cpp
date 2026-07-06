#include "Game.h"
#include <ncurses.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>

/*
 * Purpose: Initialize overall game state, defaults, and timers.
 * Inputs: None.
 * Returns: Game instance ready for initialization.
 */
Game::Game() :
    currentState(GameState::START_MENU),
    currentDifficulty(Difficulty::EASY),
    selectedButton(0),
    score(0),
    enemySpawnTimer(0),
    autoShoot(false),
    shootCooldown(0),
    minSpawnInterval(120),
    maxSpawnInterval(120),
    maxEnemies(1),
    spawnCount(1),
    enemyMoveInterval(2),
    playerLives(1),
    shootCooldownBase(5),
    scorePerKill(10),
    bulletDamage(1),
    bossActive(false),
    frameCounter(0),
    bossSpawnFrame(1500),
    enemiesDefeated(0),
    bossDamageDealtForScore(0) { srand(time(nullptr)); }

/*
 * Purpose: Provide explicit destructor even though smart pointers own resources.
 * Inputs: None.
 * Returns: Nothing.
 */
Game::~Game() {
    // unique_ptr automatically manages memory
}

/*
 * Purpose: Build a difficulty configuration describing spawn pace and rewards.
 * Inputs: diff - difficulty level requested.
 * Returns: DifficultyConfig instance with tuned values.
 */
DifficultyConfig Game::getConfigForDifficulty(Difficulty diff) const {
    DifficultyConfig cfg;
    switch (diff) {
        case Difficulty::EASY:
            cfg.minSpawnInterval = 50;
            cfg.maxSpawnInterval = 70;
            cfg.maxEnemies = 15;     // Increased to allow multiple waves on screen
            cfg.spawnCount = 1;
            cfg.enemyMoveInterval = 4;
            cfg.playerLives = 5;
            cfg.shootCooldownBase = 4;
            cfg.scorePerKill = 10;
            break;
        case Difficulty::MEDIUM:
            cfg.minSpawnInterval = 30;
            cfg.maxSpawnInterval = 50;
            cfg.maxEnemies = 25;     // Increased to allow multiple waves on screen
            cfg.spawnCount = 2;
            cfg.enemyMoveInterval = 3;
            cfg.playerLives = 4;
            cfg.shootCooldownBase = 5;
            cfg.scorePerKill = 15;
            break;
        case Difficulty::HARD:
            cfg.minSpawnInterval = 15;
            cfg.maxSpawnInterval = 25;
            cfg.maxEnemies = 40;    // Increased to allow multiple waves on screen
            cfg.spawnCount = 3;
            cfg.enemyMoveInterval = 2;
            cfg.playerLives = 3;
            cfg.shootCooldownBase = 6;
            cfg.scorePerKill = 20;
            break;
    }
    return cfg;
}

/*
 * Purpose: Map a difficulty value to its corresponding stats file.
 * Inputs: diff - difficulty for which to fetch persistent scores.
 * Returns: File name containing stored scores.
 */
std::string Game::getStatsFileName(Difficulty diff) {
    switch(diff) {
        case Difficulty::EASY: return "stats_easy.txt";
        case Difficulty::MEDIUM: return "stats_medium.txt";
        case Difficulty::HARD: return "stats_hard.txt";
        default: return "stats_easy.txt";
    }
}

/*
 * Purpose: Load the leaderboard for the selected difficulty.
 * Inputs: diff - difficulty whose leaderboard should be loaded.
 * Returns: Nothing; populates the leaderboard vector.
 */
void Game::loadGameStats(Difficulty diff) {
    leaderboard.clear();
    std::ifstream inFile(getStatsFileName(diff));
    if (inFile.is_open()) {
        int s;
        while (inFile >> s) {
            leaderboard.push_back(s);
        }
        inFile.close();
    }
    // Sort descending
    std::sort(leaderboard.begin(), leaderboard.end(), std::greater<int>());
}

/*
 * Purpose: Persist the latest score to the leaderboard file.
 * Inputs: diff - difficulty used during the completed run.
 * Returns: Nothing.
 */
void Game::saveGameStats(Difficulty diff) {
    // Add current score to leaderboard
    leaderboard.push_back(score);
    
    // Sort descending
    std::sort(leaderboard.begin(), leaderboard.end(), std::greater<int>());

    std::ofstream outFile(getStatsFileName(diff));
    if (outFile.is_open()) {
        for (int s : leaderboard) {
            outFile << s << std::endl;
        }
        outFile.close();
    }
}

/*
 * Purpose: Wipe the leaderboard file for a given difficulty.
 * Inputs: diff - difficulty to clear.
 * Returns: Nothing.
 */
void Game::clearGameStats(Difficulty diff) {
    leaderboard.clear();
    std::ofstream outFile(getStatsFileName(diff), std::ofstream::trunc); // Open in truncate mode to clear content
    if (outFile.is_open()) {
        outFile.close();
    }
}

/*
 * Purpose: Initialize display resources before entering the game loop.
 * Inputs: None.
 * Returns: Nothing.
 */
void Game::init() {
    display.init();
}

/*
 * Purpose: Reset gameplay variables for a fresh session.
 * Inputs: None.
 * Returns: Nothing.
 */
void Game::resetGame() {
    score = 0;
    enemiesDefeated = 0;
    enemySpawnTimer = 0;
    autoShoot = false;
    shootCooldown = 0;
    bulletDamage = 1;
    bossActive = false;
    frameCounter = 0;
    bossDamageDealtForScore = 0;

    // Clear entities
    bullets.clear();
    enemies.clear();
    boss.reset();
    
    // Reset player position
    player = Player();
    
    // Load stats for current difficulty
    loadGameStats(currentDifficulty);
}

/*
 * Purpose: Provide a health bonus derived from the player's bullet upgrades.
 * Inputs: None.
 * Returns: Bonus health value applied to certain obstacles.
 */
int Game::getDifficultyHealthBonus() const {
    int damageIncrease = bulletDamage - 1;
    if (damageIncrease <= 0) return 0;
    return (damageIncrease / 5) * 6;
}

/*
 * Purpose: Spawn a new wave of enemies when conditions allow.
 * Inputs: None.
 * Returns: Nothing.
 */
void Game::spawnEnemy() {
    if (enemySpawnTimer > 0) {
        enemySpawnTimer--;
        return;
    }

    // Count active enemies
    int activeEnemies = 0;
    for (const auto& e : enemies) {
        if (e->isActiveState()) activeEnemies++;
    }
    if (activeEnemies >= maxEnemies) {
        return;
    }

    // Spawn a wave of enemies
    // Determine spawn range
    int minSpawn = 1; 
    int maxSpawn = 1;
    
    // Base slowest speed interval (EASY mode default)
    // Let's use the EASY mode speed (4 frames per move) as the global reference for "slowest".
    const float BASE_SLOWEST_INTERVAL = 4.0f; 

    switch (currentDifficulty) {
        case Difficulty::EASY:
            minSpawn = 2;
            maxSpawn = 4;
            break;
        case Difficulty::MEDIUM:
            minSpawn = 4;
            maxSpawn = 6;
            break;
        case Difficulty::HARD:
            minSpawn = 6;
            maxSpawn = 8;
            break;
    }

    // Randomize spawn count for this wave
    int currentSpawnCount = minSpawn + (rand() % (maxSpawn - minSpawn + 1));
    
    int spawned = 0;
    for (int i = 0; i < currentSpawnCount; ++i) {
        if (activeEnemies >= maxEnemies) break;

        // Try to find a non-overlapping position (up to 10 attempts)
        int x = 1;
        bool validPos = false;
        for (int attempt = 0; attempt < 10; ++attempt) {
            x = rand() % (WINDOW_WIDTH - 2) + 1;
            // Check overlap with existing enemies (assuming spawn y = -2, width = 3)
            bool overlap = false;
            int newW = 3; 
            int newH = 2; 
            int newY = -2;
            
            for (const auto& ex : enemies) {
                if (!ex->isActiveState()) continue;
                // Check AABB intersection
                // (x1 < x2 + w2) && (x1 + w1 > x2) && (y1 < y2 + h2) && (y1 + h1 > y2)
                if (x < ex->getX() + ex->getWidth() &&
                    x + newW > ex->getX() &&
                    newY < ex->getY() + ex->getHeight() &&
                    newY + newH > ex->getY()) {
                    overlap = true;
                    break;
                }
            }
            
            if (!overlap) {
                validPos = true;
                break;
            }
        }

        if (!validPos) continue; // Skip this spawn if no valid position found

        auto enemy = std::make_unique<Enemy>(x, -2);
        
        // Discrete speed selection based on difficulty
        float speedMult = 1.0f;
        bool coinFlip = (rand() % 2 == 0); // 50% chance

        switch (currentDifficulty) {
            case Difficulty::EASY:
                // 1.0f or 1.5f
                speedMult = coinFlip ? 1.0f : 1.5f;
                break;
            case Difficulty::MEDIUM:
                // 1.5f or 2.5f
                speedMult = coinFlip ? 1.5f : 2.0f;
                break;
            case Difficulty::HARD:
                // 2.0f or 3.0f
                speedMult = coinFlip ? 2.0f : 2.5f;
                break;
        }
        
        // Convert speed multiplier to interval (Interval = Base / Multiplier)
        // Higher multiplier = Faster speed = Lower interval
        // We use float for precision
        float finalInterval = std::max(0.1f, BASE_SLOWEST_INTERVAL / speedMult);
        
        enemy->setMoveInterval(finalInterval);

        // Decide obstacle type & values based on difficulty
        ObstacleType t = ObstacleType::SOLID;
        int hp = 1;
        int value = 0;
        int r = rand() % 100;
        int healthBonus = getDifficultyHealthBonus();
        
        if (currentDifficulty == Difficulty::EASY) {
            if (r < 45) {
                t = ObstacleType::SOLID;
                hp = 1; // Solid blocks usually don't scale or scale differently, keeping as 1 for now
            } else if (r < 75) {
                t = ObstacleType::BREAKABLE;
                hp = 2 + healthBonus;
            } else if (r < 85) {
                t = ObstacleType::EXPLODE;
                hp = 1 + healthBonus;
            } else {
                t = ObstacleType::DAMAGE_BUFF;
                value = -2;
            }
        } else if (currentDifficulty == Difficulty::MEDIUM) {
            if (r < 35) {
                t = ObstacleType::SOLID;
                hp = 1;
            } else if (r < 70) {
                t = ObstacleType::BREAKABLE;
                hp = 3 + healthBonus;
            } else {
                t = ObstacleType::DAMAGE_BUFF;
                value = -3;
            }
        } else { // HARD
            if (r < 30) {
                t = ObstacleType::SOLID;
                hp = 1;
            } else if (r < 60) {
                t = ObstacleType::BREAKABLE;
                hp = 4 + healthBonus;
            } else if (r < 70) {
                t = ObstacleType::EXPLODE;
                hp = 1 + healthBonus;
            } else {
                t = ObstacleType::DAMAGE_BUFF;
                value = -4;
            }
        }
        enemy->setType(t);
        enemy->setHealth(hp);
        enemy->setValue(value);

        enemies.push_back(std::move(enemy));

        activeEnemies++;
        spawned++;
    }

    if (spawned > 0) {
        // Randomize next spawn time
        int interval = minSpawnInterval + (rand() % (maxSpawnInterval - minSpawnInterval + 1));
        enemySpawnTimer = interval;
    }
}

/*
 * Purpose: Dispatch input handling to state-specific handlers.
 * Inputs: None.
 * Returns: Nothing.
 */
void Game::handleInput() {
    switch (currentState) {
        case GameState::START_MENU:
            handleInputMenu();
            break;
        case GameState::DIFFICULTY_SELECT:
            handleInputDifficulty();
            break;
        case GameState::PLAYING:
            handleInputPlaying();
            break;
        case GameState::WIN:
            handleInputWin();
            break;
        case GameState::GAME_OVER:
            handleInputGameOver();
            break;
        case GameState::STATS_VIEW:
            handleInputStats();
            break;
    }
}

/*
 * Purpose: Process input while the start menu is visible.
 * Inputs: None.
 * Returns: Nothing.
 */
void Game::handleInputMenu() {
    int ch = getch();
    if (ch == ' ' || ch == '\n' || ch == KEY_ENTER) {
        currentState = GameState::DIFFICULTY_SELECT;
        selectedButton = 0; // Default to first button
    } else if (ch == 'q' || ch == 'Q') {
        endwin();
        exit(0);
    }
}

/*
 * Purpose: Process input on the difficulty selection screen.
 * Inputs: None.
 * Returns: Nothing.
 */
void Game::handleInputDifficulty() {
    int ch = getch();
    switch(ch) {
        case 'w': case 'W': case KEY_UP:
        case 'a': case 'A': case KEY_LEFT:
            selectedButton = (selectedButton - 1 + 4) % 4;
            break;
        case 's': case 'S': case KEY_DOWN:
        case 'd': case 'D': case KEY_RIGHT:
            selectedButton = (selectedButton + 1) % 4;
            break;
        case ' ': case '\n': case KEY_ENTER:
            // Confirm selection
            if (selectedButton == 0) {
                currentDifficulty = Difficulty::EASY;
            } else if (selectedButton == 1) {
                currentDifficulty = Difficulty::MEDIUM;
            } else if (selectedButton == 2) {
                currentDifficulty = Difficulty::HARD;
            } else {
                // Quit
                endwin();
                exit(0);
            }

            // Apply difficulty config
            currentConfig = getConfigForDifficulty(currentDifficulty);
            minSpawnInterval = currentConfig.minSpawnInterval;
            maxSpawnInterval = currentConfig.maxSpawnInterval;
            maxEnemies = currentConfig.maxEnemies;
            spawnCount = currentConfig.spawnCount;
            enemyMoveInterval = currentConfig.enemyMoveInterval;
            playerLives = currentConfig.playerLives;
            shootCooldownBase = currentConfig.shootCooldownBase;
            scorePerKill = currentConfig.scorePerKill;

            // Set boss spawn timing per difficulty
            switch (currentDifficulty) {
                case Difficulty::EASY:
                    // 20 seconds at ~20 FPS => ~400 frames
                    bossSpawnFrame = 400;
                    break;
                case Difficulty::MEDIUM:
                case Difficulty::HARD:
                    // 30 seconds at ~20 FPS => ~600 frames
                    bossSpawnFrame = 600;
                    break;
            }

            resetGame();
            currentState = GameState::PLAYING;
            break;
        case 'q': case 'Q':
            endwin();
            exit(0);
            break;
    }
}

/*
 * Purpose: Process gameplay controls including movement and auto-shoot toggling.
 * Inputs: None.
 * Returns: Nothing.
 */
void Game::handleInputPlaying() {
    int ch = getch();

    // Handle multiple keys, allow shooting while moving
    while (ch != ERR) {
        switch(ch) {
            case 'a':
            case 'A':
            case KEY_LEFT:
                player.moveLeft();
                break;
            case 'd':
            case 'D':
            case KEY_RIGHT:
                player.moveRight();
                break;
            case 'w':
            case 'W':
            case KEY_UP:
                player.moveUp();
                break;
            case 's':
            case 'S':
            case KEY_DOWN:
                player.moveDown();
                break;
            case ' ':
                // Space key: toggle auto shoot
                autoShoot = !autoShoot;
                break;
            case 'q':
            case 'Q':
                // Quit to menu? Or exit app?
                // Let's quit to main menu
                currentState = GameState::START_MENU;
                return;
        }
        ch = getch();  // Continue reading next key
    }
    
    // If auto shoot is enabled and cooldown is ready
    if (autoShoot && shootCooldown <= 0) {
        bullets.push_back(std::make_unique<Bullet>(player.getX(), player.getY() - 1));
        shootCooldown = shootCooldownBase;  // difficulty-dependent cooldown
    }
    
    if (shootCooldown > 0) {
        shootCooldown--;
    }
}

/*
 * Purpose: Handle inputs on the game-over screen.
 * Inputs: None.
 * Returns: Nothing.
 */
void Game::handleInputGameOver() {
    int ch = getch();
    if (ch == 'x' || ch == 'X') {
        // Go to stats view
        // Save stats first (handled in update/transition logic usually, but here ok)
        saveGameStats(currentDifficulty);
        currentState = GameState::STATS_VIEW;
    }
}

/*
 * Purpose: Process navigation within the stats screen.
 * Inputs: None.
 * Returns: Nothing.
 */
void Game::handleInputStats() {
    int ch = getch();
    switch(ch) {
        case 'a': case 'A': case KEY_LEFT:
            selectedButton = (selectedButton - 1 + 2) % 2;
            break;
        case 'd': case 'D': case KEY_RIGHT:
            selectedButton = (selectedButton + 1) % 2;
            break;
        case ' ': case '\n': case KEY_ENTER:
            if (selectedButton == 0) {
                // Back button
                currentState = GameState::DIFFICULTY_SELECT;
                selectedButton = 0;
            } else {
                // Clean button
                clearGameStats(currentDifficulty);
                // Don't exit state, just refresh (draw will show empty list)
            }
            break;
    }
}

/*
 * Purpose: Handle input when the player wins.
 * Inputs: None.
 * Returns: Nothing.
 */
void Game::handleInputWin() {
    int ch = getch();
    if (ch == 'x' || ch == 'X') {
        // 可选：胜利时也记录成绩
        saveGameStats(currentDifficulty);
        currentState = GameState::STATS_VIEW;
    }
}

/*
 * Purpose: Resolve all collision scenarios between bullets, enemies, boss, and player.
 * Inputs: None.
 * Returns: Nothing; may modify game state or transition screens.
 */
void Game::checkCollisions() {
    // Bullet hits enemy (check if hits any part of obstacle)
    for (auto& bullet : bullets) {
        if (!bullet->isActiveState()) continue;
        if (!bullet->isFromPlayer()) continue; // 敌方/Boss 子弹不打敌人
        
        for (auto& enemy : enemies) {
            if (!enemy->isActiveState()) continue;
            
            int enemyWidth = enemy->getWidth();
            int bulletX = bullet->getX();
            int bulletY = bullet->getY();
            int enemyX = enemy->getX();
            int enemyY = enemy->getY();
            
            // Check if bullet hits any position of obstacle
            if (bulletY >= enemyY && bulletY <= enemyY + 1 && bulletX >= enemyX && bulletX < enemyX + enemyWidth) {
                
                ObstacleType type = enemy->getType();
                bool handled = false;
                
                switch (type) {
                    case ObstacleType::SOLID:
                        bullet->setActive(false);
                        handled = true;
                        break;
                    case ObstacleType::BREAKABLE:
                        bullet->setActive(false);
                        enemy->damage(bulletDamage);
                        if (enemy->getHealth() <= 0) {
                            enemy->setActive(false);
                            score += scorePerKill;
                            enemiesDefeated++;
                        }
                        handled = true;
                        break;
                    case ObstacleType::EXPLODE:
                        bullet->setActive(false);
                        enemy->damage(bulletDamage);
                        if (enemy->getHealth() <= 0) {
                            enemy->setActive(false);
                            score += scorePerKill;
                            enemiesDefeated++;
                            
                            // Explosion logic
                            for (auto& other : enemies) {
                                if (other.get() == enemy.get()) continue;
                                if (!other->isActiveState()) continue;
                                
                                int dist = std::abs(other->getX() - enemyX) + std::abs(other->getY() - enemyY);
                                if (dist <= 10) {
                                    other->setActive(false);
                                    score += scorePerKill;
                                    enemiesDefeated++;
                                }
                            }
                        }
                        handled = true;
                        break;
                    case ObstacleType::DAMAGE_BUFF:
                        enemy->setValue(std::min(enemy->getValue() + 1, 9));
                        break;
                }
                
                if (handled) {
                    break;
                } else {
                    // Bullet continues to next enemy
                    continue;
                }
            }
        }
    }

    // Player bullets hit Boss
    if (bossActive && boss) {
        for (auto& bullet : bullets) {
            if (!bullet->isActiveState()) continue;
            if (!bullet->isFromPlayer()) continue;

            int bx = bullet->getX();
            int by = bullet->getY();

            if (boss->checkCollision(bx, by)) {
                bullet->setActive(false);
                // 先记录当前血量
                int before = boss->getHealth();
                boss->takeDamage(bulletDamage);
                int after = boss->getHealth();

                // 计算本次实际造成的伤害（防止出现负数）
                int delta = std::max(0, before - after);

                // 按累计伤害每 100 点加 2 分
                int oldBucket = bossDamageDealtForScore / 100;
                bossDamageDealtForScore += delta;
                int newBucket = bossDamageDealtForScore / 100;
                if (newBucket > oldBucket) {
                    score += (newBucket - oldBucket) * 2;
                }
            }
        }

        // Handle boss death: clear enemy bullets and go to WIN screen
        if (boss->isDead()) {
            bossActive = false;

            // Clear all non-player bullets
            for (auto& bullet : bullets) {
                if (!bullet->isFromPlayer()) {
                    bullet->setActive(false);
                }
            }

            // Switch to WIN screen
            currentState = GameState::WIN;
            return;
        }
    }

    // Player hit by enemy/Boss bullets
    for (auto& bullet : bullets) {
        if (!bullet->isActiveState()) continue;
        if (bullet->isFromPlayer()) continue;

        int bx = bullet->getX();
        int by = bullet->getY();

        int playerX = player.getX();
        int playerY = player.getY();

        bool collision = false;

        // Tip
        if (bx == playerX && by == playerY) {
            collision = true;
        }
        // Wings (y+1)
        else if (by == playerY + 1 &&
                 (bx == playerX - 1 || bx == playerX || bx == playerX + 1)) {
            collision = true;
        }

        if (collision) {
            if (playerLives > 1) {
                playerLives--;
                bullets.clear();
                enemies.clear();
                enemySpawnTimer = 0;
                player = Player();
            } else {
                currentState = GameState::GAME_OVER;
            }
            return;
        }
    }
    
    // Player collides with enemy (check if hits any part of obstacle)
    for (const auto& enemy : enemies) {
        if (!enemy->isActiveState()) continue;
        
        int enemyWidth = enemy->getWidth();
        int playerX = player.getX();
        int playerY = player.getY();
        int enemyX = enemy->getX();
        int enemyY = enemy->getY();
        
        // Check if player hits any position of obstacle
        // Player shape:
        //   ^    (x, y)
        //  /|\   (x-1, y+1), (x, y+1), (x+1, y+1)
        bool collision = false;
        
        // Check tip (x, y)
        if (playerY >= enemyY && playerY <= enemyY + 1 && playerX >= enemyX && playerX < enemyX + enemyWidth) {
            collision = true;
        }
        // Check wings (y+1)
        else if (playerY + 1 >= enemyY && playerY + 1 <= enemyY + 1) {
             // Left wing (x-1)
             if (playerX - 1 >= enemyX && playerX - 1 < enemyX + enemyWidth) collision = true;
             // Center (x)
             else if (playerX >= enemyX && playerX < enemyX + enemyWidth) collision = true;
             // Right wing (x+1)
             else if (playerX + 1 >= enemyX && playerX + 1 < enemyX + enemyWidth) collision = true;
        }

        if (collision) {

            ObstacleType type = enemy->getType();
            if (type == ObstacleType::DAMAGE_BUFF && enemy->getValue() > 0) {
                bulletDamage += enemy->getValue();
                enemy->setActive(false);
                continue;
            }

            if (playerLives > 1) {
                // Lose one life, reset wave but continue game
                playerLives--;

                bullets.clear();
                enemies.clear();
                enemySpawnTimer = 0;
                player = Player();   // reset position
            } else {
                // No lives left -> game over
                currentState = GameState::GAME_OVER;
            }
            return;
        }
    }
    return;
}

/*
 * Purpose: Advance the simulation by one frame when playing.
 * Inputs: None.
 * Returns: Nothing.
 */
void Game::update() {
    if (currentState != GameState::PLAYING) return;

    // Advance frame counter (for boss timing)
    frameCounter++;

    // Spawn boss after certain time
    if (!bossActive && frameCounter >= bossSpawnFrame) {
        boss = std::make_unique<Boss>(currentDifficulty);
        bossActive = true;
    }

    // Update bullets
    for (auto& bullet : bullets) {
        if (bullet->isActiveState()) {
            bullet->update();
        }
    }
    
    // Remove inactive bullets
    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
            [](const std::unique_ptr<Bullet>& b) { return !b->isActiveState(); }),
        bullets.end()
    );
    
    // Update enemies
    // Sort by Y (bottom-up) to process lower enemies first (though for falling, logic can handle any order if checking next pos)
    // Actually, simple collision check: if enemy moves to next position, does it hit another enemy?
    for (auto& enemy : enemies) {
        if (!enemy->isActiveState()) continue;
        
        // Tick timer
        enemy->tickTimer();
        
        if (enemy->isReadyToMove()) {
            // Check if movement would collide with another enemy
            int nextY = enemy->getY() + 1; // moving down 1 unit
            int x = enemy->getX();
            int w = enemy->getWidth();
            int h = enemy->getHeight();
            
            bool blocked = false;
            for (const auto& other : enemies) {
                if (other.get() == enemy.get()) continue;
                if (!other->isActiveState()) continue;
                
                // Check intersection with other enemy at nextY
                if (x < other->getX() + other->getWidth() &&
                    x + w > other->getX() &&
                    nextY < other->getY() + other->getHeight() &&
                    nextY + h > other->getY()) {
                    
                    // There is an obstacle below/intersecting
                    // Check if that obstacle is actually below us (to block movement)
                    // If it's overlapping because it spawned on top (shouldn't happen now) or other weirdness
                    // Basically, if we move into it, we are blocked.
                    blocked = true;
                    break;
                }
            }
            
            if (!blocked) {
                enemy->performMove();
            } else {
                enemy->stall();
            }
        }
    }
    
    // Remove inactive enemies
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const std::unique_ptr<Enemy>& e) { return !e->isActiveState(); }),
        enemies.end()
    );
    
    // Spawn new enemies (stop normal waves after boss appears)
    if (!bossActive) {
        spawnEnemy();
    }
    
    // Update boss (if active)
    if (bossActive && boss && !boss->isDead()) {
        boss->update(bullets);
    }

    // Check collisions
    checkCollisions();
    return;
}

/*
 * Purpose: Delegate rendering to the Display using the latest state.
 * Inputs: None.
 * Returns: Nothing.
 */
void Game::draw() {
    display.draw(currentState, 
        player, 
        bullets, 
        enemies,
        boss.get(),
        score, 
        enemiesDefeated, 
        playerLives, 
        bulletDamage, 
        currentDifficulty, 
        autoShoot, 
        selectedButton, 
        leaderboard
    );
    return;
}

/*
 * Purpose: Run the primary loop that handles input, updates, draws, and timing.
 * Inputs: None.
 * Returns: Nothing.
 */
void Game::gameLoop() {
    init();
    
    while (true) {
        handleInput();
        update();
        draw();
        usleep(50000); // 50ms delay
    }
    
    // Cleanup handled by destructor
    display.cleanup();
    return;
}
