#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "AuthManager.h"

enum class LoginScreenState
{
    LOGIN,       
    REGISTER,   
    AUTHENTICATED   
};

class LoginScreen
{
public:
    LoginScreen(int windowWidth, int windowHeight);
    ~LoginScreen();

    void handleEvent(const sf::Event& event);
    void update(float dt);
    void draw(sf::RenderWindow& window);

    LoginScreenState getState() const { return state; }
    bool isAuthenticated() const { return state == LoginScreenState::AUTHENTICATED; }
    std::string getAuthenticatedUser() const { return authManager.getCurrentUser(); }

private:
    void drawInputFields(sf::RenderWindow& window);
    void drawButtons(sf::RenderWindow& window);
    void drawMessages(sf::RenderWindow& window);
    void drawBackground(sf::RenderWindow& window);
    void drawAnimals(sf::RenderWindow& window);
    void drawTitle(sf::RenderWindow& window);
    void handleTextInput(sf::Uint32 unicode);
    bool isButtonClicked(const sf::Vector2f& mousePos, const sf::FloatRect& buttonRect);
    void updateLayout();  // Tính toán layout responsive

    void clearInputs();
    void switchToRegister();
    void switchToLogin();
    void attemptLogin();
    void attemptRegister();

    AuthManager authManager;
    LoginScreenState state;

    sf::Font font;
    int windowWidth, windowHeight;

    // Responsive layout
    float centerX, centerY;
    float fieldWidth, fieldHeight;
    float buttonWidth, buttonHeight;
    float labelFontSize, inputFontSize, buttonFontSize;
    float spacing;

    // Input fields
    std::string usernameInput;
    std::string passwordInput;
    bool usernameInputActive;
    bool passwordInputActive;

    // Messages
    std::string messageText;
    float messageTimer;

    // Button rectangles
    sf::FloatRect loginButtonRect;
    sf::FloatRect registerButtonRect;
    sf::FloatRect switchToRegisterButtonRect;
    sf::FloatRect switchToLoginButtonRect;
    sf::FloatRect forgotPasswordButtonRect;

    // Input field rectangles
    sf::FloatRect usernameFieldRect;
    sf::FloatRect passwordFieldRect;
};