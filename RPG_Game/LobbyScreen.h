#pragma once
#include <SFML/Graphics.hpp>
#include "AuthManager.h"
#include "Player.h"

enum class LobbyState {
    DECIDING,
    CHOOSING,
    CHARACTER_SELECTED
};

class LobbyScreen
{
public:
    LobbyScreen(int windowWidth, int windowHeight, AuthManager& auth);
    ~LobbyScreen();

    void updateLayout();
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

    LobbyState getState() const { return state; }
    CharacterClass getSelectedClass() const { return selectedClass; }

    void initLobby(bool hasOldProgress, int oldClass);
    void reset() { state = LobbyState::DECIDING; selectedClass = CharacterClass::NONE; }

private:
    LobbyState state;
    CharacterClass selectedClass;
    AuthManager& authManager;

    int windowWidth;
    int windowHeight;

    sf::Font font;
    bool hasSavedData;
    int savedClassID;

    sf::FloatRect foxButtonRect;
    sf::FloatRect rabbitButtonRect;
    sf::FloatRect snakeButtonRect;
    sf::FloatRect dogButtonRect;

    sf::FloatRect continueButtonRect;
    sf::FloatRect newGameButtonRect;

    void drawBackground(sf::RenderWindow& window);
    void drawDecisionScreen(sf::RenderWindow& window);
    void drawCharacterCards(sf::RenderWindow& window);
};