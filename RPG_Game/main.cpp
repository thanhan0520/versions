#include "Game.h"
#include "LoginScreen.h"
#include "AuthManager.h" // 1. NHÚNG THÊM FILE H để hệ thống nhận diện AuthManager
#include <iostream>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "RPG Game");

    // 2. KHỞI TẠO ĐỐI TƯỢNG KẾT NỐI DATABASE TRƯỚC
    AuthManager authManager;

    // 3. TRUYỀN THAM CHIẾU authManager VÀO CONSTRUCTOR CỦA LOGINSCREEN
    LoginScreen loginScreen(800, 600, authManager);

    // BƯỚC 1: Vòng lặp chỉ dành cho Đăng nhập
    // Check trạng thái thông qua getState() để đồng bộ hoàn toàn với enum class
    while (window.isOpen() && loginScreen.getState() != LoginScreenState::AUTHENTICATED) {
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

    // BƯỚC 2: Chỉ khi thoát vòng lặp trên (đã đăng nhập thành công) mới chạy Game
    if (window.isOpen() && loginScreen.getState() == LoginScreenState::AUTHENTICATED) {
        std::cout << "Chuyen canh: Dang vao Game voi tai khoan " << loginScreen.getAuthenticatedUser() << "..." << std::endl;

        // Khởi tạo game và truyền window vào
        Game game(window);
        game.run(); // Lúc này game.run() mới được phép chiếm quyền điều khiển
    }

    return 0;
}