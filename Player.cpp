#include "Player.h"
#include <ncurses.h>

/*
 * Purpose: Initialize the player at the bottom-middle of the window with a plane symbol.
 * Inputs: None.
 * Returns: Player instance ready for play.
 */
Player::Player() 
    : GameObject(WINDOW_WIDTH / 2, WINDOW_HEIGHT - 3, '^') {
}

/*
 * Purpose: Move the player one column to the left when space allows.
 * Inputs: None.
 * Returns: Nothing.
 */
void Player::moveLeft() {
    if (x > 1) {
        x--;
    }
}

/*
 * Purpose: Move the player one column to the right when space allows.
 * Inputs: None.
 * Returns: Nothing.
 */
void Player::moveRight() {
    if (x < WINDOW_WIDTH - 2) {
        x++;
    }
}

/*
 * Purpose: Move the player upward while staying inside the playfield.
 * Inputs: None.
 * Returns: Nothing.
 */
void Player::moveUp() {
    if (y > 0) {
        y--;
    }
}

/*
 * Purpose: Move the player downward without colliding with the status HUD.
 * Inputs: None.
 * Returns: Nothing.
 */
void Player::moveDown() {
    if (y < WINDOW_HEIGHT - 3) {
        y++;
    }
}

/*
 * Purpose: Render the player plane sprite using ncurses characters.
 * Inputs: None.
 * Returns: Nothing.
 */
void Player::draw() const {
    if (isActive) {
        if (has_colors()) {
            attron(COLOR_PAIR(1));  // Green
        }
        /* Draw larger plane shape
           ^
          /|\
        */
        mvaddch(translateY(y), translateX(x), '^');
        mvaddch(translateY(y + 1), translateX(x - 1), '/');
        mvaddch(translateY(y + 1), translateX(x), '|');
        mvaddch(translateY(y + 1), translateX(x + 1), '\\');
        
        if (has_colors()) {
            attroff(COLOR_PAIR(1));
        }
    }
}

