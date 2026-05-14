#include "Bullet.h"
#include <cmath>

Bullet::Bullet(sf::Vector2f startPos, sf::Vector2f direction, float speed, float damage, const sf::Texture& texture)
    : damage(damage), alive(true), maxDistance(800.0f), distanceTraveled(0.0f)
{
    // 1. KIỂM TRA TEXTURE (Phần quan trọng nhất)
    // Nếu texture chưa được nạp (kích thước bằng 0), chúng ta tạo một vùng hiển thị tạm thời
    if (texture.getSize().x == 0 || texture.getSize().y == 0) {
        // Tạo một ô vuông 10x10 màu vàng để thay thế ảnh bị thiếu
        sprite.setTextureRect(sf::IntRect(0, 0, 10, 10));
        sprite.setColor(sf::Color::Yellow);
    }
    else {
        sprite.setTexture(texture);
    }

    // 2. Căn tâm cho ảnh/ô màu
    sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
    sprite.setPosition(startPos);

    // 3. Tính toán hướng bay (Chuẩn hóa vector)
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length != 0)
        velocity = (direction / length) * speed;

    // 4. Xoay viên đạn theo hướng di chuyển
    // Dùng 3.14159f để tính toán góc xoay (angle)
    float angle = std::atan2(direction.y, direction.x) * 180 / 3.14159f;
    sprite.setRotation(angle);
}

void Bullet::update(float dt)
{
    // Di chuyển viên đạn
    sf::Vector2f movement = velocity * dt;
    sprite.move(movement);

    // Tính quãng đường tích lũy để tự hủy khi bay quá xa
    distanceTraveled += std::sqrt(movement.x * movement.x + movement.y * movement.y);
    if (distanceTraveled > maxDistance)
    {
        alive = false;
    }
}

void Bullet::draw(sf::RenderWindow& window)
{
    if (alive)
    {
        // Kiểm tra nếu sprite không có ảnh thì ép nó hiện màu đỏ đậm để dễ nhìn
        if (sprite.getTexture() == nullptr) {
            sf::RectangleShape debugShape(sf::Vector2f(10.f, 10.f));
            debugShape.setOrigin(5.f, 5.f);
            debugShape.setPosition(sprite.getPosition());
            debugShape.setRotation(sprite.getRotation());
            debugShape.setFillColor(sf::Color::Red); // Đổi sang màu Đỏ cho nổi bật
            window.draw(debugShape);
        }
        else {
            window.draw(sprite);
        }
    }
}