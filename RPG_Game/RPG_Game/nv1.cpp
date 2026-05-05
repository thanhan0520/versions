#include "nv1.h"
#include <cmath>

nv1::nv1(sf::Vector2f startPos, sf::Vector2f direction, float speed, float damage)
    : damage(damage), alive(true), maxDistance(1000.0f), distanceTraveled(0.0f)
{
    shape.setRadius(5.0f);
    shape.setFillColor(sf::Color::Yellow);
    shape.setPosition(startPos);
    
    // Chuẩn hóa hướng
    float len = sqrt(direction.x * direction.x + direction.y * direction.y);
    if (len > 0)
    {
        velocity.x = (direction.x / len) * speed;
        velocity.y = (direction.y / len) * speed;
    }
}

void nv1::update(float dt)
{
    if (!alive) return;
    
    sf::Vector2f newPos = shape.getPosition();
    newPos.x += velocity.x * dt;
    newPos.y += velocity.y * dt;
    
    float distMoved = sqrt(velocity.x * velocity.x + velocity.y * velocity.y) * dt;
    distanceTraveled += distMoved;
    
    if (distanceTraveled > maxDistance)
    {
        alive = false;
    }
    else
    {
        shape.setPosition(newPos);
    }
}

void nv1::draw(sf::RenderWindow& window)
{
    if (alive)
        window.draw(shape);
}
