#include "Bullet.h"

Bullet::Bullet(sf::Vector2f startPos, sf::Vector2f dir, float moveSpeed, float dmg, const sf::Texture& texture, int maxBounces, BulletType type)
    : position(startPos), speed(moveSpeed), damage(dmg), alive(true), bounceCount(maxBounces), type(type), lifetime(0.0f)
{
    chainState = ChainState::FLYING_OUT;
    startPosition = startPos;
    maxRange = 320.0f;
    pinnedEnemyIndex = -1;
    ultTimer = 0.0f;
    triggeredRoot = false;
    radius = 160.0f;
    maxLifetime = -1.0f; // Mặc định -1 tức là không giới hạn thời gian (dựa vào alive)

    float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (length != 0) { direction = dir / length; }
    else { direction = sf::Vector2f(1.0f, 0.0f); }

    float angle = std::atan2(direction.y, direction.x) * 180.0f / 3.14159265f;

    if (type == BulletType::FOX_Q) {
        arcShape.setPointCount(6);
        arcShape.setPoint(0, sf::Vector2f(0, 0));
        arcShape.setPoint(1, sf::Vector2f(50, -80));
        arcShape.setPoint(2, sf::Vector2f(120, -40));
        arcShape.setPoint(3, sf::Vector2f(140, 0));
        arcShape.setPoint(4, sf::Vector2f(120, 40));
        arcShape.setPoint(5, sf::Vector2f(50, 80));

        arcShape.setFillColor(sf::Color(255, 69, 0, 230));
        arcShape.setOutlineThickness(3.0f);
        arcShape.setOutlineColor(sf::Color(255, 215, 0));
        arcShape.setOrigin(0, 0);
        arcShape.setRotation(angle);
        arcShape.setPosition(position);
        maxLifetime = 0.25f;
    }
    else if (type == BulletType::FOX_W) {
        shape.setSize(sf::Vector2f(50.0f, 50.0f));
        shape.setFillColor(sf::Color(169, 169, 169, 180));
        shape.setOrigin(25.0f, 25.0f);
        shape.setPosition(position);
    }
    else if (type == BulletType::FOX_R) {
        shape.setSize(sf::Vector2f(46.0f, 46.0f));
        shape.setFillColor(sf::Color(0, 255, 255, 120));
        shape.setOrigin(23.0f, 23.0f);
        shape.setRotation(angle);
        shape.setPosition(position);
    }
    else if (type == BulletType::SNAKE_Q) {
        shape.setSize(sf::Vector2f(25.0f, 10.0f));
        shape.setFillColor(sf::Color(50, 205, 50));
        shape.setOrigin(12.5f, 5.0f);
        shape.setRotation(angle);
        shape.setPosition(position);
    }
    else if (type == BulletType::SNAKE_W) {
        shape.setSize(sf::Vector2f(120.0f, 120.0f));
        shape.setFillColor(sf::Color(144, 238, 144, 100));
        shape.setOrigin(60.0f, 60.0f);
        shape.setPosition(position);
    }
    else if (type == BulletType::SNAKE_E) {
        shape.setSize(sf::Vector2f(20.0f, 20.0f));
        shape.setFillColor(sf::Color(128, 0, 128));
        shape.setOrigin(10.0f, 10.0f);
        shape.setRotation(angle);
        shape.setPosition(position);
    }
    // ====================================================================
    // THÊM THIẾT LẬP ĐỒ HỌA CHO BỘ KỸ NĂNG CỦA CHÓ
    // ====================================================================
    else if (type == BulletType::DOG_Q) {
        shape.setSize(sf::Vector2f(55.0f, 55.0f));
        shape.setFillColor(sf::Color(255, 140, 0, 80)); // Màu cam lửa mờ bọc quanh vai
        shape.setOrigin(27.5f, 27.5f);
        shape.setPosition(position);
        maxLifetime = 0.4f; // Biến mất sau khi hết hoạt ảnh lướt
    }
    else if (type == BulletType::DOG_W) {
        shape.setSize(sf::Vector2f(150.0f, 150.0f));
        shape.setFillColor(sf::Color(139, 69, 19, 140)); // Màu nâu đất đại diện cho sóng chấn động
        shape.setOrigin(75.0f, 75.0f);
        shape.setPosition(position);
        maxLifetime = 0.25f; // Tan biến nhanh sau khi nện đất
    }
    else if (type == BulletType::DOG_R_SHOCKWAVE) {
        shape.setSize(sf::Vector2f(70.0f, 70.0f));
        shape.setFillColor(sf::Color(178, 34, 34, 160)); // Rung chấn đỏ rực khi đánh thường
        shape.setOrigin(35.0f, 35.0f);
        shape.setPosition(position);
        maxLifetime = 0.15f; // Xuất hiện giật chớp nhoáng
    }
    else {
        shape.setSize(sf::Vector2f(8.0f, 8.0f));
        shape.setFillColor(sf::Color::Yellow);
        shape.setOrigin(4.0f, 4.0f);
        shape.setPosition(position);
    }
}

void Bullet::update(float dt) {
    if (!alive) return;
    lifetime += dt;

    // Quản lý thời gian tự hủy cho các kỹ năng có giới hạn thời gian tồn tại (maxLifetime)
    if (maxLifetime > 0.0f && lifetime >= maxLifetime) {
        alive = false;
        return;
    }

    if (type == BulletType::FOX_Q) {
        float scaleFactor = 0.4f + (lifetime / maxLifetime) * 0.8f;
        arcShape.setScale(scaleFactor, scaleFactor);
    }
    else if (type == BulletType::SNAKE_R) {
        ultTimer += dt;
    }
    // Chặn không cho các chiêu thức đứng yên/vùng tác dụng di chuyển tịnh tiến
    else if (type != BulletType::FOX_W && type != BulletType::SNAKE_W &&
        type != BulletType::SNAKE_Q && type != BulletType::SNAKE_E &&
        type != BulletType::DOG_W && type != BulletType::DOG_R_SHOCKWAVE)
    {
        position += direction * speed * dt;
        shape.setPosition(position);
    }
}

void Bullet::updateSnakeChain(float dt, sf::Vector2f playerPos) {
    if (!alive) return;

    if (chainState == ChainState::FLYING_OUT) {
        position += direction * speed * dt;
        shape.setPosition(position);

        float dist = std::sqrt(std::pow(position.x - startPosition.x, 2) + std::pow(position.y - startPosition.y, 2));
        if (dist >= maxRange) {
            chainState = ChainState::RETRACTING;
        }
    }
    else if (chainState == ChainState::RETRACTING) {
        sf::Vector2f toPlayer = playerPos - position;
        float dist = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

        if (dist <= 25.0f) {
            alive = false;
        }
        else {
            sf::Vector2f dir = toPlayer / dist;
            position += dir * speed * dt;
            shape.setPosition(position);
        }
    }
}

void Bullet::followPlayer(sf::Vector2f playerPos) {
    if (type == BulletType::FOX_Q || type == BulletType::DOG_Q) {
        position = playerPos;
        shape.setPosition(position);
    }
}

void Bullet::setBulletSize(sf::Vector2f size) {
    if (type != BulletType::FOX_Q) {
        shape.setSize(size);
        shape.setOrigin(size.x / 2.0f, size.y / 2.0f);
    }
}

void Bullet::setBulletColor(sf::Color color) {
    if (type != BulletType::FOX_Q) {
        shape.setFillColor(color);
    }
}

void Bullet::draw(sf::RenderWindow& window) {
    if (!alive) return;

    if (type == BulletType::FOX_Q) {
        window.draw(arcShape);
    }
    else if (type == BulletType::SNAKE_R) {
        sf::CircleShape zone(radius);
        zone.setOrigin(radius, radius);
        zone.setPosition(position);

        if (ultTimer < 3.0f) {
            zone.setFillColor(sf::Color(0, 255, 64, 35));
            zone.setOutlineThickness(2.5f);
            zone.setOutlineColor(sf::Color(50, 205, 50, 150));
        }
        else {
            zone.setFillColor(sf::Color(139, 0, 139, 60));
            zone.setOutlineThickness(3.5f);
            zone.setOutlineColor(sf::Color::Magenta);
        }
        window.draw(zone);
    }
    else {
        window.draw(shape);
    }
}

void Bullet::drawChain(sf::RenderWindow& window, sf::Vector2f playerPos) {
    if (!alive) return;

    sf::VertexArray chainLines(sf::Lines, 2);
    chainLines[0].position = playerPos;
    chainLines[0].color = sf::Color(107, 142, 35);
    chainLines[1].position = position;
    chainLines[1].color = sf::Color(50, 205, 50);

    window.draw(chainLines);
    draw(window);
}

sf::FloatRect Bullet::getGlobalBounds() const {
    return (type == BulletType::FOX_Q) ? arcShape.getGlobalBounds() : shape.getGlobalBounds();
}

void Bullet::setPosition(sf::Vector2f newPos) {
    position = newPos;
    shape.setPosition(position);
}

void Bullet::decreaseBounce() {
    if (bounceCount > 0) bounceCount--;
}

void Bullet::setDirection(sf::Vector2f newDir)
{
    float len =
        std::sqrt(
            newDir.x * newDir.x +
            newDir.y * newDir.y
        );

    if (len > 0)
    {
        direction = newDir / len;
    }
}

void Bullet::addHitTarget(void* targetAddress) {
    hitTargets.push_back(targetAddress);
}

bool Bullet::hasHitTarget(void* targetAddress) const {
    return std::find(hitTargets.begin(), hitTargets.end(), targetAddress) != hitTargets.end();
}


void Bullet::clearHitTargets()
{
    hitTargets.clear();
}