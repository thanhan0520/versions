#include "qvCc.h"
#include <cmath>
#include <cstdlib>
#include <iostream>

qvCc::qvCc(sf::Vector2f startPos, float health, float damage, float speed)
    : health(health), maxHealth(health), damage(damage), speed(speed),
    baseSpeed(speed), slowTimer(0.0f), currentSlowPercent(0.0f), moveTimer(0)
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

// ====================================================================
// ĐỊNH NGHĨA CHÍNH XÁC CÁC HÀM XỬ LÝ THEO STRUCT POISONEFFECT
// ====================================================================

void qvCc::applySlow(float slowDuration, float slowPercent)
{
    // Nếu chiêu thức mới làm chậm nặng hơn chiêu cũ, hoặc hiệu ứng cũ đã hết, cập nhật tỉ lệ mới
    if (slowPercent > currentSlowPercent || slowTimer <= 0.0f) {
        currentSlowPercent = slowPercent;
    }
    slowTimer = slowDuration; // Làm mới thời gian khống chế
}

void qvCc::addPoisonStack(float dmgPerSec, float duration)
{
    // Giới hạn quái vật dính tối đa 8 tầng độc cùng lúc để tối ưu hiệu năng bản đồ
    if (poisonStacks.size() < 8) {
        // Khởi tạo chính xác theo kiểu dữ liệu PoisonEffect được khai báo trong qvCc.h
        PoisonEffect effect;
        effect.damagePerSecond = dmgPerSec;
        effect.duration = duration;
        poisonStacks.push_back(effect);
    }
}

void qvCc::updatePoisonAndSlow(float dt)
{
    if (health <= 0) return;

    // 1. Xử lý thời gian và rút máu từ các stack độc (poisonStacks)
    for (auto it = poisonStacks.begin(); it != poisonStacks.end(); )
    {
        health -= it->damagePerSecond * dt; // Rút máu dựa trên thuộc tính damagePerSecond
        it->duration -= dt;

        if (it->duration <= 0.0f) {
            it = poisonStacks.erase(it); // Xóa độc tố khi hết thời gian tác dụng
        }
        else {
            ++it;
        }
    }

    // Khóa máu không cho âm bậy để thanh máu hiển thị chính xác
    if (health < 0.0f) health = 0.0f;

    // 2. Xử lý giảm tốc chạy và đổi màu sắc nhận diện trạng thái quái vật
    if (slowTimer > 0.0f || !poisonStacks.empty())
    {
        if (slowTimer > 0.0f) {
            slowTimer -= dt;
            speed = baseSpeed * (1.0f - currentSlowPercent);
        }
        else {
            speed = baseSpeed; // Hết làm chậm nhưng còn độc thì trả lại tốc độ gốc
        }

        // Đổi màu quái sang Xanh Lá (SeaGreen) khi đang dính hiệu ứng độc/chậm
        shape.setFillColor(sf::Color(46, 139, 87));
    }
    else
    {
        slowTimer = 0.0f;
        currentSlowPercent = 0.0f;
        speed = baseSpeed; // Hoàn trả lại hoàn toàn tốc độ ban đầu
        shape.setFillColor(sf::Color::Blue); // Trả lại màu xanh dương mặc định
    }
}

void qvCc::update(float dt, const std::vector<std::vector<int>>& tiles, int tileSize, sf::Vector2f playerPos)
{
    if (health <= 0) return;

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

    // Di chuyển theo X (Biến 'speed' đã được tính toán tự động ở hàm updatePoisonAndSlow)
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
    if (health <= 0) return;

    window.draw(shape);

    // Vẽ health bar cơ sở (Màu đỏ)
    sf::RectangleShape healthBar(sf::Vector2f(40, 5));
    healthBar.setPosition(shape.getPosition().x, shape.getPosition().y - 10);
    healthBar.setFillColor(sf::Color::Red);
    window.draw(healthBar);

    float healthPercent = health / maxHealth;
    if (healthPercent < 0.0f) healthPercent = 0.0f;

    // Vẽ lượng máu hiện tại (Màu xanh lá)
    sf::RectangleShape healthBarFill(sf::Vector2f(40 * healthPercent, 5));
    healthBarFill.setPosition(shape.getPosition().x, shape.getPosition().y - 10);
    healthBarFill.setFillColor(sf::Color::Green);
    window.draw(healthBarFill);
}