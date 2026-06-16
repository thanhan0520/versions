#pragma execution_character_set("utf-8")
#include "Game.h"
#include "LoginScreen.h"
#include "LobbyScreen.h" 
#include "AuthManager.h"
#include "Rabbit.h"       
#include "Fox.h"
#include "Snake.h"
#include "Dog.h"
#include <iostream>

int main() {
    // Độ phân giải màn hình chuẩn 800x600
    sf::RenderWindow window(sf::VideoMode(800, 600), "RPG Game - Linh Thu Dai Chien");
    window.setFramerateLimit(60);

    // Khởi tạo bộ kết nối Database
    AuthManager authManager;

    // Khởi tạo màn hình Đăng nhập
    LoginScreen loginScreen(800, 600, authManager);

    // Khởi tạo màn hình Sảnh chờ chọn nhân vật
    LobbyScreen lobbyScreen(800, 600, authManager);

    // Các biến phụ để hứng dữ liệu tải từ SQL Server lên
    int savedStage = 1;
    int savedHp = 100;
    int savedScore = 0;
    int savedCharacterClass = 0; // Lưu mã loài vật (0: Chưa chọn, 1: Cáo, 2: Thỏ, 3: Rắn, 4: Chó)

    // ==========================================
    // BƯỚC 1: VÒNG LẶP CHỈ DÀNH CHO ĐĂNG NHẬP
    // ==========================================
    while (window.isOpen() && !loginScreen.isAuthenticated()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            loginScreen.handleEvent(event);
        }
        loginScreen.update(0.016f);

        window.clear();
        loginScreen.draw(window);
        window.display();
    }

    // ==========================================
    // BƯỚC 2: ĐIỀU HƯỚNG VÀO SẢNH CHỜ (LOBBY)
    // ==========================================
    if (window.isOpen() && loginScreen.isAuthenticated()) {
        std::cout << "Dang nhap thanh cong! Dang kiem tra nhan vat tren Database..." << std::endl;

        // 1. Tải dữ liệu tiến trình chơi từ SQL Server lên
        bool hasProgress = authManager.loadGameProgress(savedStage, savedHp, savedScore, savedCharacterClass);

        // 2. BẮT BUỘC KHỞI TẠO SẢNH CHỜ: Truyền thông tin có tiến trình cũ hay không vào Lobby
        // Nếu hasProgress = true và có Class hợp lệ (> 0) thì Lobby sẽ hiện nút "CHƠI TIẾP"
        bool isValidOldData = (hasProgress && savedCharacterClass > 0);
        lobbyScreen.initLobby(isValidOldData, savedCharacterClass);

        std::cout << "Chuyen huong den sanh cho he thong..." << std::endl;

        // 3. CHẠY VÒNG LẶP SẢNH CHỜ (Tài khoản cũ hay mới đều phải duyệt qua đây)
        while (window.isOpen() && lobbyScreen.getState() != LobbyState::CHARACTER_SELECTED) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) window.close();
                lobbyScreen.handleEvent(event, window);
            }

            window.clear();
            lobbyScreen.draw(window);
            window.display();
        }

        // 4. CẬP NHẬT LẠI CLASS NHÂN VẬT SAU KHI RỜI SẢNH CHỜ
        savedCharacterClass = static_cast<int>(lobbyScreen.getSelectedClass());
        std::cout << "Vao game voi nhan vat " << savedCharacterClass << std::endl;
    }

    // ==========================================
    // BƯỚC 3: KHỞI TẠO GAME VÀ VÒNG LẶP TRỞ VỀ SẢNH CHỜ
    // ==========================================
    while (window.isOpen()) {
        std::cout << "Chuyen canh: Dang nap tai nguyen..." << std::endl;

        // Tạo con trỏ Player dựa trên savedCharacterClass
        Player* mainPlayer = nullptr;
        CharacterClass currentClass = static_cast<CharacterClass>(savedCharacterClass);
        if (currentClass == CharacterClass::RABBIT) mainPlayer = new Rabbit();
        else if (currentClass == CharacterClass::FOX) mainPlayer = new Fox();
        else if (currentClass == CharacterClass::SNAKE) mainPlayer = new Snake();
        else if (currentClass == CharacterClass::DOG) mainPlayer = new Dog();
        else mainPlayer = new Player();

        Game game(window);
        game.setPlayer(mainPlayer);
        game.setAuthManager(&authManager);
        // Initialize score: if player chose to continue, restore savedScore; otherwise reset score to 0
        if (lobbyScreen.isContinuing()) {
            game.setScore(savedScore);
        } else {
            game.setScore(0);
            // ensure DB progress score cleared when starting new game
            if (authManager.getCurrentUser().length() > 0) {
                int hpVal = mainPlayer ? static_cast<int>(mainPlayer->getHealth()) : 0;
                int charClassVal = mainPlayer ? static_cast<int>(mainPlayer->getCharacterClass()) : 0;
                authManager.saveGameProgress(1, hpVal, static_cast<int>(game.getScore()), charClassVal, mainPlayer ? mainPlayer->getPosition().x : 0.0f, mainPlayer ? mainPlayer->getPosition().y : 0.0f, 0);
            }
        }
        if (mainPlayer) mainPlayer->setCharacterClass(currentClass);

        // Nếu người chơi chọn CHƠI TIẾP từ lobby ban đầu
        if (lobbyScreen.isContinuing()) {
            std::string fullState;
            if (authManager.loadFullGameState(fullState)) {
                if (game.restoreState(fullState)) {
                    std::cout << "==> Da khoi phuc tran dau tu trang thai luu tru! <==" << std::endl;
                } else {
                    std::cout << "==> Loi khi phuc hoi trang thai. Choi tu dau." << std::endl;
                }
            }
        }
        else {
            std::string startState = game.serializeState();
            authManager.saveFullGameState(startState);
        }

        game.run();

        bool goToLobby = game.shouldReturnToMain();

        // cleanup player
        if (mainPlayer != nullptr) { delete mainPlayer; mainPlayer = nullptr; }

        if (!goToLobby) {
            // normal exit from game (window closed or quit)
            break;
        }

        // Otherwise, return to lobby flow: reload saved progress and show lobby
        if (!window.isOpen()) break;

        // load latest progress
        bool hasProgress = authManager.loadGameProgress(savedStage, savedHp, savedScore, savedCharacterClass);
        bool isValidOldData = (hasProgress && savedCharacterClass > 0);
        lobbyScreen.reset();
        lobbyScreen.initLobby(isValidOldData, savedCharacterClass);

        // Reset the window view to default so lobby UI renders correctly (game may have changed the view)
        window.setView(window.getDefaultView());

        // Run lobby again until character selected or window closed
        while (window.isOpen() && lobbyScreen.getState() != LobbyState::CHARACTER_SELECTED) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) window.close();
                lobbyScreen.handleEvent(event, window);
            }
            window.clear();
            lobbyScreen.draw(window);
            window.display();
        }

        if (!window.isOpen()) break;

        savedCharacterClass = static_cast<int>(lobbyScreen.getSelectedClass());
        std::cout << "Vao game voi nhan vat " << savedCharacterClass << std::endl;
    }

    return 0;
} 