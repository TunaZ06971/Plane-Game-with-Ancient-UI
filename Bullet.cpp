/*
 * File: Bullet.cpp
 * Purpose: Implements projectile movement, ownership, and rendering routines.
 */
#include "Bullet.h"
#include <ncurses.h>

/*
 * Purpose: Spawn a default upward-traveling player bullet.
 * Inputs: x/y specify the spawn coordinate relative to the playfield.
 * Returns: Bullet configured to travel upward from the player.
 */
Bullet::Bullet(int x, int y)
    : Bullet(x, y, 0, -1, '|', true) {
}

/*
 * Purpose: Fully configure a bullet, including direction, appearance, and ownership.
 * Inputs: x/y define the spawn position, dx/dy define velocity, symbol selects the character, fromPlayer flags ownership.
 * Returns: Bullet ready to be updated and drawn.
 */
Bullet::Bullet(int x, int y, int dx, int dy, char symbol, bool fromPlayer)
    : GameObject(x, y, symbol),
      dx(dx),
      dy(dy),
      fromPlayer(fromPlayer),
      colorPair(2) {
    // Player shots use yellow, enemy/boss shots use an orange pair for contrast.
    if (fromPlayer) {
        colorPair = 2; // yellow
    } else {
        colorPair = 8; // orange-ish
    }
}

/*
 * Purpose: Advance the bullet according to its velocity each frame.
 * Inputs: None.
 * Returns: Nothing, but the active flag flips off when the bullet exits the window.
 */
void Bullet::update() {
    x += dx;
    y += dy;

    if (!isInWindow()) {
        isActive = false;
    }
}

/*
 * Purpose: Render the bullet if it remains active and on screen.
 * Inputs: None.
 * Returns: Nothing.
 */
void Bullet::draw() const {
    if (isActive && isInWindow()) {
        if (has_colors()) {
            attron(COLOR_PAIR(colorPair));
        }
        mvaddch(translateY(y), translateX(x), symbol);
        if (has_colors()) {
            attroff(COLOR_PAIR(colorPair));
        }
    }
}

