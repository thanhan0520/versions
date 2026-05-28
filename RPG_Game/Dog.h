#pragma once
#include "Player.h"
#include <vector>

class Dog : public Player
{
public:
    Dog();

    // Khai báo ghi đè hàm update khớp chuẩn tham số
    void update(float dt, const std::vector<std::vector<int>>& tiles, int tileSize, sf::RenderWindow& window) override;

    bool isIronSkinActive() const { return ironSkinTimer > 0.0f; }
    bool isBerserkActive() const { return berserkTimer > 0.0f; }

    void updateDogTimers(float dt);
    void setMousePosForCharge(sf::Vector2f mousePos) { currentMousePos = mousePos; }

protected:
    void skillQ(sf::Vector2f direction) override;
    void skillW(sf::Vector2f direction,
        const std::vector<std::vector<int>>& tiles,
        int tileSize) override;

    void skillE(sf::Vector2f direction) override;

    void skillR(sf::Vector2f direction) override;

private:
    float ironSkinTimer;
    float berserkTimer;

    bool isCharging;
    sf::Vector2f chargeDir;
    float chargeSpeed;
    float chargeDuration;
    float chargeTimer;

    sf::Vector2f currentMousePos;
};