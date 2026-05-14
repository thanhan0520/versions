#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Bullet.h"

class Player
{
public:
    Player();
    void update(float dt, const std::vector<std::vector<int>>& tiles, int tileSize, sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
    void drawProjectiles(sf::RenderWindow& window);

    sf::Vector2f getPosition() const { return shape.getPosition(); }
    void setPosition(float x, float y) { shape.setPosition(x, y); }
    std::vector<Bullet>& getProjectiles() { return projectiles; }

    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    float getDamage() const { return baseDamage; }
    void takeDamage(float damage) { health -= damage; }
    void setTargetEnemy(sf::Vector2f targetPos) { lockedTargetPos = targetPos; hasLockedTarget = true; }
    void clearTarget() { hasLockedTarget = false; }

    // Thêm vào trong class Player, phần public:
    float getSkillQ_Cooldown() const { return skillQ_cooldown; }
    float getSkillW_Cooldown() const { return skillW_cooldown; }
    float getSkillE_Cooldown() const { return skillE_cooldown; }
    float getSkillR_Cooldown() const { return skillR_cooldown; }

protected:
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

    sf::Texture bulletTexture;

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
    std::vector<Bullet> projectiles;
    sf::Vector2f lastProjectilePos;

    // Target locking
    bool hasLockedTarget;
    sf::Vector2f lockedTargetPos;

    // Keyboard tracking
    bool lastKeyQ, lastKeyW, lastKeyE, lastKeyR;

    // Skill functions
    virtual void skillQ(sf::Vector2f direction);  // Bắn projectile thẳng
    virtual void skillW(const std::vector<std::vector<int>>& tiles, int tileSize);      // Dịch chuyển đến projectile + dame
    virtual void skillE();                         // Cường hóa (tăng damage + defense)
    virtual void skillR();                         // Dame liên tiếp 3 lần

    bool checkCollision(const sf::FloatRect& rect, const std::vector<std::vector<int>>& tiles, int tileSize);
};