#include "Snake.h"
#include <iostream>

Snake::Snake() : Player()
{
    this->currentClass = CharacterClass::SNAKE;

    this->shape.setSize(sf::Vector2f(45.0f, 45.0f));
    this->shape.setFillColor(sf::Color(34, 139, 34)); // ForestGreen
    this->shape.setPosition(512.0f, 384.0f);
    this->targetPosition = this->shape.getPosition();

    this->maxHealth = 120.0f;
    this->health = this->maxHealth;
    this->baseDamage = 16.0f;
    this->defenseBoost = 0.0f; // Đã sửa từ 6.0f về 0.0f (Tránh lỗi cộng ngược máu khi bị quái cắn)
    this->speed = 340.0f;

    this->skillQ_cooldownMax = 1.5f;
    this->skillW_cooldownMax = 6.0f;
    this->skillE_cooldownMax = 8.0f;
    this->skillR_cooldownMax = 18.0f;

    this->isAwakened = false;
    this->awakeningTimer = 0.0f;
    this->originalBaseDamage = this->baseDamage;
}

// ====================================================================
// THÊM MỚI: HÀM UPDATE CHÍNH CỦA ĐỐI TƯỢNG SNAKE
// ====================================================================
void Snake::update(float dt, const std::vector<std::vector<int>>& tiles, int tileSize, sf::RenderWindow& window)
{
    // CỰC KỲ QUAN TRỌNG: Gọi hàm update của lớp cha Player để xử lý di chuyển,
    // va chạm tường, cập nhật đạn và đếm ngược thời gian hồi chiêu.
    Player::update(dt, tiles, tileSize, window);

    // Tích hợp luồng cập nhật trạng thái thức tỉnh tự động vào đây
    this->updateAwakening(dt);
}

void Snake::updateAwakening(float dt)
{
    if (this->isAwakened)
    {
        this->awakeningTimer -= dt;
        this->shape.setFillColor(sf::Color(0, 255, 127));

        if (this->awakeningTimer <= 0.0f)
        {
            this->isAwakened = false;
            this->baseDamage = this->originalBaseDamage;
            this->shape.setFillColor(sf::Color(34, 139, 34));
            std::cout << "==> Cobra Awakening het thoi gian thuc tinh!" << std::endl;
        }
    }
}

// KỸ NĂNG 1: Venom Strike (Q) - Quật xích duỗi thẳng kéo về
void Snake::skillQ(sf::Vector2f direction)
{
    std::cout << "[Skill Q] Venom Strike! Vung xich doc vao huong muc tieu!" << std::endl;
    static sf::Texture dummyTexture;

    float dmg = this->isAwakened ? this->baseDamage * 1.6f : this->baseDamage * 1.1f;
    // Tốc độ vung đòn 750.0f nhanh mạnh
    this->projectiles.emplace_back(this->shape.getPosition(), direction, 750.0f, dmg, dummyTexture, 0, BulletType::SNAKE_Q);
}

// KỸ NĂNG 2: Poison Fog (W) - Sương độc
void Snake::skillW(sf::Vector2f direction,
    const std::vector<std::vector<int>>& tiles,
    int tileSize)
{
    std::cout << "[Skill W] Poison Fog! Giai phong suong doc pham vi!" << std::endl;
    static sf::Texture dummyTexture;

    sf::Vector2f spawnPos = this->shape.getPosition();
    if (this->hasLockedTarget) {
        spawnPos = this->lockedTargetPos;
    }

    this->projectiles.emplace_back(spawnPos, sf::Vector2f(0, 0), 0.0f, this->baseDamage * 0.5f, dummyTexture, 0, BulletType::SNAKE_W);
    float size = this->isAwakened ? 180.0f : 120.0f;
    this->projectiles.back().setBulletSize(sf::Vector2f(size, size));
    this->projectiles.back().setBulletColor(sf::Color(144, 238, 144, 100));
}

// KỸ NĂNG 3: Snake Chain (E) - Phóng xích ghim chặt rút máu mục tiêu đến chết
void Snake::skillE(sf::Vector2f direction)
{
    std::cout << "[Skill E] Snake Chain! Phong xich ghim doc len muc tieu!" << std::endl;
    static sf::Texture dummyTexture;

    if (this->hasLockedTarget) {
        direction = this->lockedTargetPos - this->shape.getPosition();
    }

    this->projectiles.emplace_back(this->shape.getPosition(), direction, 800.0f, this->baseDamage * 0.4f, dummyTexture, 0, BulletType::SNAKE_E);
}

// KỸ NĂNG CUỐI: Cobra Awakening (R) - Cắm xích tạo ma trận kết giới địa ngục xích trồi
void Snake::skillR(sf::Vector2f direction)
{
    std::cout << "[Ultimate R] COBRA AWAKENING! Cam xich xuong dat lap Ma Tran Xich Xa!" << std::endl;
    static sf::Texture dummyTexture;

    this->isAwakened = true;
    this->awakeningTimer = 7.0f;
    this->originalBaseDamage = this->baseDamage;
    this->baseDamage *= 1.3f;

    // Triệu hồi kết giới đứng yên tại vị trí hiện tại dưới chân con Rắn
    this->projectiles.emplace_back(this->shape.getPosition(), sf::Vector2f(0.0f, 0.0f), 0.0f, this->baseDamage * 3.0f, dummyTexture, 0, BulletType::SNAKE_R);
}