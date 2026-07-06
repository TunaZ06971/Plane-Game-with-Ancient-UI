/*
 * File: Boss.h
 * Purpose: Declares the Boss class responsible for the final encounter behavior and firing patterns.
 */
#ifndef BOSS_H
#define BOSS_H

#include "GameObject.h"
#include "GameTypes.h"
#include <memory>
#include <vector>

class Bullet;

class Boss : public GameObject {
private:
    int maxHealth;
    int currentHealth;
    int width;
    int height;

    int shootTimer;
    int shootInterval;
    int baseInterval;
    Difficulty difficulty;

    // Spawn animation & delay
    bool isSpawning;
    int appearTargetY;
    int appearTimer;
    int appearSpeedFrames;
    int shootDelayFrames; // frames to wait after reaching final position

public:
    Boss(Difficulty diff);

    using GameObject::update;

    // Update boss state and fire bullets into shared bullet list
    void update(std::vector<std::unique_ptr<Bullet>>& bullets);

    // Patterned bullet firing using shared Bullet objects
    void fireSpreadPattern(std::vector<std::unique_ptr<Bullet>>& bullets);
    void fireRandomScatter(std::vector<std::unique_ptr<Bullet>>& bullets);
    void fireCrazyRandom(std::vector<std::unique_ptr<Bullet>>& bullets, bool burst);

    // Draw boss body and health bar
    void draw() const override;

    // Damage taken from player bullets
    void takeDamage(int amount);

    int getHealth() const { return currentHealth; }
    int getMaxHealth() const { return maxHealth; }

    // Check if a point (e.g. player or bullet) collides with boss body
    bool checkCollision(int px, int py) const;

    bool isDead() const { return currentHealth <= 0; }
};

#endif

