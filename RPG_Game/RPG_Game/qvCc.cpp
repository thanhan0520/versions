#include "qvCc.h"
#include <cmath>
#include <cstdlib>
#include <iostream>

qvCc::qvCc(sf::Vector2f startPos, float health, float damage, float speed)
    : health(health), maxHealth(health), damage(damage), speed(speed), moveTimer(0)
{
    shape.setSize(sf::Vector2f(40, 40));
    shape.setFillColor(sf::Color::Blue);
    shape.setPosition(startPos);
    targetPosition = startPos;
    velocity = sf::Vector2f(0, 0);
}

bool qvCc::checkCollision(const sf::FloatRect& rect, const std::vector<std::vector<int>>& tiles, int tileSize)
{
    int leftTile = rect.left / tileSize;
    int rightTile = (rect.left + rect.width) / tileSize;
    int topTile = rect.top / tileSize;
    int bottomTile = (rect.top + rect.height) / tileSize;

    for (int y = topTile; y <= bottomTile; y++)
    {
        if (y < 0 || y >= (int)tiles.size()) return true;

        for (int x = leftTile; x <= rightTile; x++)
        {
            if (x < 0 || x >= (int)tiles[0].size()) return true;

            if (tiles[y][x] == 1) // tường
                return true;
        }
    }
    return false;
}

void qvCc::update(float dt, const std::vector<std::vector<int>>& tiles, int tileSize, sf::Vector2f playerPos)
{
    moveTimer += dt;
    
    // Tính khoảng cách đến player
    sf::Vector2f currentPos = shape.getPosition();
    sf::Vector2f toPlayer = playerPos - currentPos;
    float distToPlayer = sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
    
    // Nếu player ở gần (500 pixels), quái theo dõi
    if (distToPlayer < 500.0f && distToPlayer > 50.0f)
    {
        // Đi theo player
        float len = sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
        if (len > 0)
        {
            velocity.x = (toPlayer.x / len);
            velocity.y = (toPlayer.y / len);
        }
    }
    else if (moveTimer > 2.0f)
    {
        // Di chuyển ngẫu nhiên
        int randDir = rand() % 4;
        moveTimer = 0;
        
        if (randDir == 0) velocity = sf::Vector2f(1, 0);      // Phải
        else if (randDir == 1) velocity = sf::Vector2f(-1, 0); // Trái
        else if (randDir == 2) velocity = sf::Vector2f(0, 1);  // Dưới
        else velocity = sf::Vector2f(0, -1);                   // Trên
    }
    else if (distToPlayer <= 50.0f)
    {
        // Dừng lại khi gần player (để tấn công)
        velocity = sf::Vector2f(0, 0);
    }

    // Di chuyển theo X
    sf::Vector2f newPos = shape.getPosition();
    newPos.x += velocity.x * speed * dt;
    shape.setPosition(newPos);

    sf::FloatRect enemyRect = shape.getGlobalBounds();
    if (checkCollision(enemyRect, tiles, tileSize))
    {
        newPos.x -= velocity.x * speed * dt;
        shape.setPosition(newPos);
    }

    // Di chuyển theo Y
    newPos = shape.getPosition();
    newPos.y += velocity.y * speed * dt;
    shape.setPosition(newPos);

    enemyRect = shape.getGlobalBounds();
    if (checkCollision(enemyRect, tiles, tileSize))
    {
        newPos.y -= velocity.y * speed * dt;
        shape.setPosition(newPos);
    }
}

void qvCc::draw(sf::RenderWindow& window)
{
    window.draw(shape);
    
    // Vẽ health bar
    sf::RectangleShape healthBar(sf::Vector2f(40, 5));
    healthBar.setPosition(shape.getPosition().x, shape.getPosition().y - 10);
    healthBar.setFillColor(sf::Color::Red);
    window.draw(healthBar);
    
    float healthPercent = health / maxHealth;
    sf::RectangleShape healthBarFill(sf::Vector2f(40 * healthPercent, 5));
    healthBarFill.setPosition(shape.getPosition().x, shape.getPosition().y - 10);
    healthBarFill.setFillColor(sf::Color::Green);
    window.draw(healthBarFill);
}