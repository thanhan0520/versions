#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Map
{
public:
    Map();
    void load(const std::string& filename);
    void draw(sf::RenderWindow& window);
    bool checkCollision(const sf::FloatRect& rect);
    sf::Vector2u getMapSize() const { return mapSize; }

    // Thêm 2 phương thức này
    const std::vector<std::vector<int>>& getTiles() const { return tiles; }
    int getTileSize() const { return tileSize; }

private:
    std::vector<std::vector<int>> tiles;
    sf::Texture tileTexture;
    sf::RectangleShape tileShape;
    sf::Vector2u mapSize;
    int tileSize = 32;
};