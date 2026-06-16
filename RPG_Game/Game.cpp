#include "Game.h"
#include "Dog.h" 
#include "Fox.h"
#include "Snake.h"
#include "Rabbit.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <algorithm> 
#include <sstream>
#include <string>
#include "AuthManager.h"

#define UI_TEXT(str) sf::String::fromUtf8(reinterpret_cast<const sf::Uint8*>(str), reinterpret_cast<const sf::Uint8*>(str) + std::string(str).length())

Game::Game(sf::RenderWindow& sharedWindow)
    : window(sharedWindow), player(nullptr), selectedEnemyIndex(-1)
{
    window.setFramerateLimit(60);
    map.load("map.txt");
    camera.reset(sf::FloatRect(0, 0, 1920, 768));
    window.setView(camera);
    spawnEnemies();

    // Autosave defaults
    authManager = nullptr;
    autosaveTimer = 0.0f;
    autosaveInterval = 15.0f; // seconds
    isGameOver = false;
}

void Game::addScore(int delta) {
    score += delta;
}

void Game::spawnPotion(sf::Vector2f pos, float healAmount)
{
    Potion p;
    p.pos = pos;
    p.healAmount = healAmount;
    p.alive = true;
    p.shape = sf::CircleShape(10.0f);
    p.shape.setFillColor(sf::Color(255, 105, 180)); // pinkish
    p.shape.setPosition(pos - sf::Vector2f(10,10));
    potions.push_back(p);
}

// Simple serialization: "playerX,playerY,playerHP;enemyCount;ex1,ey1,ehp1;ex2,ey2,ehp2;..."
std::string Game::serializeState() const {
    std::ostringstream ss;
    if (player) {
        ss << player->getPosition().x << "," << player->getPosition().y << "," << player->getHealth();
    }
    else {
        ss << "0,0,0";
    }
    ss << ";" << enemies.size();
    for (const auto& e : enemies) {
        ss << ";" << e.getPosition().x << "," << e.getPosition().y << "," << e.getHealth();
    }
    // serialize potions after enemies
    for (const auto &p : potions) {
        if (!p.alive) continue;
        ss << ";" << p.pos.x << "," << p.pos.y << "," << p.healAmount;
    }
    return ss.str();
}

bool Game::restoreState(const std::string& serialized)
{
    if (serialized.empty()) return false;
    try {
        std::vector<std::string> parts;
        std::string cur;
        for (char c : serialized) {
            if (c == ';') { parts.push_back(cur); cur.clear(); }
            else cur.push_back(c);
        }
        if (!cur.empty()) parts.push_back(cur);

        if (parts.size() < 2) return false;

        // player
        {
            std::istringstream ps(parts[0]);
            float px, py, ph;
            char comma;
            ps >> px >> comma >> py >> comma >> ph;
            if (player) {
                player->setPosition(px, py);
                player->setHealth(ph);
                if (!player->isAlive()) {
                    isGameOver = true;
                }
            }
        }

        // enemies
        size_t enemyCount = std::stoul(parts[1]);
        enemies.clear();
        for (size_t i = 0; i < enemyCount && 2 + i < parts.size(); ++i) {
            std::istringstream es(parts[2 + i]);
            float ex, ey, eh;
            char comma;
            es >> ex >> comma >> ey >> comma >> eh;
            enemies.emplace_back(sf::Vector2f(ex, ey), eh, 10.0f, 80.0f);
        }
        // initialize enemyCounted vector to same size
        enemyCounted.clear();
        enemyCounted.resize(enemies.size(), false);
        // potions (optional part)
        if (parts.size() > 2 + enemyCount) {
            potions.clear();
            for (size_t i = 0; i < parts.size() - (2 + enemyCount); ++i) {
                std::istringstream ps(parts[2 + enemyCount + i]);
                float px, py, ph;
                char comma;
                ps >> px >> comma >> py >> comma >> ph;
                Potion p;
                p.pos = sf::Vector2f(px, py);
                p.healAmount = ph;
                p.alive = true;
                p.shape = sf::CircleShape(10.0f);
                p.shape.setFillColor(sf::Color(255, 105, 180));
                p.shape.setPosition(p.pos - sf::Vector2f(10,10));
                potions.push_back(p);
            }
        }
        return true;
    }
    catch (...) { return false; }
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
    std::cout << "Trieu hoi " << enemies.size() << " quai thanh cong" << std::endl;
    enemyCounted.clear();
    enemyCounted.resize(enemies.size(), false);
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
                        float dmg = proj.getDamage();
                        enemy.takeDamage(dmg);
                        std::cout << "Projectile AOE hit: type=" << static_cast<int>(pType) << " idx=" << i << " dmg=" << dmg << " hpAfter=" << enemy.getHealth() << std::endl;
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
                    float dmg = proj.getDamage();
                    enemy.takeDamage(dmg);
                    std::cout << "Projectile hit: type=" << static_cast<int>(pType) << " idx=" << i << " dmg=" << dmg << " hpAfter=" << enemy.getHealth() << std::endl;
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
                    if (pType == BulletType::BOUNCE)
                    {
                        int nextTarget = -1;

                        float closestDist = 999999.0f;

                        sf::Vector2f currentPos = proj.getPosition();

                        for (size_t j = 0; j < enemies.size(); ++j)
                        {
                            if (j == i)
                                continue;

                            if (!enemies[j].isAlive())
                                continue;

                            float dist =
                                std::sqrt(
                                    std::pow(enemies[j].getPosition().x - currentPos.x, 2) +
                                    std::pow(enemies[j].getPosition().y - currentPos.y, 2)
                                );

                            if (dist < closestDist)
                            {
                                closestDist = dist;
                                nextTarget = static_cast<int>(j);
                            }
                        }

                        // Nếu tìm được mục tiêu mới
                        if (nextTarget != -1)
                        {
                            sf::Vector2f newDir =
                                enemies[nextTarget].getPosition() - currentPos;

                            proj.setDirection(newDir);

                            proj.clearHitTargets();
                        }
                        else
                        {
                            proj.setAlive(false);
                        }
                    }
                    else
                    {
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

        // If pause menu requested return to main, exit run loop so caller can handle lobby
        if (returnToMain) {
            std::cout << "Game::run: exit requested to return to main" << std::endl;
            break;
        }

        // Toggle pause via Escape key (checked here so we capture toggles between frames)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
            if (!lastEscPressed) {
                paused = !paused;
                if (paused) {
                    if (authManager) {
                        std::string st = serializeState();
                        authManager->saveFullGameState(st);
                        int hpVal = player ? static_cast<int>(player->getHealth()) : 0;
                        int charClassVal = player ? static_cast<int>(player->getCharacterClass()) : 0;
                        float px = player ? player->getPosition().x : 0.0f;
                        float py = player ? player->getPosition().y : 0.0f;
                        authManager->saveGameProgress(1, hpVal, score, charClassVal, px, py, static_cast<int>(enemies.size()));
                        std::cout << "Game: Paused and autosaved" << std::endl;
                    }
                } else {
                    std::cout << "Game: Resumed" << std::endl;
                }
            }
            lastEscPressed = true;
        } else {
            lastEscPressed = false;
        }

        if (!paused) update(dt);
        render();
    }
}

void Game::processEvents()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed) {
            // If dead, ensure state saved before close
            if (isGameOver && authManager) {
                std::string st = serializeState();
                authManager->saveFullGameState(st);
                std::cout << "Game: Luu trang thai chet truoc khi dong" << std::endl;
            }
            window.close();
        }
        // If game-over overlay active, handle clicks here so we don't poll twice
        // If pause menu active, handle its clicks first
        if (paused && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f worldPos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
            if (pauseBtnResume.contains(worldPos)) {
                paused = false;
                std::cout << "Game: Resumed from pause" << std::endl;
            }
            else if (pauseBtnSave.contains(worldPos)) {
                if (authManager) {
                    std::string st = serializeState();
                    authManager->saveFullGameState(st);
                    int hpVal = player ? static_cast<int>(player->getHealth()) : 0;
                    int charClassVal = player ? static_cast<int>(player->getCharacterClass()) : 0;
                    float px = player ? player->getPosition().x : 0.0f;
                    float py = player ? player->getPosition().y : 0.0f;
                    authManager->saveGameProgress(1, hpVal, score, charClassVal, px, py, static_cast<int>(enemies.size()));
                    std::cout << "Game: Saved from pause menu" << std::endl;
                }
            }
            else if (pauseBtnReturnMain.contains(worldPos)) {
                // Return to lobby (without closing window). Caller will handle lobby flow.
                returnToMain = true;
                paused = false;
                std::cout << "Game: Returning to lobby (sanh cho)" << std::endl;
            }
            else if (pauseBtnExit.contains(worldPos)) {
                // Close window to fully exit
                if (authManager) {
                    std::string st = serializeState();
                    authManager->saveFullGameState(st);
                }
                window.close();
                std::cout << "Game: Exiting application from pause menu" << std::endl;
            }
            continue; // consume event when paused
        }

        // If not paused, allow clicking the HUD pause button
        if (!paused && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f worldPos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
            if (pauseHudRect.contains(worldPos)) {
                paused = true;
                if (authManager) {
                        std::string st = serializeState();
                        authManager->saveFullGameState(st);
                        int hpVal = player ? static_cast<int>(player->getHealth()) : 0;
                        int charClassVal = player ? static_cast<int>(player->getCharacterClass()) : 0;
                        float px = player ? player->getPosition().x : 0.0f;
                        float py = player ? player->getPosition().y : 0.0f;
                        std::cout << "Game: Saving progress (user='" << authManager->getCurrentUser() << "', class=" << charClassVal << ", score=" << score << ")" << std::endl;
                        authManager->saveGameProgress(1, hpVal, score, charClassVal, px, py, static_cast<int>(enemies.size()));
                }
                continue;
            }
        }

        if (isGameOver && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f m(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
            // convert screen coords to world coords because overlay drawn in world view
            sf::Vector2f worldPos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
            if (gameOverBtnNew.contains(worldPos)) {
                // Return to main screen / lobby instead of creating a new game
                returnToMain = true;
                isGameOver = false;
                paused = false;
                std::cout << "Game: Returning to main screen from game over" << std::endl;
                // persist full state if available
                if (authManager) {
                    std::string st = serializeState();
                    authManager->saveFullGameState(st);
                }
            }
            else if (gameOverBtnRevive.contains(worldPos)) {
                if (player) {
                    player->setHealth(player->getMaxHealth() * 0.5f);
                    isGameOver = false;
                    std::cout << "Game: Nguoi choi da duoc hoi sinh" << std::endl;
                    if (authManager) {
                        std::string st = serializeState();
                        authManager->saveFullGameState(st);
                        int hpVal = static_cast<int>(player->getHealth());
                        int charClassVal = static_cast<int>(player->getCharacterClass());
                        float px = player->getPosition().x;
                        float py = player->getPosition().y;
                        std::cout << "Game: Saving progress (user='" << authManager->getCurrentUser() << "', class=" << charClassVal << ", score=" << score << ")" << std::endl;
                        authManager->saveGameProgress(1, hpVal, score, charClassVal, px, py, static_cast<int>(enemies.size()));
                    }
                }
            }
        }
    }
}

void Game::update(float dt)
{
    // Autosave ticking
    if (authManager != nullptr) {
        autosaveTimer += dt;
        if (autosaveTimer >= autosaveInterval) {
            autosaveTimer = 0.0f;
            // perform autosave
            std::string state = serializeState();
            // Save basic progress: stage=1, hp=player hp (int), score=0, class
            int hpInt = player ? static_cast<int>(player->getHealth()) : 0;
            int charClass = player ? static_cast<int>(player->getCharacterClass()) : 0;
            float px = player ? player->getPosition().x : 0.0f;
            float py = player ? player->getPosition().y : 0.0f;
            int mcount = static_cast<int>(enemies.size());

            bool ok1 = authManager->saveGameProgress(1, hpInt, score, charClass, px, py, mcount);
            bool ok2 = authManager->saveFullGameState(state);
            std::cout << "Game: Autosave triggered. DB progress=" << ok1 << " full=" << ok2 << " len=" << state.size() << std::endl;
        }
    }
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
            // Khi chết: bật game-over overlay
            isGameOver = true;
            // vẫn cho phép nhấn R để revive trong phiên bản chơi (debug)
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
                // revive logic: hồi phục 50% HP và đặt lại vị trí trung tâm
                player->setHealth(player->getMaxHealth() * 0.5f);
                player->setPosition(512, 384);
                isGameOver = false;
                std::cout << "Game: Nguoi choi da duoc hoi sinh tam thoi (R)" << std::endl;
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
            // If enemy just died this frame, nothing here; potion/score awarding handled in post-pass
            if (!enemies[i].isAlive()) {
                // nothing here; handled later
            }
        }
    }

    // Separate pass: award score for any enemies that died during projectile processing or earlier but weren't counted
    for (size_t i = 0; i < enemies.size(); ++i) {
        if (!enemies[i].isAlive()) {
            if (i < enemyCounted.size() && !enemyCounted[i]) {
                // Award score
                addScore(10);
                enemyCounted[i] = true;
                std::cout << "Game: Giat diem khi diet quai -> +10. Total=" << score << std::endl;

                // Potion drop chance
                float r = static_cast<float>(rand()) / RAND_MAX;
                if (r < potionDropRate) {
                    float healAmt = potionHealFraction * (player ? player->getMaxHealth() : 100.0f);
                    spawnPotion(enemies[i].getPosition(), healAmt);
                    std::cout << "Game: Quai roi mau va rot potion (+" << healAmt << ")" << std::endl;
                }

                // Save immediate
                if (authManager) {
                    int hpVal = player ? static_cast<int>(player->getHealth()) : 0;
                    int charClassVal = player ? static_cast<int>(player->getCharacterClass()) : 0;
                    authManager->saveGameProgress(1, hpVal, score, charClassVal, player ? player->getPosition().x : 0.0f, player ? player->getPosition().y : 0.0f, static_cast<int>(enemies.size()));
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

    // potion pickup handling
    if (player != nullptr) {
        for (auto &pt : potions) {
            if (!pt.alive) continue;
            sf::FloatRect pr(pt.pos.x - 10, pt.pos.y - 10, 20, 20);
            if (pr.intersects(player->getGlobalBounds())) {
                float oldHp = player->getHealth();
                float newHp = oldHp + pt.healAmount;
                if (newHp > player->getMaxHealth()) newHp = player->getMaxHealth();
                player->setHealth(newHp);
                pt.alive = false;
                std::cout << "Game: Nguoi choi nhan potion +" << pt.healAmount << " HP. Current=" << newHp << "\n";
                // save state immediately
                if (authManager) {
                    std::string st = serializeState();
                    authManager->saveFullGameState(st);
                    int hpVal = static_cast<int>(player->getHealth());
                    int charClassVal = static_cast<int>(player->getCharacterClass());
                    authManager->saveGameProgress(1, hpVal, score, charClassVal, player->getPosition().x, player->getPosition().y, static_cast<int>(enemies.size()));
                }
            }
        }
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

    // draw potions
    for (auto &pt : potions) {
        if (!pt.alive) continue;
        window.draw(pt.shape);
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

        // Draw score box next to HP bar
        {
            float scoreBoxW = 160.0f;
            float scoreBoxH = 36.0f;
            float scoreX = hpStartX + 260 + 12.0f;
            float scoreY = hpStartY;
            sf::RectangleShape scoreBg(sf::Vector2f(scoreBoxW, scoreBoxH));
            scoreBg.setPosition(scoreX, scoreY);
            scoreBg.setFillColor(sf::Color(50, 50, 50, 200));
            scoreBg.setOutlineThickness(2);
            scoreBg.setOutlineColor(sf::Color::White);
            window.draw(scoreBg);

            sf::Font f2;
            if (!f2.loadFromFile("C:/Windows/Fonts/segoeui.ttf")) f2.loadFromFile("C:/Windows/Fonts/arial.ttf");
            sf::Text scoreText;
            scoreText.setFont(f2);
            std::string s = std::string((const char*)u8"Điểm: ") + std::to_string(score);
            scoreText.setString(UI_TEXT(s.c_str()));
            scoreText.setCharacterSize(18);
            scoreText.setFillColor(sf::Color::White);
            sf::FloatRect tb = scoreText.getLocalBounds();
            scoreText.setOrigin(tb.left, tb.top + tb.height/2);
            scoreText.setPosition(scoreX + 8.0f, scoreY + scoreBoxH/2.0f);
            window.draw(scoreText);
        }
    }

    // Draw larger pause button on HUD (top-right)
    {
        sf::Vector2f viewPos = window.getView().getCenter();
        sf::Vector2f viewSize = window.getView().getSize();
        float bx = viewPos.x + viewSize.x/2 - 80;
        float by = viewPos.y - viewSize.y/2 + 10;
        pauseHudRect = sf::FloatRect(bx, by, 64, 40);
        sf::RectangleShape pauseBtn(sf::Vector2f(pauseHudRect.width, pauseHudRect.height));
        pauseBtn.setPosition(pauseHudRect.left, pauseHudRect.top);
        pauseBtn.setFillColor(sf::Color(50,50,50,220));
        pauseBtn.setOutlineThickness(2);
        pauseBtn.setOutlineColor(sf::Color::White);
        window.draw(pauseBtn);
        sf::Font f;
        if (!f.loadFromFile("C:/Windows/Fonts/segoeui.ttf")) f.loadFromFile("C:/Windows/Fonts/arial.ttf");
        sf::Text t; t.setFont(f); t.setString(UI_TEXT((const char*)u8"||")); t.setCharacterSize(24); t.setFillColor(sf::Color::White);
        sf::FloatRect tb = t.getLocalBounds(); t.setOrigin(tb.left + tb.width/2, tb.top + tb.height/2); t.setPosition(pauseHudRect.left + pauseHudRect.width/2, pauseHudRect.top + pauseHudRect.height/2 - 2); window.draw(t);
    }

    // Draw overlays (game over / pause) and present once to avoid flicker
    if (isGameOver) {
        renderGameOver();
    }

    if (paused && !isGameOver) {
        renderPauseMenu();
    }

    window.display();
}

void Game::renderPauseMenu()
{
    sf::View prev = window.getView();
    sf::Vector2f viewPos = window.getView().getCenter();
    sf::Vector2f viewSize = window.getView().getSize();

    sf::RectangleShape overlay(viewSize);
    overlay.setPosition(viewPos.x - viewSize.x/2, viewPos.y - viewSize.y/2);
    overlay.setFillColor(sf::Color(0,0,0,180));
    window.draw(overlay);

    sf::Font f;
    if (!f.loadFromFile("C:/Windows/Fonts/segoeui.ttf")) {
        f.loadFromFile("C:/Windows/Fonts/arial.ttf");
    }

    sf::Text title;
    title.setFont(f);
    title.setString(UI_TEXT((const char*)u8"PAUSED"));
    title.setCharacterSize(36);
    title.setFillColor(sf::Color::White);
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.left + tb.width/2, tb.top + tb.height/2);
    title.setPosition(viewPos.x, viewPos.y - 80);
    window.draw(title);

    // Buttons: Resume (Tiếp tục), Save (Lưu game), Return to Lobby (Về sảnh chờ), Exit (Thoát game)
    pauseBtnResume = sf::FloatRect(viewPos.x - 260, viewPos.y - 30, 180, 70);
    pauseBtnSave = sf::FloatRect(viewPos.x - 60, viewPos.y - 30, 180, 70);
    pauseBtnReturnMain = sf::FloatRect(viewPos.x + 140, viewPos.y - 30, 200, 70);
    pauseBtnExit = sf::FloatRect(viewPos.x + 360, viewPos.y - 30, 160, 70);

    sf::RectangleShape b1(sf::Vector2f(pauseBtnResume.width, pauseBtnResume.height));
    b1.setPosition(pauseBtnResume.left, pauseBtnResume.top);
    b1.setFillColor(sf::Color(70,130,180));
    window.draw(b1);
    sf::Text t1; t1.setFont(f); t1.setString(UI_TEXT((const char*)u8"Tiếp tục")); t1.setCharacterSize(26); t1.setFillColor(sf::Color::White);
    sf::FloatRect tb1 = t1.getLocalBounds(); t1.setOrigin(tb1.left + tb1.width/2, tb1.top + tb1.height/2); t1.setPosition(pauseBtnResume.left + pauseBtnResume.width/2, pauseBtnResume.top + pauseBtnResume.height/2); window.draw(t1);

    sf::RectangleShape b2(sf::Vector2f(pauseBtnSave.width, pauseBtnSave.height));
    b2.setPosition(pauseBtnSave.left, pauseBtnSave.top);
    b2.setFillColor(sf::Color(34,139,34));
    window.draw(b2);
    sf::Text t2; t2.setFont(f); t2.setString(UI_TEXT((const char*)u8"Lưu game")); t2.setCharacterSize(26); t2.setFillColor(sf::Color::White);
    sf::FloatRect tb2 = t2.getLocalBounds(); t2.setOrigin(tb2.left + tb2.width/2, tb2.top + tb2.height/2); t2.setPosition(pauseBtnSave.left + pauseBtnSave.width/2, pauseBtnSave.top + pauseBtnSave.height/2); window.draw(t2);

    sf::RectangleShape b3(sf::Vector2f(pauseBtnReturnMain.width, pauseBtnReturnMain.height));
    b3.setPosition(pauseBtnReturnMain.left, pauseBtnReturnMain.top);
    b3.setFillColor(sf::Color(200,140,20));
    window.draw(b3);
    sf::Text t3; t3.setFont(f); t3.setString(UI_TEXT((const char*)u8"Về sảnh chờ")); t3.setCharacterSize(22); t3.setFillColor(sf::Color::White);
    sf::FloatRect tb3 = t3.getLocalBounds(); t3.setOrigin(tb3.left + tb3.width/2, tb3.top + tb3.height/2); t3.setPosition(pauseBtnReturnMain.left + pauseBtnReturnMain.width/2, pauseBtnReturnMain.top + pauseBtnReturnMain.height/2); window.draw(t3);

    sf::RectangleShape b4(sf::Vector2f(pauseBtnExit.width, pauseBtnExit.height));
    b4.setPosition(pauseBtnExit.left, pauseBtnExit.top);
    b4.setFillColor(sf::Color(178,34,34));
    window.draw(b4);
    sf::Text t4; t4.setFont(f); t4.setString(UI_TEXT((const char*)u8"Thoát game")); t4.setCharacterSize(22); t4.setFillColor(sf::Color::White);
    sf::FloatRect tb4 = t4.getLocalBounds(); t4.setOrigin(tb4.left + tb4.width/2, tb4.top + tb4.height/2); t4.setPosition(pauseBtnExit.left + pauseBtnExit.width/2, pauseBtnExit.top + pauseBtnExit.height/2); window.draw(t4);

    window.setView(prev);
}

void Game::renderGameOver()
{
    if (!isGameOver) return;

    sf::View prev = window.getView();
    sf::Vector2f viewPos = window.getView().getCenter();
    sf::Vector2f viewSize = window.getView().getSize();

    sf::RectangleShape overlay(viewSize);
    overlay.setPosition(viewPos.x - viewSize.x/2, viewPos.y - viewSize.y/2);
    overlay.setFillColor(sf::Color(0,0,0,180));
    window.draw(overlay);

    sf::Font f;
    if (!f.loadFromFile("C:/Windows/Fonts/segoeui.ttf")) {
        f.loadFromFile("C:/Windows/Fonts/arial.ttf");
    }

    sf::Text title;
    title.setFont(f);
    title.setString(UI_TEXT((const char*)u8"BẠN ĐÃ CHẾT"));
    title.setCharacterSize(40);
    title.setFillColor(sf::Color::White);
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.left + tb.width/2, tb.top + tb.height/2);
    title.setPosition(viewPos.x, viewPos.y - 60);
    window.draw(title);

    // Buttons: size them dynamically to fit their text
    sf::Text t1;
    t1.setFont(f);
    t1.setString(UI_TEXT((const char*)u8"TRỞ VỀ MÀN HÌNH CHÍNH"));
    t1.setCharacterSize(18);
    t1.setFillColor(sf::Color::White);
    sf::FloatRect tb1 = t1.getLocalBounds();

    sf::Text t2;
    t2.setFont(f);
    t2.setString(UI_TEXT((const char*)u8"HỒI SINH"));
    t2.setCharacterSize(18);
    t2.setFillColor(sf::Color::White);
    sf::FloatRect tb2 = t2.getLocalBounds();

    float paddingX = 40.0f; // horizontal padding inside button
    float paddingY = 20.0f; // vertical padding inside button
    float gap = 20.0f; // gap between buttons

    float btnWidth1 = tb1.width + paddingX;
    float btnWidth2 = tb2.width + paddingX;
    float btnHeight = std::max(tb1.height, tb2.height) + paddingY;

    float totalWidth = btnWidth1 + gap + btnWidth2;
    float leftStart = viewPos.x - totalWidth/2.0f;
    float top = viewPos.y + 10.0f;

    gameOverBtnNew = sf::FloatRect(leftStart, top, btnWidth1, btnHeight);
    gameOverBtnRevive = sf::FloatRect(leftStart + btnWidth1 + gap, top, btnWidth2, btnHeight);

    sf::RectangleShape b1(sf::Vector2f(gameOverBtnNew.width, gameOverBtnNew.height));
    b1.setPosition(gameOverBtnNew.left, gameOverBtnNew.top);
    b1.setFillColor(sf::Color(178,34,34));
    window.draw(b1);
    tb1 = t1.getLocalBounds();
    t1.setOrigin(tb1.left + tb1.width/2, tb1.top + tb1.height/2);
    t1.setPosition(gameOverBtnNew.left + gameOverBtnNew.width/2, gameOverBtnNew.top + gameOverBtnNew.height/2);
    window.draw(t1);

    sf::RectangleShape b2(sf::Vector2f(gameOverBtnRevive.width, gameOverBtnRevive.height));
    b2.setPosition(gameOverBtnRevive.left, gameOverBtnRevive.top);
    b2.setFillColor(sf::Color(34,139,34));
    window.draw(b2);
    tb2 = t2.getLocalBounds();
    t2.setOrigin(tb2.left + tb2.width/2, tb2.top + tb2.height/2);
    t2.setPosition(gameOverBtnRevive.left + gameOverBtnRevive.width/2, gameOverBtnRevive.top + gameOverBtnRevive.height/2);
    window.draw(t2);

    window.setView(prev);
}
