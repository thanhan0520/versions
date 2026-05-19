#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "AuthManager.h" // Nhúng AuthManager vào để LoginScreen có thể nhận diện

// Định nghĩa các trạng thái của màn hình giao diện
enum class LoginScreenState {
    LOGIN,
    REGISTER,
    AUTHENTICATED
};

class LoginScreen
{
public:
    // Sửa Constructor: Nhận thêm tham chiếu AuthManager& từ main truyền vào
    LoginScreen(int windowWidth, int windowHeight, AuthManager& auth);
    ~LoginScreen();

    void handleEvent(const sf::Event& event);
    void update(float dt);
    void draw(sf::RenderWindow& window);

    // Các hàm bổ trợ logic
    LoginScreenState getState() const { return state; }
    std::string getAuthenticatedUser() const { return authManager.getCurrentUser(); }

private:
    void updateLayout();
    void drawBackground(sf::RenderWindow& window);
    void drawInputFields(sf::RenderWindow& window);
    void drawButtons(sf::RenderWindow& window);
    void drawMessages(sf::RenderWindow& window);

    void clearInputs() {
        usernameInput.clear();
        passwordInput.clear();
    }

    // --- Biến quản lý trạng thái và kích thước ---
    LoginScreenState state;
    int windowWidth;
    int windowHeight;

    // --- Biến lưu trữ dữ liệu người dùng nhập vào ---
    std::string usernameInput;
    std::string passwordInput;
    std::string messageText;

    // --- Quản lý focus ô nhập liệu ---
    bool usernameInputActive;
    bool passwordInputActive;
    float messageTimer;
    bool isAuthenticatedUser;

    // --- Định hình các ô chứa (Hitbox để click chuột) ---
    sf::FloatRect usernameFieldRect;
    sf::FloatRect passwordFieldRect;

    // Khai báo các vùng Rect của nút bấm (để sửa dứt điểm lỗi E0349 gán nhầm)
    sf::FloatRect loginButtonRect;
    sf::FloatRect registerButtonRect;
    sf::FloatRect switchToRegisterButtonRect;
    sf::FloatRect switchToLoginButtonRect;

    float buttonWidth;
    float buttonHeight;
    unsigned int labelFontSize;
    unsigned int inputFontSize;
    unsigned int buttonFontSize;

    sf::Font font;

    // TRỌNG TÂM: Khai báo tham chiếu tới AuthManager toàn cục của game
    AuthManager& authManager;
};