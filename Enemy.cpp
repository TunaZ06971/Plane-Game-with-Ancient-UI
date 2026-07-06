#include "Enemy.h"
#include <ncurses.h>
#include <algorithm>
#include <string>

/*
 * Purpose: Initialize an enemy block at a given coordinate with default stats.
 * Inputs: x/y define the spawn position above the playfield.
 * Returns: Enemy configured with default movement values and obstacle width.
 */
Enemy::Enemy(int x, int y) 
    : GameObject(x, y, '#'),
      moveSpeed(1),
      moveTimer(0.0f),
      moveInterval(2.0f),   // default, will be overridden by Game
      width(3),
      type(ObstacleType::SOLID),
      health(1),
      maxHealth(1),
      value(0) {
    // Large obstacle, width 3, height 2
}

/*
 * Purpose: Provide a fallback update loop for enemies when the game does not micro-manage them.
 * Inputs: None.
 * Returns: Nothing; may trigger a downward move.
 */
void Enemy::update() {
    // Default update logic (legacy fallback, Game will call specific methods)
    tickTimer();
    if (isReadyToMove()) {
        performMove();
    }
}

/*
 * Purpose: Advance the internal movement timer by one frame.
 * Inputs: None.
 * Returns: Nothing.
 */
void Enemy::tickTimer() {
    moveTimer += 1.0f;
}

/*
 * Purpose: Determine whether the accumulated timer allows another move.
 * Inputs: None.
 * Returns: True when the stored time meets or exceeds the move interval.
 */
bool Enemy::isReadyToMove() const {
    return moveTimer >= moveInterval;
}

/*
 * Purpose: Move the enemy downward once and deactivate it if it exits the screen.
 * Inputs: None.
 * Returns: Nothing.
 */
void Enemy::performMove() {
    y += moveSpeed;
    moveTimer -= moveInterval;
    // If enemy moves out of bottom of screen, mark as inactive
    if (y >= WINDOW_HEIGHT) {
        isActive = false;
    }
}

/*
 * Purpose: Clamp the timer when motion is blocked so the enemy does not skip ahead later.
 * Inputs: None.
 * Returns: Nothing.
 */
void Enemy::stall() {
    // Cap timer to avoid "jumping" later
    if (moveTimer > moveInterval) {
        moveTimer = moveInterval;
    }
}

/*
 * Purpose: Draw the multi-tile enemy representation with colors and overlays.
 * Inputs: None.
 * Returns: Nothing.
 */
void Enemy::draw() const {
    if (!isActive) return;
    
    short colorPair = 3;
    switch (type) {
        case ObstacleType::SOLID:
            colorPair = 3;
            break;
        case ObstacleType::BREAKABLE:
            colorPair = 5;
            break;
        case ObstacleType::EXPLODE:
            colorPair = 6;
            break;
        case ObstacleType::DAMAGE_BUFF:
            colorPair = 7;
            break;
    }
    
    if (has_colors()) {
        attron(COLOR_PAIR(colorPair));
    }
    
    char fillChar = symbol;
    if (type == ObstacleType::DAMAGE_BUFF) {
        for (int i = 0; i < width && (x + i) < WINDOW_WIDTH; i++) {
            if (y >= 0 && y < WINDOW_HEIGHT) {
                mvaddch(translateY(y), translateX(x + i), fillChar);
            }
        }
        if (y + 1 < WINDOW_HEIGHT) {
            for (int i = 0; i < width && (x + i) < WINDOW_WIDTH; i++) {
                mvaddch(translateY(y + 1), translateX(x + i), fillChar);
            }
        }
        char centerChar = value > 0 ? '+' : '-';
        int textX = x + width / 2;
        if (y >= 0 && y < WINDOW_HEIGHT) {
            mvaddch(translateY(y), translateX(textX), centerChar);
        }
    } else {
        // Draw first row of obstacle (occupies 3 character width)
        for (int i = 0; i < width && (x + i) < WINDOW_WIDTH; i++) {
            if (y >= 0 && y < WINDOW_HEIGHT) {
                mvaddch(translateY(y), translateX(x + i), fillChar);
            }
        }
        
        // Draw second row of obstacle (if height allows)
        if (y + 1 < WINDOW_HEIGHT) {
            for (int i = 0; i < width && (x + i) < WINDOW_WIDTH; i++) {
                mvaddch(translateY(y + 1), translateX(x + i), fillChar);
            }
        }
    }
    
    // Overlay text for blocks that need it
    std::string text;
    if (type == ObstacleType::BREAKABLE) {
        int display = std::max(health, 0);
        text = std::to_string(display);
    } else if (type == ObstacleType::EXPLODE) {
        text = "X";
    } else if (type == ObstacleType::DAMAGE_BUFF) {
        text = std::to_string(value);
    }
    
    if (!text.empty()) {
        int textLen = std::min(static_cast<int>(text.size()), width);
        int textX = x + (width - textLen) / 2;
        int textY = y;
        if (textY >= 0 && textY < WINDOW_HEIGHT) {
            mvaddnstr(translateY(textY), translateX(textX), text.c_str(), textLen);
        }
    }
    
    if (has_colors()) {
        attroff(COLOR_PAIR(colorPair));
    }
}

