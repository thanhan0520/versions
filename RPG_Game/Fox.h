#pragma once
#include "Player.h"
#include <vector>

class Fox : public Player
{
public:
    Fox();
    virtual ~Fox() {}

    // Khai báo ghi đè hàm update khớp chuẩn tham số
    void update(float dt, const std::vector<std::vector<int>>& tiles, int tileSize, sf::RenderWindow& window) override;

    void skillQ(sf::Vector2f direction) override;
    void skillW(sf::Vector2f direction,
        const std::vector<std::vector<int>>& tiles,
        int tileSize) override;

    void skillE(sf::Vector2f direction) override;

    void skillR(sf::Vector2f direction) override;

private:
    bool isInvulnerable;
    float originalSpeed;
};