#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "nv1.h"

class Player
{
public:
    Player();
    void update(float dt, const std::vector<std::vector<int>>& tiles, int tileSize, sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
    void drawProjectiles(sf::RenderWindow& window);

    sf::Vector2f getPosition() const { return shape.getPosition(); }
    void setPosition(float x, float y) { shape.setPosition(x, y); }
    std::vector<nv1>& getProjectiles() { return projectiles; }

    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    float getDamage() const { return baseDamage; }
    void takeDamage(float damage) { health -= damage; }
    void setTargetEnemy(sf::Vector2f targetPos) { lockedTargetPos = targetPos; hasLockedTarget = true; }
    void clearTarget() { hasLockedTarget = false; }

private:
    // Hình dáng và di chuyển
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    sf::Vector2f targetPosition;
    float speed;
    bool isMoving;
    bool lastMousePressed;

    // Stats
    float health;
    float maxHealth;
    float baseDamage;
    float defenseBoost;

    // Skill cooldown
    float skillQ_cooldown;
    float skillQ_cooldownMax;
    float skillW_cooldown;
    float skillW_cooldownMax;
    float skillE_cooldown;
    float skillE_cooldownMax;
    float skillE_duration;
    float skillR_cooldown;
    float skillR_cooldownMax;

    // Projectiles
    std::vector<nv1> projectiles;
    sf::Vector2f lastProjectilePos;

    // Target locking
    bool hasLockedTarget;
    sf::Vector2f lockedTargetPos;

    // Keyboard tracking
    bool lastKeyQ, lastKeyW, lastKeyE, lastKeyR;

    // Skill functions
    void skillQ(sf::Vector2f direction);  // Bắn projectile thẳng
    void skillW();                         // Dịch chuyển đến projectile + dame
    void skillE();                         // Cường hóa (tăng damage + defense)
    void skillR();                         // Dame liên tiếp 3 lần

    bool checkCollision(const sf::FloatRect& rect, const std::vector<std::vector<int>>& tiles, int tileSize);
};