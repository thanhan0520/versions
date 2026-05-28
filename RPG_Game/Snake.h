#pragma once
#include "Player.h"
#include <vector>

class Snake : public Player
{
public:
    Snake();
    virtual ~Snake() {}
    void update(float dt, const std::vector<std::vector<int>>& tiles, int tileSize, sf::RenderWindow& window) override;
    void updateAwakening(float dt);
    void skillQ(sf::Vector2f direction) override;
    void skillW(sf::Vector2f direction,
        const std::vector<std::vector<int>>& tiles,
        int tileSize) override;

    void skillE(sf::Vector2f direction) override;

    void skillR(sf::Vector2f direction) override;

private:
    bool isAwakened;
    float awakeningTimer;
    float originalBaseDamage;
};