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
    bool isPaused() const { return paused; }
    bool shouldReturnToMain() const { return returnToMain; }
    int getScore() const { return score; }
    void setScore(int s) { score = s; }

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
    int score = 0; // player's current score (points from defeated enemies)
    void addScore(int delta);
    std::vector<qvCc> enemies;
    int selectedEnemyIndex;

    std::vector<RootedEffect> rootedEnemies;
    struct Potion {
        sf::Vector2f pos;
        float healAmount;
        bool alive;
        sf::CircleShape shape;
        Potion() : pos(0,0), healAmount(0), alive(true) {}
    };

    std::vector<Potion> potions;
    std::vector<bool> enemyCounted; // tracks whether a dead enemy has been scored
    void spawnPotion(sf::Vector2f pos, float healAmount);
    const float potionDropRate = 0.25f; // 25% chance
    const float potionHealFraction = 0.35f; // restores 35% of max HP

    // Pause state and UI
    bool paused = false;
    bool returnToMain = false;
    bool lastEscPressed = false;
    sf::FloatRect pauseBtnResume;
    sf::FloatRect pauseBtnSave;
    sf::FloatRect pauseBtnReturn;
    sf::FloatRect pauseBtnReturnMain;
    sf::FloatRect pauseBtnExit;
    void renderPauseMenu();
    // HUD pause button
    sf::FloatRect pauseHudRect;
};