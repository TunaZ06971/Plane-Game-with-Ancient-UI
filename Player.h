#ifndef PLAYER_H
#define PLAYER_H

#include "GameObject.h"

// Player aircraft class
class Player : public GameObject {
public:
    // Constructor: initialize player at bottom center of screen
    Player();
    
    // Movement methods
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    
    // Override draw method for special drawing logic
    void draw() const override;
};

#endif

