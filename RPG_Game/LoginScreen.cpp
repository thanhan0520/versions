#pragma execution_character_set("utf-8")
#include "LoginScreen.h"
#include "AuthManager.h"
#include <iostream>

#define UI_TEXT(str) sf::String::fromUtf8(reinterpret_cast<const sf::Uint8*>(str), reinterpret_cast<const sf::Uint8*>(str) + std::string(str).length())

LoginScreen::LoginScreen(int windowWidth, int windowHeight, AuthManager& auth)
    : state(LoginScreenState::LOGIN), windowWidth(windowWidth), windowHeight(windowHeight), authManager(auth),
    usernameInputActive(true), passwordInputActive(false), emailInputActive(false),
    messageTimer(0.0f), isAuthenticatedUser(false),
    buttonWidth(180.0f), buttonHeight(45.0f),
    labelFontSize(16), inputFontSize(18), buttonFontSize(20)
{
    if (!font.loadFromFile("C:/Windows/Fonts/segoeui.ttf")) {
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
            std::cerr << "LoginScreen: Khong the tai font chu!" << std::endl;
        }
    }
    updateLayout();
}

LoginScreen::~LoginScreen() {}

std::string LoginScreen::getAuthenticatedUser() const {
    return authManager.getCurrentUser();
}

void LoginScreen::updateLayout() {
    float fieldWidth = 300.0f;
    float fieldHeight = 40.0f;
    float startX = (windowWidth - fieldWidth) / 2.0f;

    // Thay đổi kích thước nút nhỏ lại một chút để vừa vặn khi xếp hàng ngang
    buttonWidth = 140.0f;
    buttonHeight = 45.0f;

    // Khoảng cách trống nhỏ ở giữa 2 nút bấm
    float buttonGap = fieldWidth - (buttonWidth * 2); // 300 - (140 * 2) = 20px trống ở giữa

    if (state == LoginScreenState::LOGIN) {
        usernameFieldRect = sf::FloatRect(startX, windowHeight * 0.35f, fieldWidth, fieldHeight);
        passwordFieldRect = sf::FloatRect(startX, windowHeight * 0.48f, fieldWidth, fieldHeight);
        emailFieldRect = sf::FloatRect(0, 0, 0, 0);

        // Nút thứ nhất nằm sát lề bên trái của cụm ô nhập
        loginButtonRect = sf::FloatRect(startX, windowHeight * 0.62f, buttonWidth, buttonHeight);
        // Nút thứ hai nằm sát lề bên phải của cụm ô nhập và cách nút một đoạn buttonGap
        switchToRegisterButtonRect = sf::FloatRect(startX + buttonWidth + buttonGap, windowHeight * 0.62f, buttonWidth, buttonHeight);
    }
    else if (state == LoginScreenState::REGISTER) {
        usernameFieldRect = sf::FloatRect(startX, windowHeight * 0.28f, fieldWidth, fieldHeight);
        passwordFieldRect = sf::FloatRect(startX, windowHeight * 0.39f, fieldWidth, fieldHeight);
        emailFieldRect = sf::FloatRect(startX, windowHeight * 0.50f, fieldWidth, fieldHeight);

        // Áp dụng tương tự cho màn hình Đăng ký
        registerButtonRect = sf::FloatRect(startX, windowHeight * 0.64f, buttonWidth, buttonHeight);
        switchToLoginButtonRect = sf::FloatRect(startX + buttonWidth + buttonGap, windowHeight * 0.64f, buttonWidth, buttonHeight);
    }
}

void LoginScreen::update(float dt) {
    if (messageTimer > 0.0f) {
        messageTimer -= dt;
        if (messageTimer <= 0.0f) messageText.clear();
    }
}

void LoginScreen::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));

        usernameInputActive = usernameFieldRect.contains(mousePos);
        passwordInputActive = passwordFieldRect.contains(mousePos);
        emailInputActive = emailFieldRect.contains(mousePos);

        if (state == LoginScreenState::LOGIN) {
            if (loginButtonRect.contains(mousePos)) {
                if (authManager.login(usernameInput, passwordInput)) {
                    state = LoginScreenState::AUTHENTICATED;
                }
                else {
                    messageText = (const char*)u8"Sai tài khoản hoặc mật khẩu!";
                    messageTimer = 3.0f;
                }
            }
            else if (switchToRegisterButtonRect.contains(mousePos)) {
                state = LoginScreenState::REGISTER;
                clearInputs();
                updateLayout();
            }
        }
        else if (state == LoginScreenState::REGISTER) {
            if (registerButtonRect.contains(mousePos)) {
                if (usernameInput.empty() || passwordInput.empty() || emailInput.empty()) {
                    messageText = (const char*)u8"Vui lòng nhập đầy đủ thông tin!";
                    messageTimer = 3.0f;
                }
                else {
                    if (authManager.registerAccount(usernameInput, passwordInput, emailInput)) {
                        messageText = (const char*)u8"Đăng ký thành công! Hãy đăng nhập.";
                        messageTimer = 4.0f;
                        state = LoginScreenState::LOGIN;
                        clearInputs();
                        updateLayout();
                    }
                    else {
                        messageText = (const char*)u8"Tên tài khoản đã tồn tại!";
                        messageTimer = 3.0f;
                    }
                }
            }
            else if (switchToLoginButtonRect.contains(mousePos)) {
                state = LoginScreenState::LOGIN;
                clearInputs();
                updateLayout();
            }
        }
    }

    if (event.type == sf::Event::TextEntered) {
        sf::Uint32 unicode = event.text.unicode;
        if (unicode < 32 && unicode != 8) return;

        std::string* targetInput = nullptr;
        if (usernameInputActive) targetInput = &usernameInput;
        else if (passwordInputActive) targetInput = &passwordInput;
        else if (emailInputActive) targetInput = &emailInput;

        if (targetInput != nullptr) {
            if (unicode == 8) {
                if (!targetInput->empty()) targetInput->pop_back();
            }
            else if (targetInput->length() < 30) {
                *targetInput += static_cast<char>(unicode);
            }
        }
    }
}

void LoginScreen::draw(sf::RenderWindow& window) {
    drawBackground(window);
    drawInputFields(window);
    drawButtons(window);
    drawMessages(window);
}

void LoginScreen::drawBackground(sf::RenderWindow& window) {
    sf::RectangleShape bg(sf::Vector2f(static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
    bg.setFillColor(sf::Color(20, 25, 40));
    window.draw(bg);

    sf::Text titleText;
    titleText.setFont(font);
    titleText.setString(state == LoginScreenState::LOGIN ? UI_TEXT((const char*)u8"ĐĂNG NHẬP HỆ THỐNG") : UI_TEXT((const char*)u8"ĐĂNG KÝ TÀI KHOẢN"));
    titleText.setCharacterSize(32);
    titleText.setFillColor(sf::Color(0, 191, 255));
    titleText.setStyle(sf::Text::Bold);

    sf::FloatRect bounds = titleText.getLocalBounds();
    titleText.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    titleText.setPosition(windowWidth / 2.0f, windowHeight * 0.15f);
    window.draw(titleText);
}

void LoginScreen::drawInputFields(sf::RenderWindow& window) {
    auto drawField = [&](const sf::FloatRect& rect, const std::string& label, const std::string& value, bool isActive, bool isPassword) {
        sf::RectangleShape fieldRect(sf::Vector2f(rect.width, rect.height));
        fieldRect.setPosition(rect.left, rect.top);
        fieldRect.setFillColor(sf::Color(30, 40, 60));
        fieldRect.setOutlineThickness(isActive ? 2.0f : 1.0f);
        fieldRect.setOutlineColor(isActive ? sf::Color(0, 191, 255) : sf::Color(100, 100, 100));
        window.draw(fieldRect);

        sf::Text labelText;
        labelText.setFont(font);
        labelText.setString(UI_TEXT(label.c_str()));
        labelText.setCharacterSize(labelFontSize);
        labelText.setFillColor(sf::Color(180, 180, 180));
        labelText.setPosition(rect.left, rect.top - 25.0f);
        window.draw(labelText);

        sf::Text valueText;
        valueText.setFont(font);
        if (isPassword) valueText.setString(std::string(value.length(), '*'));
        else valueText.setString(UI_TEXT(value.c_str()));
        valueText.setCharacterSize(inputFontSize);
        valueText.setFillColor(sf::Color::White);
        valueText.setPosition(rect.left + 10.0f, rect.top + (rect.height - inputFontSize) / 2.0f - 4.0f);
        window.draw(valueText);
        };

    drawField(usernameFieldRect, (const char*)u8"Tài khoản:", usernameInput, usernameInputActive, false);
    drawField(passwordFieldRect, (const char*)u8"Mật khẩu:", passwordInput, passwordInputActive, true);
    if (state == LoginScreenState::REGISTER) {
        drawField(emailFieldRect, (const char*)u8"Email liên hệ:", emailInput, emailInputActive, false);
    }
}

void LoginScreen::drawButtons(sf::RenderWindow& window) {
    auto drawBtn = [&](const sf::FloatRect& rect, const std::string& text, sf::Color color) {
        sf::RectangleShape btn(sf::Vector2f(rect.width, rect.height));
        btn.setPosition(rect.left, rect.top);
        btn.setFillColor(color);
        window.draw(btn);

        sf::Text btnText;
        btnText.setFont(font);
        btnText.setString(UI_TEXT(text.c_str()));
        btnText.setCharacterSize(16);
        btnText.setFillColor(sf::Color::White);
        btnText.setStyle(sf::Text::Bold);

        sf::FloatRect b = btnText.getLocalBounds();
        btnText.setOrigin(b.left + b.width / 2.0f, b.top + b.height / 2.0f);
        btnText.setPosition(rect.left + rect.width / 2.0f, rect.top + rect.height / 2.0f);
        window.draw(btnText);
        };

    if (state == LoginScreenState::LOGIN) {
        drawBtn(loginButtonRect, (const char*)u8"ĐĂNG NHẬP", sf::Color(0, 150, 255));
        drawBtn(switchToRegisterButtonRect, (const char*)u8"TẠO TÀI KHOẢN", sf::Color(80, 80, 80));
    }
    else if (state == LoginScreenState::REGISTER) {
        drawBtn(registerButtonRect, (const char*)u8"ĐĂNG KÝ NGAY", sf::Color(46, 139, 87));
        drawBtn(switchToLoginButtonRect, (const char*)u8"QUAY LẠI", sf::Color(80, 80, 80));
    }
}

void LoginScreen::drawMessages(sf::RenderWindow& window) {
    if (!messageText.empty()) {
        sf::Text msg;
        msg.setFont(font);
        msg.setString(UI_TEXT(messageText.c_str()));
        msg.setCharacterSize(16);
        msg.setFillColor(sf::Color(255, 99, 71));
        sf::FloatRect b = msg.getLocalBounds();
        msg.setOrigin(b.left + b.width / 2.0f, b.top + b.height / 2.0f);
        msg.setPosition(windowWidth / 2.0f, windowHeight * 0.76f);
        window.draw(msg);
    }
}