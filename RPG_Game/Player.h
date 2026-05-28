#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Bullet.h"
#include "qvCc.h" 

enum class CharacterClass {
    NONE = 0,
    FOX = 1,
    RABBIT = 2,
    SNAKE = 3,
    DOG = 4
};

class Player
{
public:
    Player();
    virtual ~Player() {}

    // Kích hoạt tính đa hình cho hàm update bằng từ khóa virtual
    virtual void update(float dt, const std::vector<std::vector<int>>& tiles, int tileSize, sf::RenderWindow& window);

    void draw(sf::RenderWindow& window);
    void drawProjectiles(sf::RenderWindow& window);

    sf::Vector2f getPosition() const { return shape.getPosition(); }
    void setPosition(float x, float y) { shape.setPosition(x, y); }
    std::vector<Bullet>& getProjectiles() { return projectiles; }

    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    float getDamage() const { return baseDamage; }
    bool isAlive() const { return health > 0; }

    void takeDamage(float damage) {
        if (health > 0) {
            health -= damage;
            if (health < 0) health = 0;
        }
    }

    sf::FloatRect getGlobalBounds() const { return shape.getGlobalBounds(); }

    void setTargetEnemy(sf::Vector2f targetPos) { lockedTargetPos = targetPos; hasLockedTarget = true; }
    void clearTarget() { hasLockedTarget = false; }

    float getSkillQ_Cooldown() const { return skillQ_cooldown; }
    float getSkillW_Cooldown() const { return skillW_cooldown; }
    float getSkillE_Cooldown() const { return skillE_cooldown; }
    float getSkillR_Cooldown() const { return skillR_cooldown; }

    void autoTargetClosestEnemy(const std::vector<qvCc>& enemies);
    CharacterClass getCharacterClass() const { return currentClass; }

protected:
    CharacterClass currentClass;

    sf::RectangleShape shape;
    sf::Vector2f velocity;
    sf::Vector2f targetPosition;
    float speed;
    bool isMoving;
    bool lastMousePressed;

    float health;
    float maxHealth;
    float baseDamage;
    float defenseBoost;

    sf::Texture bulletTexture;

    float skillQ_cooldown;
    float skillQ_cooldownMax;
    float skillW_cooldown;
    float skillW_cooldownMax;
    float skillE_cooldown;
    float skillE_cooldownMax;
    float skillE_duration;
    float skillR_cooldown;
    float skillR_cooldownMax;

    std::vector<Bullet> projectiles;
    sf::Vector2f lastProjectilePos;

    bool hasLockedTarget;
    sf::Vector2f lockedTargetPos;

    bool lastKeyQ, lastKeyW, lastKeyE, lastKeyR;

    virtual void skillQ(sf::Vector2f direction);
    virtual void skillW(sf::Vector2f direction,
        const std::vector<std::vector<int>>& tiles,
        int tileSize);
    virtual void skillE(sf::Vector2f direction);
    virtual void skillR(sf::Vector2f direction);

    bool checkCollision(const sf::FloatRect& rect, const std::vector<std::vector<int>>& tiles, int tileSize);
};