#include "Game.h"
#include <iostream>
#include <cstdlib>

Game::Game()
    : window(sf::VideoMode(1024, 768), "RPG Dungeon Explorer")
{
    window.setFramerateLimit(60);
    map.load("map.txt");
    camera.reset(sf::FloatRect(0, 0, 1920, 768));
    window.setView(camera);
    spawnEnemies();
    selectedEnemyIndex = -1;
}

void Game::spawnEnemies()
{
    // Spawn 5 quái vật tại vị trí ngẫu nhiên với tốc độ giảm (150 → 80)
    for (int i = 0; i < 5; i++)
    {
        float x = 100 + rand() % 400;
        float y = 100 + rand() % 300;
        enemies.emplace_back(sf::Vector2f(x, y), 30.0f, 10.0f, 80.0f);
    }
    std::cout << "Spawned " << enemies.size() << " enemies with reduced speed!" << std::endl;
}

void Game::checkProjectileCollisions()
{
    auto& projectiles = player.getProjectiles();

    for (auto& proj : projectiles)
    {
        if (!proj.isAlive()) continue;

        for (auto& enemy : enemies)
        {
            if (!enemy.isAlive()) continue;

            if (proj.getGlobalBounds().intersects(enemy.getGlobalBounds()))
            {
                enemy.takeDamage(proj.getDamage());
                proj.setAlive(false);
                std::cout << "Hit! Enemy health: " << enemy.getHealth() << std::endl;
                break;
            }
        }
    }
}

void Game::selectTarget(sf::Vector2f mouseWorldPos)
{
    selectedEnemyIndex = -1;
    player.clearTarget();

    for (int i = 0; i < enemies.size(); i++)
    {
        if (enemies[i].isAlive() && enemies[i].getGlobalBounds().contains(mouseWorldPos))
        {
            selectedEnemyIndex = i;
            // Set locked target trên player
            player.setTargetEnemy(enemies[i].getPosition());
            std::cout << "Target locked: Enemy " << i << " at (" << enemies[i].getPosition().x 
                << ", " << enemies[i].getPosition().y << ")" << std::endl;
            return;
        }
    }
}

void Game::run()
{
    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        processEvents();
        update(dt);
        render();
    }
}

void Game::processEvents()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            window.close();
        else if (event.type == sf::Event::MouseButtonPressed)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2i mousePixelPos(event.mouseButton.x, event.mouseButton.y);
                sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixelPos);
                selectTarget(mouseWorldPos);
            }
        }
    }
}

void Game::update(float dt)
{
    player.update(dt, map.getTiles(), map.getTileSize(), window);

    // Cập nhật enemies
    for (auto& enemy : enemies)
    {
        if (enemy.isAlive())
        {
            enemy.update(dt, map.getTiles(), map.getTileSize(), player.getPosition());
        }
    }

    // Cập nhật vị trí target được lock (theo dõi mục tiêu khi nó di chuyển)
    if (selectedEnemyIndex >= 0 && selectedEnemyIndex < (int)enemies.size() && enemies[selectedEnemyIndex].isAlive())
    {
        player.setTargetEnemy(enemies[selectedEnemyIndex].getPosition());
    }

    // Kiểm tra va chạm projectile - enemy
    checkProjectileCollisions();

    // Xóa enemies đã chết
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const qvCc& e) { return !e.isAlive(); }),
        enemies.end()
    );

    // Nếu mục tiêu chết, bỏ chọn
    if (selectedEnemyIndex >= 0 && selectedEnemyIndex >= (int)enemies.size())
    {
        selectedEnemyIndex = -1;
        player.clearTarget();
    }

    // Spawn enemies mới khi tất cả đã chết
    if (enemies.empty())
    {
        std::cout << "All enemies defeated! Spawning new ones..." << std::endl;
        selectedEnemyIndex = -1;
        player.clearTarget();
        spawnEnemies();
    }

    // THÊM: Camera follow player
    sf::Vector2f playerPos = player.getPosition();
    camera.setCenter(playerPos);
    window.setView(camera);
}

void Game::render()
{
    window.clear(sf::Color::Black);
    map.draw(window);
    player.draw(window);
    player.drawProjectiles(window);

    // Vẽ enemies
    for (int i = 0; i < enemies.size(); i++)
    {
        enemies[i].draw(window);

        // Vẽ highlight cho mục tiêu được chọn
        if (i == selectedEnemyIndex && enemies[i].isAlive())
        {
            sf::CircleShape targetRing(30);
            targetRing.setFillColor(sf::Color::Transparent);
            targetRing.setOutlineThickness(2.0f);
            targetRing.setOutlineColor(sf::Color::Yellow);
            sf::Vector2f enemyPos = enemies[i].getPosition();
            targetRing.setPosition(enemyPos.x - 30, enemyPos.y - 30);
            window.draw(targetRing);
        }
    }

    window.display();
}