#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class qvCc
{
public:
    qvCc(sf::Vector2f startPos, float health = 30.0f, float damage = 10.0f, float speed = 80.0f);
    
    void update(float dt, const std::vector<std::vector<int>>& tiles, int tileSize, sf::Vector2f playerPos);
    void draw(sf::RenderWindow& window);
    
    sf::Vector2f getPosition() const { 
        return shape.getPosition(); 
    }
    float getHealth() const { 
        return health; 
    }
    float getDamage() const { 
        return damage; 
    }
    bool isAlive() const { 
        return health > 0; 
    }
    void takeDamage(float dmg) { 
        health -= dmg; 
    }
    sf::FloatRect getGlobalBounds() const {
        return shape.getGlobalBounds();
    }

private:
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    sf::Vector2f targetPosition;
    float health;
    float maxHealth;
    float damage;
    float speed;
    float moveTimer;
    bool checkCollision(const sf::FloatRect& rect, const std::vector<std::vector<int>>& tiles, int tileSize);
};
