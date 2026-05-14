#pragma once
#include <SFML/Graphics.hpp>

class Bullet
{
public:
    // Giữ nguyên hàm khởi tạo với Texture
    Bullet(sf::Vector2f startPos, sf::Vector2f direction, float speed, float damage, const sf::Texture& texture);

    void update(float dt);
    void draw(sf::RenderWindow& window);

    // Các hàm lấy thông tin (Getter)
    sf::Vector2f getPosition() const { return sprite.getPosition(); }
    float getDamage() const { return damage; }
    bool isAlive() const { return alive; }
    void setAlive(bool state) { alive = state; }

    // Quan trọng: Hàm này trả về vùng va chạm của viên đạn để tính sát thương
    sf::FloatRect getGlobalBounds() const { return sprite.getGlobalBounds(); }

protected:
    sf::Sprite sprite;      // Chúng ta vẫn dùng Sprite
    sf::Vector2f velocity;
    float damage;
    bool alive;
    float maxDistance;
    float distanceTraveled;
};