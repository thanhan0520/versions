#include "Game.h"
#include <iostream>
#include <cstdlib>

// Hàm tạo: Khởi tạo cửa sổ game, bản đồ, camera và spawn quái vật ban đầu
Game::Game()
    : window(sf::VideoMode(1024, 768), "RPG Dungeon Explorer")
{
    window.setFramerateLimit(60);
    map.load("map.txt");
    camera.reset(sf::FloatRect(0, 0,1920, 768));
    window.setView(camera);
    spawnEnemies();
    selectedEnemyIndex = -1;              // -1 nghĩa là chưa chọn mục tiêu nào
}

// Tạo quái vật tại vị trí ngẫu nhiên với chỉ số sức mạnh giảm (tốc độ 80)
void Game::spawnEnemies()
{
    // Spawn 5 quái vật, tránh spawn trùng lên tường (vùng an toàn 100-500 theo x,y)
    for (int i = 0; i < 5; i++)
    {
        float x = 100 + rand() % 400;     // Khoảng an toàn từ 100 đến 500px
        float y = 100 + rand() % 300;     // Từ 100 đến 400px
        // params: vị trí, tốc độ di chuyển, máu, tốc độ đánh (dùng sau)
        enemies.emplace_back(sf::Vector2f(x, y), 30.0f, 10.0f, 80.0f);
    }
    std::cout << "Spawned " << enemies.size() << " enemies with reduced speed!" << std::endl;
}

// Kiểm tra va chạm giữa đạn của player và từng quái vật
void Game::checkProjectileCollisions()
{
    auto& projectiles = player.getProjectiles();  // Tham chiếu tránh copy vector

    for (auto& proj : projectiles)                // Duyệt từng viên đạn
    {
        if (!proj.isAlive()) continue;           // Bỏ qua đạn đã nổ/hết hiệu lực

        for (auto& enemy : enemies)
        {
            if (!enemy.isAlive()) continue;      // Bỏ qua quái đã chết

            // Dùng AABB collision (hình chữ nhật bao) - nhanh, đủ chính xác cho game 2D
            if (proj.getGlobalBounds().intersects(enemy.getGlobalBounds()))
            {
                enemy.takeDamage(proj.getDamage());   // Gây sát thương lên enemy
                proj.setAlive(false);                 // Xóa viên đạn sau khi trúng
                std::cout << "Hit! Enemy health: " << enemy.getHealth() << std::endl;
                break;   // Một đạn chỉ trúng một enemy (không xuyên)
            }
        }
    }
}

// Chọn mục tiêu dựa trên vị trí chuột (trong tọa độ thế giới)
void Game::selectTarget(sf::Vector2f mouseWorldPos)
{
    selectedEnemyIndex = -1;          // Reset lựa chọn cũ
    player.clearTarget();             // Xóa target lock trên player

    for (int i = 0; i < enemies.size(); i++)
    {
        // enemy kiểm tra va chạm point-in-rectangle (contains)
        if (enemies[i].isAlive() && enemies[i].getGlobalBounds().contains(mouseWorldPos))
        {
            selectedEnemyIndex = i;
            // Truyền vị trí hiện tại của enemy để player lock onto (bám mục tiêu)
            player.setTargetEnemy(enemies[i].getPosition());
            std::cout << "Target locked: Enemy " << i << " at (" << enemies[i].getPosition().x
                << ", " << enemies[i].getPosition().y << ")" << std::endl;
            return;   // Chỉ chọn enemy đầu tiên trúng chuột
        }
    }
}

// Vòng lặp chính của game (đồng hồ game loop)
void Game::run()
{
    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();  // Delta time (giây) giúp chuyển động mượt mà
        processEvents();   // Xử lý sự kiện bàn phím/chuột
        update(dt);        // Cập nhật logic game (di chuyển, va chạm, AI)
        render();          // Vẽ lại màn hình
    }
}

// Xử lý tất cả sự kiện đầu vào từ cửa sổ
void Game::processEvents()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            window.close();                     // Đóng cửa sổ nếu nhấn X
        else if (event.type == sf::Event::MouseButtonPressed)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                // Chuyển pixel màn hình → tọa độ thế giới (do camera có thể di chuyển)
                sf::Vector2i mousePixelPos(event.mouseButton.x, event.mouseButton.y);
                sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixelPos);
                selectTarget(mouseWorldPos);    // Chọn enemy để tấn công
            }
        }
    }
}

// Cập nhật toàn bộ logic game mỗi frame
void Game::update(float dt)
{
    // 1. Cập nhật player (di chuyển, bắn đạn, va chạm tường)
    player.update(dt, map.getTiles(), map.getTileSize(), window);

    // 2. Cập nhật từng quái vật (AI đuổi theo player, tránh tường)
    for (auto& enemy : enemies)
    {
        if (enemy.isAlive())
        {
            enemy.update(dt, map.getTiles(), map.getTileSize(), player.getPosition());
        }
    }

    // 3. Cập nhật vị trí target lock (nếu enemy còn sống → bám theo)
    if (selectedEnemyIndex >= 0 && selectedEnemyIndex < (int)enemies.size() && enemies[selectedEnemyIndex].isAlive())
    {
        player.setTargetEnemy(enemies[selectedEnemyIndex].getPosition());
    }

    // 4. Xử lý va chạm đạn - quái
    checkProjectileCollisions();

    // 5. Xóa những enemy đã chết khỏi vector (kỹ thuật erase-remove idiom)
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const qvCc& e) { return !e.isAlive(); }),
        enemies.end()
    );

    // 6. Nếu enemy được chọn đã chết (index vượt quá kích thước vector), bỏ chọn
    if (selectedEnemyIndex >= 0 && selectedEnemyIndex >= (int)enemies.size())
    {
        selectedEnemyIndex = -1;
        player.clearTarget();
    }

    // 7. Nếu không còn enemy nào → spawn đợt mới (respawn vô hạn)
    if (enemies.empty())
    {
        std::cout << "All enemies defeated! Spawning new ones..." << std::endl;
        selectedEnemyIndex = -1;
        player.clearTarget();
        spawnEnemies();
    }

    // 8. Camera follow player (gắn camera vào vị trí nhân vật)
    sf::Vector2f playerPos = player.getPosition();
    camera.setCenter(playerPos);
    window.setView(camera);
}

// Vẽ toàn bộ khung cảnh, nhân vật, quái vật, hiệu ứng
void Game::render()
{
    window.clear(sf::Color::Black);   // Xóa màn hình (màu đen nền)
    map.draw(window);                 // Vẽ bản đồ (tiles)
    player.draw(window);              // Vẽ player sprite
    player.drawProjectiles(window);   // Vẽ đạn của player

    // Vẽ từng enemy và highlight kẻ đang bị chọn
    for (int i = 0; i < enemies.size(); i++)
    {
        enemies[i].draw(window);

        // Nếu enemy đang là mục tiêu, vẽ vòng tròn vàng highlight
        if (i == selectedEnemyIndex && enemies[i].isAlive())
        {
            sf::CircleShape targetRing(30);
            targetRing.setFillColor(sf::Color::Transparent);
            targetRing.setOutlineThickness(2.0f);
            targetRing.setOutlineColor(sf::Color::Yellow);
            sf::Vector2f enemyPos = enemies[i].getPosition();
            targetRing.setPosition(enemyPos.x - 30, enemyPos.y - 30); // căn chỉnh tâm
            window.draw(targetRing);
        }
    }

    window.display();   // Hiển thị frame đã vẽ lên màn hình
}
