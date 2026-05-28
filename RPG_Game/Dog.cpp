#include "Dog.h"
#include <cmath>
#include <iostream>

Dog::Dog() : Player()
{
    // Định danh hệ phái Chó theo Enum trong Player.h
    currentClass = CharacterClass::DOG;

    // Thiết lập chỉ số Đấu Sĩ hạng nặng (HP: 150, DEF cao, SPD thấp)
    this->maxHealth = 150.0f;
    this->health = 150.0f;
    this->speed = 110.0f;
    this->baseDamage = 25.0f; // Sát thương đại kiếm cơ bản lớn

    // Thiết lập thời gian hồi chiêu mặc định cho các kỹ năng (Ví dụ)
    this->skillQ_cooldownMax = 6.0f;
    this->skillW_cooldownMax = 8.0f;
    this->skillE_cooldownMax = 12.0f;
    this->skillR_cooldownMax = 20.0f;

    // Cấu hình thuộc tính bổ trợ nội bộ
    ironSkinTimer = 0.0f;
    berserkTimer = 0.0f;
    isCharging = false;
    chargeSpeed = 550.0f;
    chargeDuration = 0.35f;
    chargeTimer = 0.0f;

    // Đồ họa: Hình vuông lớn màu vàng sẫm đặc trưng
    shape.setSize(sf::Vector2f(45.0f, 45.0f));
    shape.setFillColor(sf::Color(218, 165, 32));
    shape.setOrigin(22.5f, 22.5f);
}

// ====================================================================
// THÊM MỚI: HÀM UPDATE CHÍNH CỦA ĐỐI TƯỢNG DOG
// ====================================================================
void Dog::update(float dt, const std::vector<std::vector<int>>& tiles, int tileSize, sf::RenderWindow& window)
{
    // CỰC KỲ QUAN TRỌNG: Gọi hàm update của lớp cha Player để xử lý di chuyển, 
    // va chạm tường và cập nhật đạn/cooldown.
    Player::update(dt, tiles, tileSize, window);

    // Tích hợp logic xử lý tăng/giảm sát thương dựa theo bùa lợi Đang hoạt động
    if (ironSkinTimer > 0.0f) {
        this->defenseBoost = 0.5f; // Giảm 50% sát thương nhận vào khi bật Iron Skin
    }
    else {
        this->defenseBoost = 0.0f;
    }

    if (berserkTimer > 0.0f) {
        this->baseDamage = 40.0f; // Tăng mạnh sát thương khi hóa điên
        this->speed = 160.0f;     // Tăng tốc chạy chân ngắn
    }
    else {
        this->baseDamage = 25.0f; // Trở về chỉ số gốc
        if (!isCharging) {
            this->speed = 110.0f;
        }
    }
}

void Dog::updateDogTimers(float dt)
{
    // Giảm thời gian duy trì trạng thái buff
    if (ironSkinTimer > 0.0f) ironSkinTimer -= dt;
    if (berserkTimer > 0.0f)  berserkTimer -= dt;

    // Đổi màu sắc ngoại trang dựa trên trạng thái buff hiện tại
    if (ironSkinTimer > 0.0f) {
        shape.setOutlineThickness(3.0f);
        shape.setOutlineColor(sf::Color::White); // Viền trắng cứng cáp
    }
    else if (berserkTimer > 0.0f) {
        shape.setOutlineThickness(3.0f);
        shape.setOutlineColor(sf::Color::Red);   // Viền đỏ rực cuồng nộ
        shape.setFillColor(sf::Color(139, 0, 0));
    }
    else {
        shape.setOutlineThickness(0.0f);
        shape.setFillColor(sf::Color(218, 165, 32)); // Trở về bình thường
    }

    // Logic tự động di chuyển tịnh tiến lên phía trước khi đang trong trạng thái lao vai (Q)
    if (isCharging) {
        chargeTimer += dt;
        sf::Vector2f movement = chargeDir * chargeSpeed * dt;
        shape.move(movement);

        if (chargeTimer >= chargeDuration) {
            isCharging = false;
        }
    }
}

// Kỹ năng 1: Heavy Charge (Lao mạnh về phía trước bằng vai)
void Dog::skillQ(sf::Vector2f direction)
{
    if (isCharging) return;

    // Tính toán hướng lướt dựa trên hướng bắn truyền từ hệ thống
    float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (len > 0.0f) {
        chargeDir = direction / len;
        isCharging = true;
        chargeTimer = 0.0f;

        // Tạo ra một vùng bọc va chạm (Bullet) ngay tại tâm người Chó để đi càn quét quái vật
        Bullet chargeHitbox(shape.getPosition(), chargeDir, chargeSpeed, baseDamage * 0.8f, bulletTexture, 0, BulletType::DOG_Q);
        chargeHitbox.setBulletSize(sf::Vector2f(55.0f, 55.0f));
        projectiles.push_back(chargeHitbox);
    }
}

// Kỹ năng 2: Earth Smash (Đập đại kiếm xuống đất tạo chấn động diện rộng)
void Dog::skillW(sf::Vector2f direction,
    const std::vector<std::vector<int>>& tiles,
    int tileSize) 
{
    sf::Vector2f currentPos = shape.getPosition();
    sf::Vector2f dummyDir(0.0f, 0.0f);

    // Kích hoạt một thực thể Bullet đứng yên tại vị trí đập, thời gian hủy nhanh
    Bullet shockwave(currentPos, dummyDir, 0.0f, baseDamage * 1.4f, bulletTexture, 0, BulletType::DOG_W);
    shockwave.setBulletSize(sf::Vector2f(150.0f, 150.0f));
    projectiles.push_back(shockwave);
}

// Kỹ năng 3: Iron Skin (Tăng mạnh khả năng chống chịu)
void Dog::skillE(sf::Vector2f direction) {
    ironSkinTimer = 4.0f; // Duy trì trong 4 giây
}

// Kỹ năng cuối: Berserk Roar (Kích hoạt trạng thái cuồng nộ)
void Dog::skillR(sf::Vector2f direction)
{
    berserkTimer = 6.0f; // Duy trì trong 6 giây
}