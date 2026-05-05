#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Player.h"
#include "Map.h"
#include "qvCc.h"

class Game
{
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void spawnEnemies();
    void checkProjectileCollisions();
    void selectTarget(sf::Vector2f mouseWorldPos);

    sf::RenderWindow window;
    Player player;
    Map map;
    sf::Clock clock;
    sf::View camera;
    std::vector<qvCc> enemies;
    int selectedEnemyIndex;  // -1 = không có mục tiêu
};