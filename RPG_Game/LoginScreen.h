#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// Forward Declaration để tránh lỗi vòng lặp include chéo
class AuthManager;

enum class LoginScreenState {
    LOGIN,
    REGISTER,
    AUTHENTICATED
};

class LoginScreen
{
public:
    LoginScreen(int windowWidth, int windowHeight, AuthManager& auth);
    ~LoginScreen();

    void handleEvent(const sf::Event& event);
    void update(float dt);
    void draw(sf::RenderWindow& window);

    LoginScreenState getState() const { return state; }
    std::string getAuthenticatedUser() const;
    bool isAuthenticated() const { return state == LoginScreenState::AUTHENTICATED; }

private:
    void updateLayout();
    void drawBackground(sf::RenderWindow& window);
    void drawInputFields(sf::RenderWindow& window);
    void drawButtons(sf::RenderWindow& window);
    void drawMessages(sf::RenderWindow& window);
    void clearInputs() {
        usernameInput.clear();
        passwordInput.clear();
        emailInput.clear();
    }

    LoginScreenState state;
    int windowWidth;
    int windowHeight;

    std::string usernameInput;
    std::string passwordInput;
    std::string emailInput;
    std::string messageText;

    bool usernameInputActive;
    bool passwordInputActive;
    bool emailInputActive;
    float messageTimer;
    bool isAuthenticatedUser;

    sf::FloatRect usernameFieldRect;
    sf::FloatRect passwordFieldRect;
    sf::FloatRect emailFieldRect;

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
    AuthManager& authManager;
};