#include "Game.h"
#include "LoginScreen.h"
#include <iostream>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "RPG Game");
    LoginScreen loginScreen(800, 600);

    // BƯỚC 1: Vòng lặp chỉ dành cho Đăng nhập
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

    // BƯỚC 2: Chỉ khi thoát vòng lặp trên (đã đăng nhập) mới chạy Game
    if (window.isOpen() && loginScreen.isAuthenticated()) {
        // Khởi tạo game và truyền window vào (nếu bạn đã sửa Game nhận refWindow)
        Game game(window);
        game.run(); // Lúc này game.run() mới được phép chiếm quyền điều khiển
    }

    return 0;
}