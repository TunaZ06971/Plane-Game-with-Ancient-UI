/*
 * File: Bullet.h
 * Purpose: Declares the Bullet class that models projectiles fired by players and enemies.
 */
#ifndef BULLET_H
#define BULLET_H

#include "GameObject.h"

// Bullet class
class Bullet : public GameObject {
private:
    int dx;          // horizontal velocity
    int dy;          // vertical velocity
    bool fromPlayer; // true = player bullet, false = enemy/boss bullet
    int colorPair;   // ncurses color pair used for drawing

public:
    // Constructor: create upward-moving player bullet at specified position
    Bullet(int x, int y);

    // Generic constructor: fully specify velocity, appearance and owner
    Bullet(int x, int y, int dx, int dy, char symbol, bool fromPlayer);

    // Override update method
    void update() override;
    
    // Override draw method
    void draw() const override;

    bool isFromPlayer() const { return fromPlayer; }
};

#endif

