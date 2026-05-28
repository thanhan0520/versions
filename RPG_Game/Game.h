#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Player.h"
#include "Map.h"
#include "qvCc.h"
#include "Rabbit.h"
#include "Snake.h"

struct RootedEffect {
    size_t enemyIndex;
    float duration;
};

class Game
{
public:
    Game(sf::RenderWindow& refWindow);
    ~Game();

    void setPlayer(Player* chosenPlayer) {
        this->player = chosenPlayer;
    }

    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void spawnEnemies();
    void checkProjectileCollisions();
    void selectTarget(sf::Vector2f mouseWorldPos);

    sf::RenderWindow& window;
    sf::View camera;
    sf::Clock clock;
    Map map;

    Player* player;
    std::vector<qvCc> enemies;
    int selectedEnemyIndex;

    std::vector<RootedEffect> rootedEnemies;
};