#pragma once
#include <SFML/Graphics.hpp>

class nv1
{
public:
    nv1(sf::Vector2f startPos, sf::Vector2f direction, float speed, float damage);
    
    void update(float dt);
    void draw(sf::RenderWindow& window);
    
    sf::Vector2f getPosition() const { return shape.getPosition(); }
    float getDamage() const { return damage; }
    bool isAlive() const { return alive; }
    void setAlive(bool state) { alive = state; }
    sf::FloatRect getGlobalBounds() const { return shape.getGlobalBounds(); }

private:
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float damage;
    bool alive;
    float maxDistance;
    float distanceTraveled;
};
