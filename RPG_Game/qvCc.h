#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include<iostream>

// Cấu trúc lưu trữ thông tin độc tố trên mỗi quái vật
struct PoisonEffect {
    float damagePerSecond;
    float duration;
};

class qvCc
{
public:
    qvCc(sf::Vector2f startPos, float health = 30.0f, float damage = 10.0f, float speed = 80.0f);

    void update(float dt, const std::vector<std::vector<int>>& tiles, int tileSize, sf::Vector2f playerPos);
    void draw(sf::RenderWindow& window);

    sf::Vector2f getPosition() const { return shape.getPosition(); }
    float getHealth() const { return health; }
    float getDamage() const { return damage; }
    bool isAlive() const { return health > 0; }
    // Trong qvCc.h, sửa hàm takeDamage thành:
    void takeDamage(float dmg) {
        health -= dmg;
        if (health < 0.0f) health = 0.0f;
        std::cout << "Mau quai con lai: " << health << std::endl; // Debug để kiểm tra
    }
    sf::FloatRect getGlobalBounds() const { return shape.getGlobalBounds(); }

    // ====================================================================
    // CÁC HÀM XỬ LÝ HIỆU ỨNG CỦA NHÂN VẬT RẮN (ĐÃ LIÊN KẾT)
    // ====================================================================
    void applySlow(float slowDuration, float slowPercent);
    void addPoisonStack(float dmgPerSec, float duration);
    void updatePoisonAndSlow(float dt);

private:
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    sf::Vector2f targetPosition;
    float health;
    float maxHealth;
    float damage;
    float speed;             // Đây sẽ đóng vai trò là tốc độ thực tế sau khi bị chậm

    // ====================================================================
    // BIẾN QUẢN LÝ TRẠNG THÁI KHỐNG CHẾ VÀ ĐỘC TỐ
    // ====================================================================
    float baseSpeed;         // Lưu tốc độ gốc để hoàn trả khi hết làm chậm
    float slowTimer;         // Thời gian còn lại bị làm chậm
    float currentSlowPercent;// Tỉ lệ làm chậm hiện tại (với chiêu W hoặc E)
    std::vector<PoisonEffect> poisonStacks; // Mảng chứa các tầng độc

    float moveTimer;
    bool checkCollision(const sf::FloatRect& rect, const std::vector<std::vector<int>>& tiles, int tileSize);
};