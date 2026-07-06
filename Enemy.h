#ifndef ENEMY_H
#define ENEMY_H

#include "GameObject.h"

enum class ObstacleType {
    SOLID,
    BREAKABLE,
    DAMAGE_BUFF,
    EXPLODE
};

// Enemy aircraft class
class Enemy : public GameObject {
private:
    int moveSpeed;      // Movement speed
    float moveTimer;    // Movement timer (changed to float for precision)
    float moveInterval; // Frames between moves (changed to float)
    int width;          // Obstacle width (occupies multiple characters)
    ObstacleType type;
    int health;
    int maxHealth;
    int value;          // For numeric/buff blocks
    
public:
    // Constructor: create enemy at specified position
    Enemy(int x, int y);
    
    // Override update method: enemy moves downward
    void update() override;
    
    // Override draw method: draw larger obstacle
    void draw() const override;
    
    int getWidth() const { return width; }
    int getHeight() const { return 2; } // Fixed height for standard enemy
    int getMoveSpeed() const { return moveSpeed; }

    // Movement control
    void tickTimer();           // Advance internal timer
    bool isReadyToMove() const; // Check if timer threshold reached
    void performMove();         // Update Y and reset timer
    void stall();               // Wait (cap timer)

    // Configure move interval from Game
    void setMoveInterval(float interval) { moveInterval = interval; }

    void setType(ObstacleType t) { type = t; }
    ObstacleType getType() const { return type; }

    void setHealth(int hp) { health = hp; maxHealth = hp; }
    int getHealth() const { return health; }
    void damage(int amount) { health -= amount; }

    void setValue(int v) { value = v; }
    int getValue() const { return value; }
};

#endif

