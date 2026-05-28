#include "Game.h"
#include "Dog.h" 
#include "Fox.h"
#include "Snake.h"
#include "Rabbit.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <algorithm> 

Game::Game(sf::RenderWindow& sharedWindow)
    : window(sharedWindow), player(nullptr), selectedEnemyIndex(-1)
{
    window.setFramerateLimit(60);
    map.load("map.txt");
    camera.reset(sf::FloatRect(0, 0, 1920, 768));
    window.setView(camera);
    spawnEnemies();
}

Game::~Game()
{
}

void Game::spawnEnemies()
{
    enemies.clear();
    for (int i = 0; i < 10; i++)
    {
        float x = 150 + rand() % 600;
        float y = 100 + rand() % 500;
        enemies.emplace_back(sf::Vector2f(x, y), 30.0f, 10.0f, 80.0f);
    }
    std::cout << "Trieu hoi " << enemies.size() << " thanh cong" << std::endl;
}

void Game::checkProjectileCollisions()
{
    if (player == nullptr) return;

    auto& projectiles = player->getProjectiles();

    for (auto& proj : projectiles)
    {
        if (!proj.isAlive()) continue;
        BulletType pType = proj.getType();

        // 1. XỬ LÝ CHIÊU R CỦA RẮN (AOE - Vùng ảnh hưởng)
        if (pType == BulletType::SNAKE_R)
        {
            sf::Vector2f center = proj.getPosition();
            for (size_t i = 0; i < enemies.size(); ++i)
            {
                auto& enemy = enemies[i];
                if (!enemy.isAlive()) continue;

                float dist = std::sqrt(std::pow(enemy.getPosition().x - center.x, 2) +
                    std::pow(enemy.getPosition().y - center.y, 2));

                if (dist <= proj.getRadius())
                {
                    if (proj.getUltTimer() < 3.0f) {
                        enemy.applySlow(0.1f, 0.65f);
                        enemy.addPoisonStack(4.0f, 0.5f);
                    }
                    else if (proj.getUltTimer() >= 3.0f && !proj.hasTriggeredRoot()) {
                        enemy.takeDamage(proj.getDamage());
                        enemy.applySlow(2.5f, 1.0f);
                    }
                }
            }
            if (proj.getUltTimer() >= 3.1f) {
                proj.setTriggeredRoot(true);
                proj.setAlive(false);
            }
            continue;
        }

        // 2. XỬ LÝ CÁC LOẠI ĐẠN KHÁC
        for (size_t i = 0; i < enemies.size(); ++i)
        {
            auto& enemy = enemies[i];
            if (!enemy.isAlive()) continue;

            // Xử lý riêng cho Xích của Rắn (E)
            if (pType == BulletType::SNAKE_E && proj.getChainState() == ChainState::PINNED)
            {
                if (proj.getPinnedEnemyIndex() == static_cast<int>(i)) {
                    proj.setPosition(enemy.getPosition());
                    enemy.addPoisonStack(15.0f, 0.2f);
                }
                continue;
            }

            // Kiểm tra va chạm vật lý
            if (proj.getGlobalBounds().intersects(enemy.getGlobalBounds()))
            {
                // Kiểm tra xem đã từng trúng con quái này chưa (tránh mất máu liên tục trong 1 frame)
                if (!proj.hasHitTarget(&enemy))
                {
                    // --- GÂY SÁT THƯƠNG CHUNG ---
                    enemy.takeDamage(proj.getDamage());
                    proj.addHitTarget(&enemy);
                    std::cout << "-> Trung quai! Tru " << proj.getDamage() << " HP." << std::endl;

                    // --- CÁC HIỆU ỨNG RIÊNG ---
                    if (pType == BulletType::DOG_Q) enemy.applySlow(1.0f, 0.5f);
                    else if (pType == BulletType::FOX_W) {
                        // Logic trói chân (Rooted)
                        bool alreadyRooted = false;
                        for (auto& effect : rootedEnemies) {
                            if (effect.enemyIndex == i) { effect.duration = 2.5f; alreadyRooted = true; break; }
                        }
                        RootedEffect newEffect;
                        newEffect.enemyIndex = static_cast<int>(i);
                        newEffect.duration = 2.5f;
                        rootedEnemies.push_back(newEffect);
                        proj.setAlive(false);
                    }
                    else if (pType == BulletType::SNAKE_Q && proj.getChainState() == ChainState::FLYING_OUT) {
                        enemy.addPoisonStack(6.0f, 4.0f);
                        proj.setChainState(ChainState::RETRACTING);
                    }
                    else if (pType == BulletType::SNAKE_W) {
                        enemy.applySlow(0.6f, 0.45f);
                        enemy.addPoisonStack(3.0f, 2.0f);
                    }
                }

                // --- LOGIC XÓA ĐẠN ---
                if (pType != BulletType::FOX_W && pType != BulletType::SNAKE_W &&
                    pType != BulletType::DOG_W && pType != BulletType::DOG_R_SHOCKWAVE)
                {
                    if (pType == BulletType::BOUNCE && proj.getBounceCount() > 0) {
                        proj.decreaseBounce();
                    }
                    else {
                        proj.setAlive(false);
                    }
                }
                break; // Thoát vòng lặp quái để tránh trúng nhiều quái 1 lúc nếu không xuyên thấu
            }
        }
    }
}

void Game::selectTarget(sf::Vector2f mouseWorldPos)
{
    selectedEnemyIndex = -1;
    if (player != nullptr) player->clearTarget();

    for (int i = 0; i < enemies.size(); i++)
    {
        if (enemies[i].isAlive() && enemies[i].getGlobalBounds().contains(mouseWorldPos))
        {
            selectedEnemyIndex = i;
            if (player != nullptr) {
                player->setTargetEnemy(enemies[i].getPosition());
            }
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
    }
}

void Game::update(float dt)
{
    for (auto it = rootedEnemies.begin(); it != rootedEnemies.end();) {
        it->duration -= dt;
        if (it->duration <= 0 || it->enemyIndex >= enemies.size() || !enemies[it->enemyIndex].isAlive()) {
            it = rootedEnemies.erase(it);
        }
        else {
            ++it;
        }
    }

    sf::Vector2f playerPos = (player != nullptr)
        ? player->getPosition()
        : sf::Vector2f(0.0f, 0.0f);

    int closestEnemyIdx = -1;
    float minDistance = 999999.0f;

    for (int i = 0; i < enemies.size(); i++)
    {
        if (enemies[i].isAlive())
        {
            float dist = std::sqrt(
                std::pow(enemies[i].getPosition().x - playerPos.x, 2) +
                std::pow(enemies[i].getPosition().y - playerPos.y, 2)
            );

            if (dist < minDistance)
            {
                minDistance = dist;
                closestEnemyIdx = i;
            }
        }
    }

    if (player != nullptr)
    {
        if (closestEnemyIdx != -1)
        {
            player->setTargetEnemy(enemies[closestEnemyIdx].getPosition());
        }
    }

    if (player != nullptr) {

        // 1. CHẠY UPDATE ĐA HÌNH CHO NHÂN VẬT CHÍNH (Chỉ chạy khi sống)
        if (player->isAlive())
        {
            player->update(dt, map.getTiles(), map.getTileSize(), window);
        }
        else
        {
            // LOGIC KHI CHẾT: Nhấn nút R để hồi sinh đầy máu test tiếp
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
                spawnEnemies();
            }
        }

        // 2. CẬP NHẬT RIÊNG CHO HỆ CHÓ (Góc xoay chuột để lướt Q)
        Dog* dogPlayer = dynamic_cast<Dog*>(player);
        if (dogPlayer != nullptr && player->isAlive()) {
            sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixelPos);
            dogPlayer->setMousePosForCharge(mouseWorldPos);
            dogPlayer->updateDogTimers(dt);
        }

        Rabbit* rabbitPlayer = dynamic_cast<Rabbit*>(player);

        if (rabbitPlayer != nullptr && player->isAlive())
        {
            rabbitPlayer->updateBurstFire(dt);
        }

        // 3. CẬP NHẬT ĐẠN/KẾT GIỚI CỦA NGƯỜI CHƠI
        for (auto& proj : player->getProjectiles()) {
            if (proj.isAlive()) {
                BulletType type = proj.getType();
                if (type == BulletType::FOX_Q || type == BulletType::DOG_Q) {
                    proj.followPlayer(player->getPosition());
                }
                else if (type == BulletType::SNAKE_Q || type == BulletType::SNAKE_E) {
                    proj.updateSnakeChain(dt, player->getPosition());
                }
                else {
                    proj.update(dt);
                }
            }
        }

        // 🌟 SỬA LỖI TẠI ĐÂY: Gọi hàm xử lý va chạm của đạn sau khi đạn di chuyển
        checkProjectileCollisions();
    }

    // 4. QUÁI VẬT DI CHUYỂN VÀ VA CHẠM GÂY SÁT THƯƠNG
    for (size_t i = 0; i < enemies.size(); ++i)
    {
        if (enemies[i].isAlive())
        {
            enemies[i].updatePoisonAndSlow(dt);

            bool isRooted = false;
            for (const auto& effect : rootedEnemies) {
                if (effect.enemyIndex == i) {
                    isRooted = true;
                    break;
                }
            }

            if (!isRooted) {
                enemies[i].update(dt, map.getTiles(), map.getTileSize(), playerPos);
            }

            // XỬ LÝ TRỪ MÁU NGƯỜI CHƠI KHI VA CHẠM QUÁI VẬT
            if (player != nullptr && player->isAlive())
            {
                if (enemies[i].getGlobalBounds().intersects(player->getGlobalBounds()))
                {
                    float damageAmount = 20.0f * dt;
                    player->takeDamage(damageAmount);
                }
            }
        }
    }

    // 5. KIỂM TRA LẠI ĐỢT QUÁI MỚI
    bool allEnemiesDead = true;
    for (const auto& enemy : enemies) {
        if (enemy.isAlive()) {
            allEnemiesDead = false;
            break;
        }
    }

    if (allEnemiesDead)
    {
        if (player != nullptr) player->clearTarget();
        rootedEnemies.clear();
        spawnEnemies();
    }

    if (player != nullptr) {
        sf::Vector2f currentPtPos = player->getPosition();
        camera.setCenter(currentPtPos);
        window.setView(camera);
    }
}

void Game::render()
{
    window.clear(sf::Color::Black);
    map.draw(window);

    if (player != nullptr && player->isAlive()) {
        player->draw(window);

        for (auto& proj : player->getProjectiles()) {
            if (proj.isAlive()) {
                BulletType type = proj.getType();
                if (type == BulletType::SNAKE_Q || type == BulletType::SNAKE_E) {
                    proj.drawChain(window, player->getPosition());
                }
                else {
                    proj.draw(window);
                }
            }
        }
    }

    sf::Vector2f playerPos = (player != nullptr) ? player->getPosition() : sf::Vector2f(0.0f, 0.0f);

    for (int i = 0; i < enemies.size(); i++)
    {
        if (enemies[i].isAlive())
        {
            enemies[i].draw(window);
        }
    }

    if (player != nullptr) {
        sf::Vector2f viewPos = window.getView().getCenter();
        sf::Vector2f viewSize = window.getView().getSize();

        // VẼ THANH MÁU (HP BAR) HUD
        float hpStartX = viewPos.x - (viewSize.x / 2) + 20;
        float hpStartY = viewPos.y - (viewSize.y / 2) + 20;

        sf::RectangleShape hpBackground(sf::Vector2f(250, 22));
        hpBackground.setPosition(hpStartX, hpStartY);
        hpBackground.setFillColor(sf::Color(60, 60, 60, 180));
        hpBackground.setOutlineThickness(2);
        hpBackground.setOutlineColor(sf::Color::White);
        window.draw(hpBackground);

        if (player->isAlive()) {
            float hpPercentage = player->getHealth() / player->getMaxHealth();

            if (hpPercentage > 1.0f) hpPercentage = 1.0f;
            if (hpPercentage < 0.0f) hpPercentage = 0.0f;

            sf::RectangleShape hpBar(sf::Vector2f(250 * hpPercentage, 22));
            hpBar.setPosition(hpStartX, hpStartY);
            hpBar.setFillColor(sf::Color(220, 20, 60));
            window.draw(hpBar);
        }

        // VẼ Ô COOLDOWN SKILL
        float uiStartX = viewPos.x - 100;
        float uiStartY = viewPos.y + (viewSize.y / 2) - 60;

        std::vector<float> cooldowns = {
            player->getSkillQ_Cooldown(),
            player->getSkillW_Cooldown(),
            player->getSkillE_Cooldown(),
            player->getSkillR_Cooldown()
        };

        for (int i = 0; i < 4; i++) {
            sf::RectangleShape slot(sf::Vector2f(40, 40));
            slot.setPosition(uiStartX + (i * 50), uiStartY);
            slot.setOutlineThickness(2);
            slot.setOutlineColor(sf::Color::White);

            if (cooldowns[i] > 0) {
                slot.setFillColor(sf::Color(100, 100, 100, 200));
            }
            else {
                slot.setFillColor(sf::Color(0, 255, 0, 150));
            }
            window.draw(slot);
        }
    }

    window.display();
}