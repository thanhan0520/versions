#pragma execution_character_set("utf-8")
#include "LoginScreen.h"
#include "TextHelper.h"
#include <iostream>
#include <algorithm>

LoginScreen::LoginScreen(int windowWidth, int windowHeight)
    : state(LoginScreenState::LOGIN), windowWidth(windowWidth), windowHeight(windowHeight),
    usernameInputActive(true), passwordInputActive(false), messageTimer(0)
{
    // Tải font hỗ trợ tiếng Việt - thử các font khác nhau
    std::vector<std::string> fontPaths = {
        "C:\\Windows\\Fonts\\segoeui.ttf",      // Segoe UI - hỗ trợ Unicode tốt nhất
        "C:\\Windows\\Fonts\\arial.ttf",        // Arial
        "C:\\Windows\\Fonts\\times.ttf",        // Times New Roman
        "C:\\Windows\\Fonts\\calibri.ttf",      // Calibri
        "C:\\Windows\\Fonts\\tahoma.ttf"        // Tahoma
    };

    bool fontLoaded = false;
    for (const auto& fontPath : fontPaths)
    {
        if (font.loadFromFile(fontPath))
        {
            std::cout << "Font tải thành công: " << fontPath << std::endl;
            fontLoaded = true;
            break;
        }
    }

    if (!fontLoaded)
    {
        std::cout << "Cảnh báo: Không thể tải font từ Windows Fonts!" << std::endl;
        std::cout << "Hãy đảm bảo các font Windows được cài đặt." << std::endl;
    }

    // Tính toán layout responsive
    updateLayout();
}

LoginScreen::~LoginScreen()
{
}

void LoginScreen::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::TextEntered)
    {
        handleTextInput(event.text.unicode);
    }
    else if (event.type == sf::Event::MouseButtonPressed)
    {
        sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x),
            static_cast<float>(event.mouseButton.y));

        // Kiểm tra click trên input fields
        if (isButtonClicked(mousePos, usernameFieldRect))
        {
            usernameInputActive = true;
            passwordInputActive = false;
        }
        else if (isButtonClicked(mousePos, passwordFieldRect))
        {
            usernameInputActive = false;
            passwordInputActive = true;
        }
        // Kiểm tra click trên nút Login/Register
        else if (state == LoginScreenState::LOGIN && isButtonClicked(mousePos, loginButtonRect))
        {
            attemptLogin();
        }
        else if (state == LoginScreenState::REGISTER && isButtonClicked(mousePos, registerButtonRect))
        {
            attemptRegister();
        }
        // Kiểm tra click trên nút chuyển đổi
        else if (state == LoginScreenState::LOGIN && isButtonClicked(mousePos, switchToRegisterButtonRect))
        {
            switchToRegister();
        }
        else if (state == LoginScreenState::REGISTER && isButtonClicked(mousePos, switchToLoginButtonRect))
        {
            switchToLogin();
        }
        // Kiểm tra click trên Quên mật khẩu
        else if (state == LoginScreenState::LOGIN && isButtonClicked(mousePos, forgotPasswordButtonRect))
        {
            messageText = "Chức năng quên mật khẩu sẽ sớm cập nhật!";
            messageTimer = 3.0f;
        }
        else
        {
            usernameInputActive = false;
            passwordInputActive = false;
        }
    }
    else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return)
    {
        if (state == LoginScreenState::LOGIN)
            attemptLogin();
        else if (state == LoginScreenState::REGISTER)
            attemptRegister();
    }
}

void LoginScreen::handleTextInput(sf::Uint32 unicode)
{
    if (usernameInputActive)
    {
        if (unicode == 8 && !usernameInput.empty())  // Backspace
        {
            usernameInput.pop_back();
        }
        else if (unicode >= 32 && unicode < 127 && usernameInput.length() < 20)
        {
            usernameInput += static_cast<char>(unicode);
        }
    }
    else if (passwordInputActive)
    {
        if (unicode == 8 && !passwordInput.empty())  // Backspace
        {
            passwordInput.pop_back();
        }
        else if (unicode >= 32 && unicode < 127 && passwordInput.length() < 20)
        {
            passwordInput += static_cast<char>(unicode);
        }
    }
}

bool LoginScreen::isButtonClicked(const sf::Vector2f& mousePos, const sf::FloatRect& buttonRect)
{
    return buttonRect.contains(mousePos);
}

void LoginScreen::clearInputs()
{
    usernameInput.clear();
    passwordInput.clear();
    usernameInputActive = true;
    passwordInputActive = false;
}

void LoginScreen::switchToRegister()
{
    state = LoginScreenState::REGISTER;
    clearInputs();
    messageText.clear();
}

void LoginScreen::switchToLogin()
{
    state = LoginScreenState::LOGIN;
    clearInputs();
    messageText.clear();
}

void LoginScreen::attemptLogin()
{
    if (usernameInput.empty() || passwordInput.empty())
    {
        messageText = "Vui lòng nhập tên đăng nhập và mật khẩu!";
        messageTimer = 3.0f;
        return;
    }

    if (authManager.login(usernameInput, passwordInput))
    {
        messageText = "Đăng nhập thành công!";
        messageTimer = 1.0f;
        state = LoginScreenState::AUTHENTICATED;
    }
    else
    {
        messageText = "Sai tên đăng nhập hoặc mật khẩu!";
        messageTimer = 3.0f;
        passwordInput.clear();
    }
}

void LoginScreen::attemptRegister()
{
    if (usernameInput.empty() || passwordInput.empty())
    {
        messageText = "Vui lòng nhập tên đăng nhập và mật khẩu!";
        messageTimer = 3.0f;
        return;
    }

    if (usernameInput.length() < 3)
    {
        messageText = "Tên đăng nhập phải ít nhất 3 ký tự!";
        messageTimer = 3.0f;
        return;
    }

    if (authManager.registerAccount(usernameInput, passwordInput))
    {
        messageText = "Đăng ký thành công! Đang đăng nhập...";
        messageTimer = 2.0f;
        authManager.login(usernameInput, passwordInput);
        state = LoginScreenState::AUTHENTICATED;
    }
    else
    {
        messageText = "Tên đăng nhập đã được sử dụng!";
        messageTimer = 3.0f;
    }
}

void LoginScreen::update(float dt)
{
    if (messageTimer > 0)
        messageTimer -= dt;
}

void LoginScreen::drawBackground(sf::RenderWindow& window)
{
    // Vẽ nền chính với màu teal/xanh dương
    sf::RectangleShape background(sf::Vector2f(windowWidth, windowHeight));
    background.setFillColor(sf::Color(45, 85, 105));  // Teal color
    window.draw(background);

    // Vẽ gradient effects ở góc
    // Top gradient circle (xanh nhạt)
    sf::CircleShape topLeftCircle(windowHeight * 0.35f);
    topLeftCircle.setFillColor(sf::Color(80, 140, 160, 25));
    topLeftCircle.setPosition(-windowHeight * 0.2f, -windowHeight * 0.2f);
    window.draw(topLeftCircle);

    // Bottom-right gradient circle (tím nhạt)
    sf::CircleShape bottomRightCircle(windowHeight * 0.3f);
    bottomRightCircle.setFillColor(sf::Color(120, 100, 140, 20));
    bottomRightCircle.setPosition(windowWidth - windowHeight * 0.15f, windowHeight - windowHeight * 0.2f);
    window.draw(bottomRightCircle);
}

void LoginScreen::draw(sf::RenderWindow& window)
{
    // Vẽ background
    drawBackground(window);

    // Vẽ các biểu tượng động vật ở bên trái
    drawAnimals(window);

    // Vẽ title
    drawTitle(window);

    // Vẽ input fields
    drawInputFields(window);

    // Vẽ buttons
    drawButtons(window);

    // Vẽ messages
    drawMessages(window);
}

void LoginScreen::drawAnimals(sf::RenderWindow& window)
{
    // Vẽ các biểu tượng động vật ở bên trái (đơn giản với hình tròn)
    float animalSize = 50.0f * (windowHeight / 600.0f);
    float animalStartX = centerX * 0.25f;  // Bên trái 1/4 màn hình
    float animalStartY = centerY - spacing * 2;

    // Chuẩn bị các vòng tròn đại diện cho các động vật
    sf::Color animalColors[] = {
        sf::Color(200, 120, 80),    // Bunny - nâu
        sf::Color(230, 100, 50),    // Fox - cam đỏ
        sf::Color(100, 100, 100),   // Dog - xám
        sf::Color(150, 200, 80)     // Snake - xanh
    };

    int animalCount = 4;
    for (int i = 0; i < animalCount; i++)
    {
        sf::CircleShape animal(animalSize / 2);
        float x = animalStartX + (i % 2) * (animalSize + 20);
        float y = animalStartY + (i / 2) * (animalSize + 20);
        animal.setPosition(x, y);
        animal.setFillColor(animalColors[i]);
        window.draw(animal);

        // Vẽ outline để làm nổi bật
        animal.setFillColor(sf::Color::Transparent);
        animal.setOutlineThickness(2.0f);
        animal.setOutlineColor(sf::Color(240, 220, 180, 100));
        window.draw(animal);
    }
}

void LoginScreen::drawTitle(sf::RenderWindow& window)
{
    // Vẽ "ZD" text lớn
    sf::Text zdText;
    zdText.setFont(font);
    zdText.setString("ZD");
    zdText.setCharacterSize(static_cast<unsigned int>(90.0f * (windowHeight / 600.0f)));
    zdText.setFillColor(sf::Color(240, 220, 180));

    sf::FloatRect zdBounds = zdText.getLocalBounds();
    zdText.setPosition(centerX - zdBounds.width / 2, centerY * 0.35f);
    window.draw(zdText);

    // Vẽ "ZOO DUNGEON" subtitle
    sf::Text subtitleText;
    subtitleText.setFont(font);
    subtitleText.setString(TextHelper::toSFString("ZOO DUNGEON"));
    subtitleText.setCharacterSize(static_cast<unsigned int>(20.0f * (windowHeight / 600.0f)));
    subtitleText.setFillColor(sf::Color(240, 220, 180));

    sf::FloatRect subtitleBounds = subtitleText.getLocalBounds();
    subtitleText.setPosition(centerX - subtitleBounds.width / 2, centerY * 0.5f);
    window.draw(subtitleText);
}

void LoginScreen::drawInputFields(sf::RenderWindow& window)
{
    // Username label
    sf::Text usernameLabel;
    usernameLabel.setFont(font);
    usernameLabel.setString(TextHelper::toSFString("Tên người dùng:"));
    usernameLabel.setCharacterSize(static_cast<unsigned int>(labelFontSize));
    usernameLabel.setFillColor(sf::Color::White);
    usernameLabel.setPosition(usernameFieldRect.left, usernameFieldRect.top - labelFontSize - spacing * 0.5f);
    window.draw(usernameLabel);

    // Username input box
    sf::RectangleShape usernameBox(sf::Vector2f(usernameFieldRect.width, usernameFieldRect.height));
    usernameBox.setPosition(usernameFieldRect.left, usernameFieldRect.top);
    usernameBox.setFillColor(sf::Color(50, 50, 60));
    usernameBox.setOutlineThickness(2.0f);
    usernameBox.setOutlineColor(usernameInputActive ? sf::Color(100, 200, 255) : sf::Color(100, 100, 120));
    window.draw(usernameBox);

    // Username text
    sf::Text usernameText;
    usernameText.setFont(font);
    usernameText.setString(usernameInput);
    usernameText.setCharacterSize(static_cast<unsigned int>(inputFontSize));
    usernameText.setFillColor(sf::Color::White);
    usernameText.setPosition(usernameFieldRect.left + 12, usernameFieldRect.top + 8);
    window.draw(usernameText);

    // Password label
    sf::Text passwordLabel;
    passwordLabel.setFont(font);
    passwordLabel.setString(TextHelper::toSFString("Mật khẩu:"));
    passwordLabel.setCharacterSize(static_cast<unsigned int>(labelFontSize));
    passwordLabel.setFillColor(sf::Color::White);
    passwordLabel.setPosition(passwordFieldRect.left, passwordFieldRect.top - labelFontSize - spacing * 0.5f);
    window.draw(passwordLabel);

    // Password input box
    sf::RectangleShape passwordBox(sf::Vector2f(passwordFieldRect.width, passwordFieldRect.height));
    passwordBox.setPosition(passwordFieldRect.left, passwordFieldRect.top);
    passwordBox.setFillColor(sf::Color(50, 50, 60));
    passwordBox.setOutlineThickness(2.0f);
    passwordBox.setOutlineColor(passwordInputActive ? sf::Color(100, 200, 255) : sf::Color(100, 100, 120));
    window.draw(passwordBox);

    // Password text (hiển thị * thay vì ký tự thật)
    sf::Text passwordText;
    passwordText.setFont(font);
    passwordText.setString(std::string(passwordInput.length(), '*'));
    passwordText.setCharacterSize(static_cast<unsigned int>(inputFontSize));
    passwordText.setFillColor(sf::Color::White);
    passwordText.setPosition(passwordFieldRect.left + 12, passwordFieldRect.top + 8);
    window.draw(passwordText);
}

void LoginScreen::drawButtons(sf::RenderWindow& window)
{
    // Main button (Login hoặc Register)
    std::string mainButtonLabel = state == LoginScreenState::LOGIN ? "Đăng Nhập" : "Đăng Ký";
    sf::Color mainButtonColor = sf::Color(76, 175, 80);  // Màu xanh lá cây

    // Tính toán vị trí button
    float buttonY = passwordFieldRect.top + passwordFieldRect.height + spacing * 1.5f;

    sf::RectangleShape mainButton(sf::Vector2f(buttonWidth, buttonHeight));
    mainButton.setPosition(usernameFieldRect.left, buttonY);
    mainButton.setFillColor(mainButtonColor);
    mainButton.setOutlineThickness(2.0f);
    mainButton.setOutlineColor(sf::Color(100, 200, 100));
    window.draw(mainButton);
    loginButtonRect = mainButton.getGlobalBounds();
    registerButtonRect = mainButton.getGlobalBounds();

    sf::Text mainButtonText;
    mainButtonText.setFont(font);
    mainButtonText.setString(TextHelper::toSFString(mainButtonLabel));
    mainButtonText.setCharacterSize(static_cast<unsigned int>(buttonFontSize));
    mainButtonText.setFillColor(sf::Color::White);

    // Căn giữa text trong button
    sf::FloatRect textBounds = mainButtonText.getLocalBounds();
    mainButtonText.setPosition(
        usernameFieldRect.left + buttonWidth / 2 - textBounds.width / 2,
        buttonY + buttonHeight / 2 - textBounds.height / 2 - 3
    );
    window.draw(mainButtonText);

    // Switch button và Quên mật khẩu button (dành cho LOGIN) hoặc Switch button (dành cho REGISTER)
    if (state == LoginScreenState::LOGIN)
    {
        // Đăng ký button
        float switchButtonWidth = (buttonWidth - spacing / 2) * 0.5f;
        float switchButtonY = buttonY + buttonHeight + spacing * 1.2f;

        sf::RectangleShape switchButton(sf::Vector2f(switchButtonWidth, buttonHeight * 0.75f));
        switchButton.setPosition(usernameFieldRect.left, switchButtonY);
        switchButton.setFillColor(sf::Color(80, 80, 100));
        switchButton.setOutlineThickness(1.0f);
        switchButton.setOutlineColor(sf::Color(150, 150, 150));
        window.draw(switchButton);
        switchToRegisterButtonRect = switchButton.getGlobalBounds();

        sf::Text switchButtonText;
        switchButtonText.setFont(font);
        switchButtonText.setString(TextHelper::toSFString("Đăng Ký"));
        switchButtonText.setCharacterSize(static_cast<unsigned int>(buttonFontSize - 2));
        switchButtonText.setFillColor(sf::Color(150, 200, 255));

        sf::FloatRect switchTextBounds = switchButtonText.getLocalBounds();
        switchButtonText.setPosition(
            usernameFieldRect.left + switchButtonWidth / 2 - switchTextBounds.width / 2,
            switchButtonY + buttonHeight * 0.375f - switchTextBounds.height / 2
        );
        window.draw(switchButtonText);

        // Quên mật khẩu button
        sf::RectangleShape forgotButton(sf::Vector2f(switchButtonWidth, buttonHeight * 0.75f));
        forgotButton.setPosition(usernameFieldRect.left + switchButtonWidth + spacing / 2, switchButtonY);
        forgotButton.setFillColor(sf::Color(80, 80, 100));
        forgotButton.setOutlineThickness(1.0f);
        forgotButton.setOutlineColor(sf::Color(150, 150, 150));
        window.draw(forgotButton);
        forgotPasswordButtonRect = forgotButton.getGlobalBounds();

        sf::Text forgotButtonText;
        forgotButtonText.setFont(font);
        forgotButtonText.setString(TextHelper::toSFString("Quên mật khẩu?"));
        forgotButtonText.setCharacterSize(static_cast<unsigned int>(buttonFontSize - 2));
        forgotButtonText.setFillColor(sf::Color(150, 200, 255));

        sf::FloatRect forgotTextBounds = forgotButtonText.getLocalBounds();
        forgotButtonText.setPosition(
            usernameFieldRect.left + switchButtonWidth + spacing / 2 + switchButtonWidth / 2 - forgotTextBounds.width / 2,
            switchButtonY + buttonHeight * 0.375f - forgotTextBounds.height / 2
        );
        window.draw(forgotButtonText);
    }
    else
    {
        // REGISTER mode - chỉ có nút Đăng nhập
        float switchButtonWidth = buttonWidth;
        float switchButtonY = buttonY + buttonHeight + spacing * 1.2f;

        sf::RectangleShape switchButton(sf::Vector2f(switchButtonWidth, buttonHeight * 0.75f));
        switchButton.setPosition(usernameFieldRect.left, switchButtonY);
        switchButton.setFillColor(sf::Color(80, 80, 100));
        switchButton.setOutlineThickness(1.0f);
        switchButton.setOutlineColor(sf::Color(150, 150, 150));
        window.draw(switchButton);
        switchToLoginButtonRect = switchButton.getGlobalBounds();

        sf::Text switchButtonText;
        switchButtonText.setFont(font);
        switchButtonText.setString(TextHelper::toSFString("Có tài khoản? Đăng nhập"));
        switchButtonText.setCharacterSize(static_cast<unsigned int>(buttonFontSize - 2));
        switchButtonText.setFillColor(sf::Color(150, 200, 255));

        sf::FloatRect switchTextBounds = switchButtonText.getLocalBounds();
        switchButtonText.setPosition(
            usernameFieldRect.left + switchButtonWidth / 2 - switchTextBounds.width / 2,
            switchButtonY + buttonHeight * 0.375f - switchTextBounds.height / 2
        );
        window.draw(switchButtonText);
    }
}

void LoginScreen::drawMessages(sf::RenderWindow& window)
{
    if (messageTimer > 0)
    {
        sf::Text messageText;
        messageText.setFont(font);
        messageText.setString(TextHelper::toSFString(this->messageText));
        messageText.setCharacterSize(static_cast<unsigned int>(buttonFontSize));
        messageText.setFillColor(sf::Color(255, 150, 100));

        sf::FloatRect messageBounds = messageText.getLocalBounds();
        messageText.setPosition(centerX - messageBounds.width / 2, centerY * 1.7f);
        window.draw(messageText);
    }
}

void LoginScreen::updateLayout()
{
    // Tính toán kích thước dựa trên màn hình
    centerX = windowWidth / 2.0f;
    centerY = windowHeight / 2.0f;

    // Responsive sizing dựa trên chiều cao
    float scale = windowHeight / 600.0f;

    // Kích thước fields & buttons - phù hợp với 16:9
    fieldWidth = std::min(350.0f, windowWidth * 0.35f);
    fieldHeight = 40.0f * scale;
    buttonWidth = fieldWidth;
    buttonHeight = 40.0f * scale;

    // Font sizes
    labelFontSize = 16.0f * scale;
    inputFontSize = 14.0f * scale;
    buttonFontSize = 14.0f * scale;
    spacing = 12.0f * scale;

    // Tính toán vị trí input fields - căn phải, để chỗ cho các biểu tượng động vật bên trái
    float inputStartX = centerX * 0.6f;  // Bên phải 1/3 màn hình
    float inputStartY = centerY * 0.85f;

    usernameFieldRect = sf::FloatRect(inputStartX, inputStartY, fieldWidth, fieldHeight);
    passwordFieldRect = sf::FloatRect(inputStartX, inputStartY + fieldHeight + spacing * 1.2f, fieldWidth, fieldHeight);
}