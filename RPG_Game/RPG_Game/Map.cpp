#include "Map.h"
#include <fstream>
#include <iostream>

Map::Map()
{
    tileShape.setSize(sf::Vector2f(tileSize, tileSize));
    // Tạo texture tạm (sau này bạn thay bằng tileset thật)
    tileTexture.create(tileSize, tileSize);
    tileShape.setTexture(&tileTexture);
}

void Map::load(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cout << "Không thể mở file map: " << filename << std::endl;
        return;
    }

    int width, height;
    file >> width >> height;
    mapSize = sf::Vector2u(width, height);

    tiles.resize(height, std::vector<int>(width));

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            file >> tiles[y][x];
        }
    }

    // Tạo màu sắc cho các loại tile
    // 0 = đường đi (xám), 1 = tường (đỏ), 2 = cửa (xanh)
    sf::Image img;
    img.create(tileSize, tileSize, sf::Color::White);

    for (int y = 0; y < tileSize; y++)
    {
        for (int x = 0; x < tileSize; x++)
        {
            img.setPixel(x, y, sf::Color(100, 100, 100)); // màu xám mặc định
        }
    }
    tileTexture.loadFromImage(img);
}

void Map::draw(sf::RenderWindow& window)
{
    for (size_t y = 0; y < tiles.size(); y++)
    {
        for (size_t x = 0; x < tiles[y].size(); x++)
        {
            // Chỉ vẽ những tile trong màn hình (optimization)
            sf::Vector2f worldPos(x * tileSize, y * tileSize);
            sf::Vector2f viewPos = window.getView().getCenter() - window.getView().getSize() / 2.0f;

            if (worldPos.x + tileSize > viewPos.x - tileSize &&
                worldPos.x < viewPos.x + window.getView().getSize().x + tileSize &&
                worldPos.y + tileSize > viewPos.y - tileSize &&
                worldPos.y < viewPos.y + window.getView().getSize().y + tileSize)
            {
                // Đặt màu theo loại tile
                if (tiles[y][x] == 0) // đường đi
                    tileShape.setFillColor(sf::Color(150, 150, 150));
                else if (tiles[y][x] == 1) // tường
                    tileShape.setFillColor(sf::Color(139, 69, 19)); // nâu
                else if (tiles[y][x] == 2) // cửa
                    tileShape.setFillColor(sf::Color(0, 255, 0)); // xanh lá

                tileShape.setPosition(worldPos);
                window.draw(tileShape);
            }
        }
    }
}

bool Map::checkCollision(const sf::FloatRect& rect)
{
    // Tính toán ô bị ảnh hưởng bởi hitbox
    int leftTile = rect.left / tileSize;
    int rightTile = (rect.left + rect.width) / tileSize;
    int topTile = rect.top / tileSize;
    int bottomTile = (rect.top + rect.height) / tileSize;

    for (int y = topTile; y <= bottomTile; y++)
    {
        if (y < 0 || y >= (int)tiles.size()) return true;

        for (int x = leftTile; x <= rightTile; x++)
        {
            if (x < 0 || x >= (int)tiles[0].size()) return true;

            if (tiles[y][x] == 1) // va chạm với tường
                return true;
        }
    }
    return false;
}