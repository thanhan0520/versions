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

    // Save/Load full game state as a simple serialized string
    std::string serializeState() const;
    bool restoreState(const std::string& serialized);

    int getEnemyCount() const { return static_cast<int>(enemies.size()); }
    void setAuthManager(class AuthManager* mgr) { authManager = mgr; }
    void setGameOver(bool v) { isGameOver = v; }
    bool getGameOver() const { return isGameOver; }

private:
    class AuthManager* authManager;
    float autosaveTimer;
    float autosaveInterval;
    bool isGameOver;
    bool overlayMouseDown;
    sf::FloatRect gameOverBtnNew;
    sf::FloatRect gameOverBtnRevive;
    void processEvents();
    void update(float dt);
    void renderGameOver();
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