#include "GameObject.h"

/*
 * Purpose: Initialize positioning and symbol data for any game object.
 * Inputs: x/y define logical coordinates, symbol is the character displayed.
 * Returns: Fully initialized GameObject marked active.
 */
GameObject::GameObject(int x, int y, char symbol) 
    : x(x), y(y), symbol(symbol), isActive(true) {
}

/*
 * Purpose: Confirm that the object is visible within the logical play area.
 * Inputs: None.
 * Returns: True when the coordinates lie inside the configured window size.
 */
bool GameObject::isInWindow() const {
    return x >= 0 && x < WINDOW_WIDTH && y >= 0 && y < WINDOW_HEIGHT;
}