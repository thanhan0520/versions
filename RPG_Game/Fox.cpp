#include "Fox.h"
#include <iostream>
#include <cmath>

Fox::Fox() : Player()
{
    this->currentClass = CharacterClass::FOX;

    // ĐỒNG BỘ ĐỒ HỌA NHÂN VẬT CÁO
    this->shape.setSize(sf::Vector2f(46.0f, 46.0f));
    this->shape.setFillColor(sf::Color(255, 69, 0)); // Màu Cam Lửa rực rỡ đặc trưng
    this->shape.setPosition(512.0f, 384.0f);
    this->targetPosition = this->shape.getPosition();

    // THIẾT LẬP CHỈ SỐ CÁO KIẾM SĨ THEO ĐÚNG MÔ TẢ (X/10)
    this->maxHealth = 120.0f;          // HP: 6/10 (Cao hơn Thỏ một chút)
    this->health = this->maxHealth;
    this->baseDamage = 24.0f;         // ATK: 7/10 (Sát thương cận chiến mạnh mẽ)
    this->defenseBoost = 0.0f;        // Đặt mặc định giảm sát thương ban đầu là 0.0f (0%)
    this->speed = 360.0f;             // SPD: 8/10 (Nhanh nhẹn, lướt đi linh hoạt)

    // Thiết lập thời gian hồi chiêu (Cooldown) phù hợp cấu trúc Combo kỹ thuật
    this->skillQ_cooldownMax = 1.5f;  // Flame Slash hồi rất nhanh để liên tục chém xả
    this->skillW_cooldownMax = 6.0f;  // Trap Wire bẫy dây giữ chân khống chế kẻ địch
    this->skillE_cooldownMax = 4.0f;  // Shadow Roll hồi nhanh để liên tục lộn né đòn
    this->skillR_cooldownMax = 15.0f; // Mirage Hunt chiêu cuối dồn combo áp sát chớp nhoáng

    this->isInvulnerable = false;
    this->originalSpeed = this->speed;

    std::cout << "Fox: Linh thu CAO KIEM SI FLAME-BLADE da san sang vao tran!" << std::endl;
}

// ====================================================================
// THÊM MỚI: HÀM UPDATE CHÍNH CỦA ĐỐI TƯỢNG FOX
void Fox::update(float dt, const std::vector<std::vector<int>>& tiles, int tileSize, sf::RenderWindow& window)
{
    Player::update(dt, tiles, tileSize, window);

    if (this->skillE_duration <= 0.0f)
    {
        this->defenseBoost = 0.0f;
        this->speed = 360.0f;

        // Đã sửa lỗi C2660 và C2186 tại đây
        if (this->shape.getFillColor() != sf::Color(255, 69, 0)) {
            this->shape.setFillColor(sf::Color(255, 69, 0));
        }
    }
}

// --- KỸ NĂNG 1 – FLAME SLASH (Q) ---
void Fox::skillQ(sf::Vector2f direction) {
    std::cout << "-> Fox vung kiem cong: FLAME SLASH!" << std::endl;

    static sf::Texture dummyTexture;
    this->projectiles.emplace_back(this->shape.getPosition(), direction, 1000.0f, this->baseDamage * 1.4f, dummyTexture, 0, BulletType::FOX_Q);

    this->projectiles.back().setBulletColor(sf::Color(255, 100, 0));
    this->projectiles.back().setBulletSize(sf::Vector2f(35.0f, 10.0f));
}

// --- KỸ NĂNG 2 – TRAP WIRE (W) ---
void Fox::skillW(sf::Vector2f direction,
    const std::vector<std::vector<int>>& tiles,
    int tileSize)
{
    std::cout << "-> Fox dat bay: TRAP WIRE duoi san!" << std::endl;

    static sf::Texture dummyTexture;
    sf::Vector2f currentPos = this->shape.getPosition();
    sf::Vector2f staticDir(0.0f, 0.0f);

    this->projectiles.emplace_back(currentPos, staticDir, 0.0f, this->baseDamage * 0.8f, dummyTexture, 0, BulletType::FOX_W);

    this->projectiles.back().setBulletColor(sf::Color(169, 169, 169, 200));
    this->projectiles.back().setBulletSize(sf::Vector2f(45.0f, 45.0f));
}

// --- KỸ NĂNG 3 – SHADOW ROLL (E) ---
void Fox::skillE(sf::Vector2f direction) {
    std::cout << "-> Fox thuc hien: SHADOW ROLL (Lon ne tranh sat thuong & tang toc)!" << std::endl;

    if (this->skillE_duration <= 0) {
        this->originalSpeed = this->speed;
        this->speed = 600.0f;
        this->skillE_duration = 1.0f;

        this->defenseBoost = 1.0f;
        this->shape.setFillColor(sf::Color(128, 0, 128));
    }
}

// --- KỸ NĂNG CUỐI – MIRAGE HUNT (R) ---
void Fox::skillR(sf::Vector2f direction) {
    if (!this->hasLockedTarget) {
        std::cout << "-> [Mirage Hunt] Khong co muc tieu nao duoc khoa!" << std::endl;
        return;
    }

    std::cout << "==> KICH HOAT ULTIMATE: MIRAGE HUNT !!! <==" << std::endl;
    sf::Vector2f targetPos = this->lockedTargetPos;
    static sf::Texture dummyTexture;

    float radius = 90.0f;
    float angles[] = { 0.0f, 2.0944f, 4.1888f };

    for (int i = 0; i < 3; i++) {
        sf::Vector2f mirageSpawnPos = targetPos + sf::Vector2f(std::cos(angles[i]) * radius, std::sin(angles[i]) * radius);
        sf::Vector2f slashDir = targetPos - mirageSpawnPos;

        this->projectiles.emplace_back(mirageSpawnPos, slashDir, 450.0f, this->baseDamage * 1.2f, dummyTexture, 0, BulletType::FOX_R);
    }

    sf::Vector2f behindTarget = targetPos - (sf::Vector2f(50.0f, 0.0f));
    this->shape.setPosition(behindTarget);
    this->targetPosition = behindTarget;
    this->isMoving = false;
}