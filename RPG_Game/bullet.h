#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <algorithm>

enum class BulletType {
    NORMAL,
    BOUNCE,
    FOX_Q,
    FOX_W,
    FOX_R,
    SNAKE_Q,        // Venom Strike (Roi xích co duỗi)
    SNAKE_W,        // Poison Fog (Vùng sương độc)
    SNAKE_E,        // Snake Chain (Xích khống chế)
    SNAKE_R,        // Cobra Ultimate (Kết giới trận pháp xích)
    // ====================================================================
    // THÊM CÁC KIỂU ĐÒN ĐÁNH / CHIÊU THỨC DIỆN RỘNG CỦA NHÂN VẬT CHÓ (DOG)
    // ====================================================================
    DOG_Q,          // Heavy Charge (Vùng bọc va chạm/hất tung khi lao vai)
    DOG_W,          // Earth Smash (Sóng chấn động mặt đất khi đập đại kiếm)
    DOG_R_SHOCKWAVE // Berserk Roar (Rung chấn lan tỏa tạo ra trên mỗi đòn đánh)
};

enum class ChainState {
    FLYING_OUT,     // Đang vung ra ngoài
    RETRACTING,     // Đang thu hồi về tay nhân vật
    PINNED          // Đang ghim chặt vào mục tiêu (Chiêu E)
};

class Bullet
{
public:
    Bullet(sf::Vector2f startPos, sf::Vector2f dir, float moveSpeed, float dmg, const sf::Texture& texture, int maxBounces = 0, BulletType type = BulletType::NORMAL);

    void update(float dt);
    void updateSnakeChain(float dt, sf::Vector2f playerPos);
    void followPlayer(sf::Vector2f playerPos);
    void setBulletSize(sf::Vector2f size);
    void setBulletColor(sf::Color color);
    void draw(sf::RenderWindow& window);
    void drawChain(sf::RenderWindow& window, sf::Vector2f playerPos);

    sf::FloatRect getGlobalBounds() const;
    sf::Vector2f getPosition() const { return position; }
    void setPosition(sf::Vector2f newPos);
    float getDamage() const { return damage; }
    bool isAlive() const { return alive; }
    void setAlive(bool state) { alive = state; }
    BulletType getType() const { return type; }
    int getBounceCount() const { return bounceCount; }
    void decreaseBounce();
    void setDirection(sf::Vector2f newDir);

    void addHitTarget(void* targetAddress);
    bool hasHitTarget(void* targetAddress) const;

    // Các hàm getter/setter phục vụ riêng cho class Snake
    ChainState getChainState() const { return chainState; }
    void setChainState(ChainState state) { chainState = state; }
    int getPinnedEnemyIndex() const { return pinnedEnemyIndex; }
    void setPinnedEnemyIndex(int idx) { pinnedEnemyIndex = idx; }
    float getUltTimer() const { return ultTimer; }
    bool hasTriggeredRoot() const { return triggeredRoot; }
    void setTriggeredRoot(bool val) { triggeredRoot = val; }
    float getRadius() const { return radius; }

protected:
    sf::RectangleShape shape;
    sf::ConvexShape arcShape;
    sf::Vector2f position;
    sf::Vector2f direction;
    float speed;
    float damage;
    bool alive;
    int bounceCount;
    BulletType type;
    std::vector<void*> hitTargets;

    float lifetime;
    float maxLifetime;

    // Thuộc tính nâng cao dành cho cơ chế xích của Rắn
    ChainState chainState;
    sf::Vector2f startPosition;
    float maxRange;
    int pinnedEnemyIndex;
    float ultTimer;
    bool triggeredRoot;
    float radius;
};