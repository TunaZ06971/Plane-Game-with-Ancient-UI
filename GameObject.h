#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "GameTypes.h"

// Base class for all game objects
class GameObject {
protected:
    int x, y;           // Position coordinates
    char symbol;        // Display symbol
    bool isActive;      // Active state
    
public:
    GameObject(int x = 0, int y = 0, char symbol = ' ');

    virtual ~GameObject() = default;

    virtual void update() {}

    virtual void draw() const {}

    int getX() const { return x; }

    int getY() const { return y; }

    char getSymbol() const { return symbol; }

    bool isActiveState() const { return isActive; }

    void setX(int newX) { x = newX; }

    void setY(int newY) { y = newY; }

    void setSymbol(char newSymbol) { symbol = newSymbol; }

    void setActive(bool state) { isActive = state; }

    bool isInWindow() const;
};

#endif

