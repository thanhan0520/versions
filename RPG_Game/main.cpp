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
        std::cout << reinterpret_cast<const char*>(u8"Dang nhap thanh cong! Dang kiem tra nhan vat tren Database...") << std::endl;

        // 1. Tải dữ liệu tiến trình chơi từ SQL Server lên
        bool hasProgress = authManager.loadGameProgress(savedStage, savedHp, savedScore, savedCharacterClass);

        // 2. BẮT BUỘC KHỞI TẠO SẢNH CHỜ: Truyền thông tin có tiến trình cũ hay không vào Lobby
        // Nếu hasProgress = true và có Class hợp lệ (> 0) thì Lobby sẽ hiện nút "CHƠI TIẾP"
        bool isValidOldData = (hasProgress && savedCharacterClass > 0);
        lobbyScreen.initLobby(isValidOldData, savedCharacterClass);

        std::cout << reinterpret_cast<const char*>(u8"Chuyen huong den sanh cho he thong...") << std::endl;

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
        std::cout << reinterpret_cast<const char*>(u8"Vao game voi nhan vat ") << savedCharacterClass << std::endl;
    }

    // ==========================================
    // BƯỚC 3: KHỞI TẠO GAME VÀ CHÍNH THỨC VÀO ẢI
    // ==========================================
    if (window.isOpen()) {
        std::cout << reinterpret_cast<const char*>(u8"Chuyen canh: Dang nap tai nguyen...") << std::endl;

        // Tạo con trỏ Player đa hình tổng quát
        Player* mainPlayer = nullptr;

        // Ép kiểu số nguyên từ Database/Lobby về enum để kiểm tra loài vật nào
        CharacterClass currentClass = static_cast<CharacterClass>(savedCharacterClass);

        if (currentClass == CharacterClass::RABBIT) {
            mainPlayer = new Rabbit(); // Khởi tạo đúng chú Thỏ Tốc Độ với chỉ số riêng biệt
            std::cout << "==> Da trieu hoi THO TOC DO vao tran dau! <==" << std::endl;
        }
        // TÍCH HỢP MỚI: Khởi tạo Cáo Kiếm Sĩ nếu được chọn từ Lobby/Database
        else if (currentClass == CharacterClass::FOX) {
            mainPlayer = new Fox(); // Khởi tạo đúng Cáo Kiếm Sĩ với bộ kỹ năng cận chiến bẫy dây
            std::cout << "==> Da trieu hoi CAO KIEM SI vao tran dau! <==" << std::endl;
        }
        else if (currentClass == CharacterClass::SNAKE) {
            mainPlayer = new Snake(); // Khởi tạo đúng Rắn Độc với bộ kỹ năng độc tố
            std::cout << "==> Da trieu hoi RAN DUC vao tran dau! <==" << std::endl;
        }
        else if (currentClass == CharacterClass::DOG) {
            mainPlayer = new Dog(); // Khởi tạo đúng Chó Dũng Mạnh với bộ kỹ năng cận chiến
            std::cout << "==> Da trieu hoi CHO DUNG Manh vao tran dau! <==" << std::endl;
        }
        else {
            mainPlayer = new Player();
            std::cout << "==> Tam thoi khoi tao vi chua co file .h cho cac con vat khac." << std::endl;
        }

        // Khởi tạo đối tượng Game của bạn
        Game game(window);

        // ĐÃ MỞ COMMENT: Truyền con trỏ đa hình vào hệ thống vận hành game
        game.setPlayer(mainPlayer);

        game.run();

        // Giải phóng con trỏ sau khi kết thúc game để tránh rò rỉ bộ nhớ (Memory Leak)
        if (mainPlayer != nullptr) {
            delete mainPlayer;
            mainPlayer = nullptr;
        }
    }

    return 0;
}