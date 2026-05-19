#include "LoginScreen.h"
#include <iostream>

// Macro xử lý chuỗi Unicode tương thích C++20 và SFML
#define UI_TEXT(str) sf::String::fromUtf8(reinterpret_cast<const sf::Uint8*>(str), reinterpret_cast<const sf::Uint8*>(str) + std::string(str).length())

// Bổ sung tham số AuthManager& vào Constructor để LoginScreen có quyền gọi xuống DB
LoginScreen::LoginScreen(int windowWidth, int windowHeight, AuthManager& auth)
    : state(LoginScreenState::LOGIN), windowWidth(windowWidth), windowHeight(windowHeight),
    usernameInputActive(true), passwordInputActive(false), messageTimer(0.0f), isAuthenticatedUser(false),
    authManager(auth) // Gán liên kết tới AuthManager toàn cục
{
    // Tải font chữ hệ thống hỗ trợ Tiếng Việt
    if (!font.loadFromFile("C:/Windows/Fonts/segoeui.ttf")) {
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
            std::cerr << "Khong the tai bat ky font chu nao!" << std::endl;
        }
    }
    updateLayout();
}

LoginScreen::~LoginScreen() {
    // Destructor rỗng
}

void LoginScreen::updateLayout() {
    float fieldWidth = 400.0f;
    float fieldHeight = 45.0f;
    float centerX = (windowWidth - fieldWidth) / 2.0f;

    usernameFieldRect = sf::FloatRect(centerX, windowHeight * 0.4f, fieldWidth, fieldHeight);
    passwordFieldRect = sf::FloatRect(centerX, windowHeight * 0.55f, fieldWidth, fieldHeight);

    buttonWidth = 200.0f;
    buttonHeight = 50.0f;
    labelFontSize = 18;
    inputFontSize = 20;
    buttonFontSize = 22;
}

void LoginScreen::draw(sf::RenderWindow& window) {
    drawBackground(window);
    drawInputFields(window);
    drawButtons(window);
    drawMessages(window);
}

void LoginScreen::drawBackground(sf::RenderWindow& window) {
    sf::RectangleShape bg(sf::Vector2f(static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
    bg.setFillColor(sf::Color(10, 30, 45));
    window.draw(bg);

    sf::CircleShape light(windowHeight * 0.7f);
    light.setFillColor(sf::Color(0, 150, 255, 12));
    light.setPosition(-windowWidth * 0.1f, -windowHeight * 0.3f);
    window.draw(light);
}

void LoginScreen::drawInputFields(sf::RenderWindow& window) {
    // --- Ô USERNAME ---
    sf::Text uLabel;
    uLabel.setFont(font);
    uLabel.setString(UI_TEXT((const char*)u8"Tên người dùng hoặc email:"));
    uLabel.setCharacterSize(labelFontSize);
    uLabel.setFillColor(sf::Color(160, 210, 255));
    uLabel.setPosition(usernameFieldRect.left, usernameFieldRect.top - 35);
    window.draw(uLabel);

    sf::RectangleShape uBox(sf::Vector2f(usernameFieldRect.width, usernameFieldRect.height));
    uBox.setPosition(usernameFieldRect.left, usernameFieldRect.top);
    uBox.setFillColor(sf::Color(20, 45, 65));
    uBox.setOutlineThickness(2);
    uBox.setOutlineColor(usernameInputActive ? sf::Color(0, 180, 255) : sf::Color(60, 90, 110));
    window.draw(uBox);

    sf::Text uText;
    uText.setFont(font);
    uText.setString(sf::String::fromUtf8(reinterpret_cast<const sf::Uint8*>(usernameInput.c_str()), reinterpret_cast<const sf::Uint8*>(usernameInput.c_str()) + usernameInput.length()));
    uText.setCharacterSize(inputFontSize);
    uText.setFillColor(sf::Color::White);
    uText.setPosition(usernameFieldRect.left + 12, usernameFieldRect.top + 7);
    window.draw(uText);

    // --- Ô PASSWORD ---
    sf::Text pLabel = uLabel;
    pLabel.setString(UI_TEXT((const char*)u8"Mật khẩu:"));
    pLabel.setPosition(passwordFieldRect.left, passwordFieldRect.top - 35);
    window.draw(pLabel);

    sf::RectangleShape pBox = uBox;
    pBox.setPosition(passwordFieldRect.left, passwordFieldRect.top);
    pBox.setOutlineColor(passwordInputActive ? sf::Color(0, 180, 255) : sf::Color(60, 90, 110));
    window.draw(pBox);

    sf::Text pText = uText;
    pText.setString(std::string(passwordInput.length(), '*'));
    pText.setPosition(passwordFieldRect.left + 12, passwordFieldRect.top + 7);
    window.draw(pText);
}

void LoginScreen::drawButtons(sf::RenderWindow& window) {
    const char* labelStr = (state == LoginScreenState::LOGIN) ? (const char*)u8"ĐĂNG NHẬP" : (const char*)u8"ĐĂNG KÝ";

    sf::RectangleShape btn(sf::Vector2f(buttonWidth, buttonHeight));
    btn.setPosition((windowWidth - buttonWidth) / 2.0f, passwordFieldRect.top + 100);
    btn.setFillColor(sf::Color(0, 110, 210));
    window.draw(btn);

    sf::Text txt;
    txt.setFont(font);
    txt.setString(UI_TEXT(labelStr));
    txt.setCharacterSize(buttonFontSize);
    txt.setFillColor(sf::Color::White);

    sf::FloatRect b = txt.getLocalBounds();
    txt.setOrigin(b.left + b.width / 2.0f, b.top + b.height / 2.0f);
    txt.setPosition(btn.getPosition().x + buttonWidth / 2.0f, btn.getPosition().y + buttonHeight / 2.0f);
    window.draw(txt);
}

void LoginScreen::drawMessages(sf::RenderWindow& window) {
    if (messageTimer > 0) {
        sf::Text msg;
        msg.setFont(font);
        // Sửa lỗi gán hiển thị UTF-8
        msg.setString(sf::String::fromUtf8(reinterpret_cast<const sf::Uint8*>(messageText.c_str()), reinterpret_cast<const sf::Uint8*>(messageText.c_str()) + messageText.length()));
        msg.setCharacterSize(18);
        msg.setFillColor(sf::Color(255, 100, 100));

        sf::FloatRect b = msg.getLocalBounds();
        msg.setPosition((windowWidth - b.width) / 2.0f, 50.0f);
        window.draw(msg);
    }
}

void LoginScreen::handleEvent(const sf::Event& event) {
    // 1. CHỨC NĂNG BẤM PHÍM TAB ĐỂ CHUYỂN NHANH Ô NHẬP LIỆU
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Tab) {
            usernameInputActive = !usernameInputActive;
            passwordInputActive = !passwordInputActive;
        }
    }

    // 2. LOGIC XỬ LÝ CHUỘT
    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));

        if (usernameFieldRect.contains(mousePos)) {
            usernameInputActive = true;
            passwordInputActive = false;
        }
        else if (passwordFieldRect.contains(mousePos)) {
            usernameInputActive = false;
            passwordInputActive = true;
        }
        else {
            sf::RectangleShape btn(sf::Vector2f(buttonWidth, buttonHeight));
            btn.setPosition((windowWidth - buttonWidth) / 2.0f, passwordFieldRect.top + 100);

            // Đã sửa lại việc gán chuỗi chính xác sang kiểu dữ liệu std::string chuẩn
            if (btn.getGlobalBounds().contains(mousePos)) {
                if (state == LoginScreenState::LOGIN) {
                    if (authManager.login(usernameInput, passwordInput)) {
                        state = LoginScreenState::AUTHENTICATED;
                    }
                    else {
                        messageText = std::string((const char*)u8"Sai tên đăng nhập hoặc mật khẩu!");
                        messageTimer = 3.0f;
                    }
                }
                else if (state == LoginScreenState::REGISTER) {
                    std::string dummyEmail = usernameInput + "@gmail.com";

                    if (authManager.registerAccount(usernameInput, passwordInput, dummyEmail)) {
                        messageText = std::string((const char*)u8"Đăng ký thành công! Hãy đăng nhập.");
                        messageTimer = 3.0f;
                        state = LoginScreenState::LOGIN;
                        passwordInput.clear();
                    }
                    else {
                        messageText = std::string((const char*)u8"Đăng ký thất bại hoặc tài khoản đã tồn tại!");
                        messageTimer = 3.0f;
                    }
                }
            }
        }
    }

    // 3. LOGIC NHẬN KÝ TỰ TỪ BÀN PHÍM
    if (event.type == sf::Event::TextEntered) {
        // Bỏ qua ký tự Tab (9) và Enter (13) để không bị in ký tự rác vào ô nhập liệu
        if (event.text.unicode == 9 || event.text.unicode == 13) return;

        if (usernameInputActive) {
            if (event.text.unicode == 8 && !usernameInput.empty()) {
                usernameInput.pop_back();
            }
            else if (event.text.unicode >= 32 && event.text.unicode < 127) {
                usernameInput += static_cast<char>(event.text.unicode);
            }
        }
        else if (passwordInputActive) {
            if (event.text.unicode == 8 && !passwordInput.empty()) {
                passwordInput.pop_back();
            }
            else if (event.text.unicode >= 32 && event.text.unicode < 127) {
                passwordInput += static_cast<char>(event.text.unicode);
            }
        }
    }
}

void LoginScreen::update(float dt) {
    if (messageTimer > 0.0f) {
        messageTimer -= dt;
    }
}