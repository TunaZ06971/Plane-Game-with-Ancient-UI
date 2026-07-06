/*
 * File: Boss.cpp
 * Purpose: Implements the boss ship logic, including spawn animation, firing patterns, and rendering.
 */
#include "Boss.h"
#include "Bullet.h"
#include <ncurses.h>
#include <algorithm>
#include <cstdlib>
#include <memory>

// ASCII art describing the boss ship, roughly matching boss_png.png.
static const char* BOSS_SHAPE[] = {
    "      /#####\\      ",
    "    ##[=====]##    ",
    "   ###| *** |###   ",
    "  ##<<| *** |>>##  ",
    "   ###| *** |###   ",
    "    ##[=====]##    ",
    "      \\_V_/        ",
};
static const int BOSS_SHAPE_ROWS = sizeof(BOSS_SHAPE) / sizeof(BOSS_SHAPE[0]);
static const int BOSS_SHAPE_COLS = 21; // Length of each ASCII line.

/*
 * Purpose: Initialize the boss ship and configure stats based on difficulty.
 * Inputs: diff - current difficulty setting selected by the player.
 * Returns: Boss instance ready for the spawn animation.
 */
Boss::Boss(Difficulty diff)
    : GameObject(),
      maxHealth(2000),
      currentHealth(2000),
      width(BOSS_SHAPE_COLS),
      height(BOSS_SHAPE_ROWS),
      shootTimer(0),
      shootInterval(4),
      baseInterval(4),
      difficulty(diff),
      isSpawning(true),
      appearTargetY(4),
      appearTimer(0),
      appearSpeedFrames(2),
      shootDelayFrames(0) { // Medium difficulty delay baseline.

    // Set boss health and firing pace by difficulty.
    switch (difficulty) {
        case Difficulty::EASY:
            maxHealth = 3000;
            baseInterval = 10;
            break;
        case Difficulty::MEDIUM:
            maxHealth = 5000;
            baseInterval = 8;
            break;
        case Difficulty::HARD:
            maxHealth = 5000;
            baseInterval = 7;
            break;
    }
    currentHealth = maxHealth;
    shootInterval = baseInterval;

    // Center the boss and start it above the visible window for a descent animation.
    x = std::max(1, (WINDOW_WIDTH - width) / 2);
    y = -height;          // Start outside the top edge.
    symbol = '#';
    isActive = true;
}

/*
 * Purpose: Drive the boss behavior including spawn animation and bullet patterns.
 * Inputs: bullets - shared projectile container to which the boss appends shots.
 * Returns: Nothing.
 */
void Boss::update(std::vector<std::unique_ptr<Bullet>>& bullets) {
    if (!isActive) return;

    // Spawn animation: boss drifts into the arena.
    if (isSpawning) {
        appearTimer++;
        if (appearTimer >= appearSpeedFrames) {
            appearTimer = 0;
            y++;
        }
        if (y >= appearTargetY) {
            y = appearTargetY;
            isSpawning = false;
            // After arriving, wait ~2.5 seconds (~50 frames) before firing.
            shootDelayFrames = 50;
        }
        return;
    }

    // Delay period before the first volley.
    if (shootDelayFrames > 0) {
        shootDelayFrames--;
        return;
    }

    if (shootTimer > 0) {
        shootTimer--;
        return;
    }

    // Select a firing pattern; randomness weighted by difficulty.
    int r = std::rand() % 100;
    bool burst = false;

    switch (difficulty) {
        case Difficulty::EASY:
            if (r < 70) {
                // Mostly light random fire.
                fireCrazyRandom(bullets, false);
            } else {
                fireSpreadPattern(bullets);
            }
            break;
        case Difficulty::MEDIUM:
            if (r < 80) {
                fireCrazyRandom(bullets, false);
            } else if (r < 90) {
                fireSpreadPattern(bullets);
            } else {
                fireRandomScatter(bullets);
            }
            break;
        case Difficulty::HARD:
            // Hard mode favors chaotic volleys with occasional bursts.
            if (r < 85) {
                fireCrazyRandom(bullets, false);
            } else {
                burst = (std::rand() % 5 == 0); // Rarely trigger a burst.
                fireCrazyRandom(bullets, burst);
            }
            break;
    }

    // Randomize the next cooldown slightly around the base interval.
    int jitter = (std::rand() % 3) - 1; // -1,0,1
    shootInterval = std::max(1, baseInterval + jitter);
    shootTimer = shootInterval;
}

/*
 * Purpose: Fire a symmetric spread toward the player.
 * Inputs: bullets - projectile container.
 * Returns: Nothing.
 */
void Boss::fireSpreadPattern(std::vector<std::unique_ptr<Bullet>>& bullets) {
    int by = y + height;
    if (by >= WINDOW_HEIGHT - 1) return;

    int centerX = x + width / 2;

    // Straight shot down the middle.
    bullets.push_back(std::make_unique<Bullet>(centerX, by, 0, 1, 'o', false));

    // Symmetric offsets create a fan shape.
    for (int i = 1; i <= 4; ++i) {
        int offset = 2 + (i - 1) * 3;
        int dxLeft = -i;   // Slight left deflection.
        int dxRight = i;   // Slight right deflection.

        int leftX  = centerX - offset;
        int rightX = centerX + offset;

        if (leftX > 0 && leftX < WINDOW_WIDTH - 1) {
            bullets.push_back(std::make_unique<Bullet>(leftX, by, dxLeft, 1, 'o', false));
        }
        if (rightX > 0 && rightX < WINDOW_WIDTH - 1) {
            bullets.push_back(std::make_unique<Bullet>(rightX, by, dxRight, 1, 'o', false));
        }
    }
}

/*
 * Purpose: Fire eight bullets from random columns with light drift.
 * Inputs: bullets - projectile container.
 * Returns: Nothing.
 */
void Boss::fireRandomScatter(std::vector<std::unique_ptr<Bullet>>& bullets) {
    int by = y + height;
    if (by >= WINDOW_HEIGHT - 1) return;

    // Fire bullets from random columns with varied horizontal drift.
    for (int i = 0; i < 8; ++i) {
        int bx = x + (std::rand() % std::max(1, width));
        int dx = (std::rand() % 5) - 2; // [-2, 2]
        if (bx > 0 && bx < WINDOW_WIDTH - 1) {
            bullets.push_back(std::make_unique<Bullet>(bx, by, dx, 1, 'o', false));
        }
    }
}

/*
 * Purpose: Emit a difficulty-scaled volley with randomness and optional burst mode.
 * Inputs: bullets - projectile container, burst - toggles higher volume and drift.
 * Returns: Nothing.
 */
void Boss::fireCrazyRandom(std::vector<std::unique_ptr<Bullet>>& bullets, bool burst) {
    int by = y + height;
    if (by >= WINDOW_HEIGHT - 1) return;

    int minCount = 0;
    int maxCount = 0;

    // Determine volley sizes for each difficulty tier.
    switch (difficulty) {
        case Difficulty::EASY:
            minCount = 3;
            maxCount = 5;
            break;
        case Difficulty::MEDIUM:
            minCount = 4;
            maxCount = 7;
            break;
        case Difficulty::HARD:
            minCount = 5;
            maxCount = 8;
            break;
    }

    if (burst && difficulty == Difficulty::HARD) {
        maxCount += 4; // Slightly more bullets during bursts.
    }

    int countRange = std::max(1, maxCount - minCount + 1);
    int count = minCount + (std::rand() % countRange);

    for (int i = 0; i < count; ++i) {
        int bx = x + (std::rand() % std::max(1, width));

        int dxRange = 0;
        switch (difficulty) {
            case Difficulty::EASY:
                dxRange = 0; // Straight downward.
                break;
            case Difficulty::MEDIUM:
                dxRange = 1; // Slight drift.
                break;
            case Difficulty::HARD:
                dxRange = 1; // Same baseline drift.
                break;
        }
        if (burst && difficulty == Difficulty::HARD) {
            dxRange += 1; // Additional chaos during bursts.
        }

        int dx = (std::rand() % (dxRange * 2 + 1)) - dxRange; // [-dxRange, dxRange]

        int dy = 1;
        if (difficulty == Difficulty::HARD) {
            dy = 1 + (std::rand() % 2); // Occasionally faster bullets.
        }

        if (bx > 0 && bx < WINDOW_WIDTH - 1) {
            bullets.push_back(std::make_unique<Bullet>(bx, by, dx, dy, 'o', false));
        }
    }
}

/*
 * Purpose: Render the boss ASCII art at its current position.
 * Inputs: None.
 * Returns: Nothing.
 */
void Boss::draw() const {
    if (!isActive) return;

    // Draw boss body using ASCII art.
    if (has_colors()) {
        attron(COLOR_PAIR(6));
    }

    for (int row = 0; row < height; ++row) {
        int drawY = y + row;
        if (drawY <= 0 || drawY >= WINDOW_HEIGHT) continue;
        for (int col = 0; col < width && (x + col) < WINDOW_WIDTH; ++col) {
            char ch = BOSS_SHAPE[row][col];
            if (ch != ' ') { // Treat spaces as transparent pixels.
                mvaddch(translateY(drawY), translateX(x + col), ch);
            }
        }
    }

    if (has_colors()) {
        attroff(COLOR_PAIR(6));
    }

}

/*
 * Purpose: Apply player damage to the boss and deactivate it on death.
 * Inputs: amount - raw damage to subtract from health.
 * Returns: Nothing.
 */
void Boss::takeDamage(int amount) {
    if (!isActive) return;
    currentHealth -= amount;
    if (currentHealth <= 0) {
        currentHealth = 0;
        isActive = false;
    }
}

/*
 * Purpose: Determine if a point intersects the boss hit box.
 * Inputs: px/py - point to test (bullet or player coordinates).
 * Returns: True when the point overlaps the boss sprite.
 */
bool Boss::checkCollision(int px, int py) const {
    if (!isActive) return false;
    if (py < y || py >= y + height) return false;
    if (px < x || px >= x + width) return false;
    return true;
}
