#include "Rabbit.h"
#include <iostream>
#include <cmath>

Rabbit::Rabbit() : Player()
{
    this->currentClass = CharacterClass::RABBIT;

    // ĐỒNG BỘ ĐỒ HỌA
    this->shape.setSize(sf::Vector2f(45.0f, 45.0f));
    this->shape.setFillColor(sf::Color(0, 191, 255)); // Màu xanh nước biển tốc độ
    this->shape.setPosition(512.0f, 384.0f);
    this->targetPosition = this->shape.getPosition();

    // CHỈ SỐ CHUẨN XẠ THỦ CO ĐỘNG
    this->maxHealth = 70.0f;
    this->health = this->maxHealth;
    this->baseDamage = 25.0f;
    this->defenseBoost = 0.0f;
    this->speed = 500.0f;             // SPD gốc của Thỏ

    this->skillQ_cooldownMax = 3.5f;
    this->skillW_cooldownMax = 6.0f;
    this->skillE_cooldownMax = 5.0f;
    this->skillR_cooldownMax = 25.0f;

    this->isOverclocked = false;
    this->overclockTimer = 0.0f;

    this->originalSpeed = this->speed;
    this->originalDamage = this->baseDamage;

    isBurstFiring = false;
    burstShotsLeft = 0;

    burstTimer = 0.0f;
    burstInterval = 0.08f;

    std::cout << "Rabbit: Linh thu THO XA THU DUAL-PISTOLS khoi tao hoan tat!" << std::endl;
}

void Rabbit::update(float dt, const std::vector<std::vector<int>>& tiles, int tileSize, sf::RenderWindow& window)
{
    // Gọi hàm update của lớp cha Player để xử lý di chuyển và hệ thống
    Player::update(dt, tiles, tileSize, window);

    // Cập nhật bộ đếm thời gian trạng thái Overclock (Ultimate)
    this->updateOverclock(dt);
}

void Rabbit::updateOverclock(float dt)
{
    if (this->isOverclocked)
    {
        this->overclockTimer -= dt;

        // ÉP MÀU HỒNG NEON khi đang trong trạng thái Ultimate
        this->shape.setFillColor(sf::Color(255, 20, 147));

        if (this->overclockTimer <= 0.0f)
        {
            this->isOverclocked = false;
            this->overclockTimer = 0.0f;

            // Hết chiêu cuối: Trả lại chỉ số chạy, công phá ban đầu và phục hồi màu xanh mặc định
            this->speed = this->originalSpeed;
            this->baseDamage = this->originalDamage;
            this->shape.setFillColor(sf::Color(0, 191, 255));

            std::cout << "==> OVERCLOCK MODE ENDED! Tra lai trang thai mac dinh cho Tho." << std::endl;
        }
    }
}

// --- KỸ NĂNG 1: RAPID DASH (Q) ---
void Rabbit::skillQ(sf::Vector2f direction)
{
    // ===== DASH THEO CHUỘT =====
    sf::Vector2i mousePixelPos = sf::Mouse::getPosition();
    sf::Vector2f mousePos(
        static_cast<float>(mousePixelPos.x),
        static_cast<float>(mousePixelPos.y)
    );
    sf::Vector2f dashDir =
        mousePos - this->shape.getPosition();
    float dashLength =
        std::sqrt(dashDir.x * dashDir.x +
            dashDir.y * dashDir.y);
    if (dashLength != 0)
    {
        dashDir /= dashLength;
        float dashDistance = 160.0f;
        sf::Vector2f newPos =
            this->shape.getPosition()
            + dashDir * dashDistance;
        this->shape.setPosition(newPos);
        this->targetPosition = newPos;
        this->isMoving = false;
    }

    // ===== KÍCH HOẠT BURST =====
    sf::Vector2f newDirection =
        lockedTargetPos - this->shape.getPosition();

    float dirLength =
        std::sqrt(
            newDirection.x * newDirection.x +
            newDirection.y * newDirection.y
        );

    if (dirLength != 0)
    {
        newDirection /= dirLength;
    }
    burstDirection = newDirection;
    isBurstFiring = true;
    burstShotsLeft = 5;
    burstTimer = 0.0f;
}

// --- KỸ NĂNG 2: BULLET STORM (W) ---
void Rabbit::skillW(sf::Vector2f direction,
    const std::vector<std::vector<int>>& tiles,
    int tileSize)
{
    std::cout << "-> Rabbit xala BULLET STORM (Xao quet hinh quat)!" << std::endl;

    static sf::Texture dummyTexture;
    sf::Vector2f baseDir(1.0f, 0.0f);

    if (this->hasLockedTarget) {
        baseDir = this->lockedTargetPos - this->shape.getPosition();
    }

    float baseAngle = std::atan2(baseDir.y, baseDir.x);
    float angleSpread = 15.0f * 3.14159265f / 180.0f;

    for (int i = -2; i <= 2; i++) {
        float currentAngle = baseAngle + (i * angleSpread);
        sf::Vector2f finalDir(std::cos(currentAngle), std::sin(currentAngle));

        this->projectiles.emplace_back(this->shape.getPosition(), finalDir, 700.0f, this->baseDamage * 0.6f, dummyTexture);
    }
}

// --- KỸ NĂNG 3: BOUNCE SHOT (E) ---
void Rabbit::skillE(sf::Vector2f direction) {
    std::cout << "-> Rabbit khai hoa BOUNCE SHOT (Dan xuyen thau)!" << std::endl;

    static sf::Texture dummyTexture;
    sf::Vector2f targetDir(1.0f, 0.0f);

    if (this->hasLockedTarget) {
        targetDir = this->lockedTargetPos - this->shape.getPosition();
    }

    // Đã đồng bộ số tham số phù hợp với hàm dựng Bullet cơ bản của bạn
    this->projectiles.emplace_back(
        this->shape.getPosition(),
        targetDir,
        600.0f,
        this->baseDamage * 1.6f,
        dummyTexture,
        9999,
        BulletType::BOUNCE
    );
}

// --- KỸ NĂNG CUỐI: OVERCLOCK MODE (R) ---
void Rabbit::skillR(sf::Vector2f direction) {
    std::cout << "==> OVERCLOCK MODE ACTIVATED !!! <==" << std::endl;

    float duration = 6.0f;

    if (!this->isOverclocked) {
        this->originalSpeed = this->speed;
        this->originalDamage = this->baseDamage;

        this->speed += 250.0f; // Kích tốc
        this->baseDamage *= 1.4f; // Kích công
        this->isOverclocked = true;
    }

    this->overclockTimer = duration;
    this->shape.setFillColor(sf::Color(255, 20, 147));
}

void Rabbit::updateBurstFire(float dt)
{
    if (!isBurstFiring)
        return;

    burstTimer += dt;

    if (burstTimer >= burstInterval)
    {
        burstTimer = 0.0f;

        static sf::Texture dummyTexture;

        Bullet bullet(
            this->shape.getPosition(),
            burstDirection,
            450.0f,
            this->baseDamage * 0.35f,
            dummyTexture
        );

        this->projectiles.push_back(bullet);

        burstShotsLeft--;

        if (burstShotsLeft <= 0)
        {
            isBurstFiring = false;
        }
    }
}