#pragma execution_character_set("utf-8")
#include "LobbyScreen.h"
#include <iostream>

#define UI_TEXT(str) sf::String::fromUtf8(reinterpret_cast<const sf::Uint8*>(str), reinterpret_cast<const sf::Uint8*>(str) + std::string(str).length())

LobbyScreen::LobbyScreen(int windowWidth, int windowHeight, AuthManager& auth)
    : state(LobbyState::CHOOSING), selectedClass(CharacterClass::NONE), authManager(auth),
    windowWidth(windowWidth), windowHeight(windowHeight), hasSavedData(false), savedClassID(0)
{
    choseContinue = false;
    deadWarningActive = false;
    if (!font.loadFromFile("C:/Windows/Fonts/segoeui.ttf")) {
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
            std::cerr << "LobbyScreen: Khong the tai font chu!" << std::endl;
        }
    }
    updateLayout();
}

LobbyScreen::~LobbyScreen() {}

void LobbyScreen::initLobby(bool hasOldProgress, int oldClass) {
    hasSavedData = hasOldProgress;
    savedClassID = oldClass;
    // reset choice and recompute layout (in case window changed)
    choseContinue = false;
    updateLayout();

    if (hasSavedData) {
        state = LobbyState::DECIDING; // Nếu có dữ liệu cũ, bắt người chơi chọn "Chơi tiếp" hoặc "Tạo mới"
    }
    else {
        state = LobbyState::CHOOSING; // Nếu tài khoản trống, nhảy vào chọn nhân vật luôn
    }
}

void LobbyScreen::updateLayout() {
    // 1. Tính toán tọa độ cho màn hình chọn 4 linh thú
    float cardWidth = 180.0f;
    float cardHeight = 280.0f;
    float gap = 30.0f;

    float totalWidth = (cardWidth * 4) + (gap * 3);
    float startX = (windowWidth - totalWidth) / 2.0f;
    float startY = windowHeight * 0.35f;

    foxButtonRect = sf::FloatRect(startX, startY, cardWidth, cardHeight);
    rabbitButtonRect = sf::FloatRect(startX + (cardWidth + gap) * 1, startY, cardWidth, cardHeight);
    snakeButtonRect = sf::FloatRect(startX + (cardWidth + gap) * 2, startY, cardWidth, cardHeight);
    dogButtonRect = sf::FloatRect(startX + (cardWidth + gap) * 3, startY, cardWidth, cardHeight);

    // 2. Tính toán tọa độ cho màn hình Quyết định (2 nút: Chơi tiếp / Tạo mới)
    float btnWidth = 220.0f;
    float btnHeight = 55.0f;
    float btnGap = 40.0f;
    float decStartX = (windowWidth - (btnWidth * 2 + btnGap)) / 2.0f;
    float decStartY = windowHeight * 0.45f;

    continueButtonRect = sf::FloatRect(decStartX, decStartY, btnWidth, btnHeight);
    newGameButtonRect = sf::FloatRect(decStartX + btnWidth + btnGap, decStartY, btnWidth, btnHeight);

    // dialog buttons for dead warning (centered)
    float warnBtnWidth = 300.0f;
    float warnBtnHeight = 55.0f;
    float warnGap = 30.0f;
    float warnStartX = (windowWidth - (warnBtnWidth * 2 + warnGap)) / 2.0f;
    float warnStartY = decStartY + btnHeight + 40.0f;
    continueFromSaveRect = sf::FloatRect(warnStartX, warnStartY, warnBtnWidth, warnBtnHeight);
    createNewCharacterRect = sf::FloatRect(warnStartX + warnBtnWidth + warnGap, warnStartY, warnBtnWidth, warnBtnHeight);
}

void LobbyScreen::draw(sf::RenderWindow& window) {
    drawBackground(window);

    if (state == LobbyState::DECIDING) {
        drawDecisionScreen(window);
    }
    else if (state == LobbyState::CHOOSING) {
        drawCharacterCards(window);
    }
}

void LobbyScreen::drawBackground(sf::RenderWindow& window) {
    sf::RectangleShape bg(sf::Vector2f(static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
    bg.setFillColor(sf::Color(15, 20, 35));
    window.draw(bg);

    sf::Text titleText;
    titleText.setFont(font);

    if (state == LobbyState::DECIDING) {
        titleText.setString(UI_TEXT((const char*)u8"BẠN MUỐN TIẾP TỤC HÀNH TRÌNH?"));
    }
    else {
        titleText.setString(UI_TEXT((const char*)u8"CHỌN LINH THÚ ĐẠI DIỆN"));
    }

    titleText.setCharacterSize(34);
    titleText.setFillColor(sf::Color(255, 215, 0));
    titleText.setStyle(sf::Text::Bold);

    sf::FloatRect bounds = titleText.getLocalBounds();
    titleText.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    titleText.setPosition(windowWidth / 2.0f, windowHeight * 0.15f);
    window.draw(titleText);
}

void LobbyScreen::drawDecisionScreen(sf::RenderWindow& window) {
    auto drawBtn = [&](const sf::FloatRect& rect, const std::string& text, sf::Color baseColor) {
        sf::RectangleShape btn(sf::Vector2f(rect.width, rect.height));
        btn.setPosition(rect.left, rect.top);
        btn.setFillColor(baseColor);
        btn.setOutlineThickness(2);
        btn.setOutlineColor(sf::Color(200, 200, 200));
        window.draw(btn);

        sf::Text btnText;
        btnText.setFont(font);
        btnText.setString(UI_TEXT(text.c_str()));
        btnText.setCharacterSize(20);
        btnText.setFillColor(sf::Color::White);
        btnText.setStyle(sf::Text::Bold);

        sf::FloatRect b = btnText.getLocalBounds();
        btnText.setOrigin(b.left + b.width / 2.0f, b.top + b.height / 2.0f);
        btnText.setPosition(rect.left + rect.width / 2.0f, rect.top + rect.height / 2.0f);
        window.draw(btnText);
        };

    drawBtn(continueButtonRect, (const char*)u8"CHƠI TIẾP", sf::Color(34, 139, 34)); // Xanh lá cây
    drawBtn(newGameButtonRect, (const char*)u8"TẠO MỚI", sf::Color(178, 34, 34));    // Đỏ gạch

    // If dead warning active, draw message and two larger buttons
    if (deadWarningActive) {
        // Darken background a bit
        sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
        overlay.setFillColor(sf::Color(0,0,0,160));
        window.draw(overlay);

        sf::Text warnTitle;
        warnTitle.setFont(font);
        warnTitle.setString(UI_TEXT((const char*)u8"Nhân vật của bạn đã thất bại ở lần chơi trước.\nBạn muốn tiếp tục từ điểm lưu gần nhất hay bắt đầu lại?"));
        warnTitle.setCharacterSize(20);
        warnTitle.setFillColor(sf::Color::White);

        sf::FloatRect wb = warnTitle.getLocalBounds();
        warnTitle.setOrigin(wb.left + wb.width/2.0f, wb.top + wb.height/2.0f);
        warnTitle.setPosition(windowWidth/2.0f, windowHeight*0.35f);
        window.draw(warnTitle);

        // Buttons
        auto drawWarnBtn = [&](const sf::FloatRect& rect, const std::string& text, sf::Color color){
            sf::RectangleShape btn(sf::Vector2f(rect.width, rect.height));
            btn.setPosition(rect.left, rect.top);
            btn.setFillColor(color);
            btn.setOutlineThickness(2);
            btn.setOutlineColor(sf::Color(200,200,200));
            window.draw(btn);

            sf::Text btnText;
            btnText.setFont(font);
            btnText.setString(UI_TEXT(text.c_str()));
            btnText.setCharacterSize(18);
            btnText.setFillColor(sf::Color::White);
            sf::FloatRect b = btnText.getLocalBounds();
            btnText.setOrigin(b.left + b.width/2.0f, b.top + b.height/2.0f);
            btnText.setPosition(rect.left + rect.width/2.0f, rect.top + rect.height/2.0f);
            window.draw(btnText);
        };

        drawWarnBtn(continueFromSaveRect, (const char*)u8"Tiếp tục từ điểm lưu", sf::Color(34,139,34));
        drawWarnBtn(createNewCharacterRect, (const char*)u8"Tạo nhân vật mới", sf::Color(178,34,34));
    }
}

void LobbyScreen::drawCharacterCards(sf::RenderWindow& window) {
    struct CardData {
        sf::FloatRect rect;
        std::string name;
        sf::Color color;
    };

    CardData cards[4] = {
        { foxButtonRect,    (const char*)u8"CÁO LINH HOẠT", sf::Color(230, 95, 0) },
        { rabbitButtonRect, (const char*)u8"THỎ TỐC ĐỘ",   sf::Color(220, 220, 220) },
        { snakeButtonRect,  (const char*)u8"RẮN ĐỘC DƯỢC", sf::Color(45, 160, 45) },
        { dogButtonRect,    (const char*)u8"CHÓ DŨNG MÃNH", sf::Color(140, 100, 60) }
    };

    for (int i = 0; i < 4; i++) {
        sf::RectangleShape card(sf::Vector2f(cards[i].rect.width, cards[i].rect.height));
        card.setPosition(cards[i].rect.left, cards[i].rect.top);
        card.setFillColor(sf::Color(25, 35, 55));
        card.setOutlineThickness(3);
        card.setOutlineColor(cards[i].color);
        window.draw(card);

        sf::CircleShape avatar(40.0f);
        avatar.setFillColor(cards[i].color);
        avatar.setOrigin(40.0f, 40.0f);
        avatar.setPosition(cards[i].rect.left + cards[i].rect.width / 2.0f, cards[i].rect.top + 90.0f);
        window.draw(avatar);

        sf::Text nameTxt;
        nameTxt.setFont(font);
        nameTxt.setString(UI_TEXT(cards[i].name.c_str()));
        nameTxt.setCharacterSize(18);
        nameTxt.setFillColor(sf::Color::White);

        sf::FloatRect b = nameTxt.getLocalBounds();
        nameTxt.setOrigin(b.left + b.width / 2.0f, b.top + b.height / 2.0f);
        nameTxt.setPosition(cards[i].rect.left + cards[i].rect.width / 2.0f, cards[i].rect.top + cards[i].rect.height - 40.0f);
        window.draw(nameTxt);
    }

    // (No character hover description panel)
}

void LobbyScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));

        // TẦNG 1: XỬ LÝ MÀN HÌNH QUYẾT ĐỊNH
        if (state == LobbyState::DECIDING) {
            if (continueButtonRect.contains(mousePos)) {
                // Người chơi chọn chơi tiếp -> need to check if saved full state has HP<=0
                std::string fullState;
                bool hasFull = authManager.loadFullGameState(fullState);
                bool prevDead = false;
                if (hasFull && !fullState.empty()) {
                    // parse simple serialized format: playerX,playerY,playerHP;... -> extract playerHP
                    size_t semi = fullState.find(';');
                    std::string playerPart = (semi == std::string::npos) ? fullState : fullState.substr(0, semi);
                    // playerPart: x,y,hp
                    std::vector<std::string> parts;
                    std::string cur;
                    for (char c : playerPart) { if (c == ',') { parts.push_back(cur); cur.clear(); } else cur.push_back(c); }
                    if (!cur.empty()) parts.push_back(cur);
                    if (parts.size() >= 3) {
                        try {
                            int php = std::stoi(parts[2]);
                            if (php <= 0) prevDead = true;
                        } catch(...) {}
                    }
                }

                if (prevDead) {
                    // Show warning dialog
                    deadWarningActive = true;
                    std::cout << "Lobby: Phat hien nhan vat da chet lan truoc -> hien thong bao" << std::endl;
                } else {
                    // proceed normally
                    choseContinue = true;
                    selectedClass = static_cast<CharacterClass>(savedClassID);
                    state = LobbyState::CHARACTER_SELECTED; // Cho qua màn sảnh luôn
                    std::cout << "Lobby: Nguoi choi chon CHOI TIEP voi nhan vat cu." << std::endl;
                }
            }
            else if (newGameButtonRect.contains(mousePos)) {
                // Người chơi muốn tạo mới -> Chuyển sang màn hình chọn 4 con vật
                choseContinue = false;
                state = LobbyState::CHOOSING;
                std::cout << "Lobby: Nguoi choi chon TAO MOI, mo menu chon linh thu..." << std::endl;
            }
            // Handle dead-warning dialog clicks
            if (deadWarningActive) {
                if (continueFromSaveRect.contains(mousePos)) {
                    // User wants to continue from save despite death -> show game-over screen as-is
                    choseContinue = true;
                    selectedClass = static_cast<CharacterClass>(savedClassID);
                    state = LobbyState::CHARACTER_SELECTED;
                    deadWarningActive = false;
                    std::cout << "Lobby: Chon 'Tiep tuc tu diem luu' -> tiep tuc voi game-over screen" << std::endl;
                }
                else if (createNewCharacterRect.contains(mousePos)) {
                    // Go to character selection to start over
                    deadWarningActive = false;
                    choseContinue = false;
                    state = LobbyState::CHOOSING;
                    std::cout << "Lobby: Chon 'Tao nhan vat moi' -> choi lai tu dau" << std::endl;
                }
            }
        }
        // TẦNG 2: XỬ LÝ MÀN HÌNH CHỌN LINH THÚ (KHI TẠO MỚI)
        else if (state == LobbyState::CHOOSING) {
            CharacterClass chosen = CharacterClass::NONE;

            if (foxButtonRect.contains(mousePos))         chosen = CharacterClass::FOX;
            else if (rabbitButtonRect.contains(mousePos))  chosen = CharacterClass::RABBIT;
            else if (snakeButtonRect.contains(mousePos))   chosen = CharacterClass::SNAKE;
            else if (dogButtonRect.contains(mousePos))     chosen = CharacterClass::DOG;

            if (chosen != CharacterClass::NONE) {
                selectedClass = chosen;
                state = LobbyState::CHARACTER_SELECTED;

                int charClassInt = static_cast<int>(chosen);
                // Khởi tạo ghi đè dữ liệu mới tinh xuống SQL Server
                bool saved = authManager.saveGameProgress(1, 100, 0, charClassInt);

                    if (saved) {
                    std::cout << "Lobby: Da luu nhan vat moi (ID: " << charClassInt << ") len Database" << std::endl;
                }
                else {
                    std::cerr << "Lobby: Loi! Khong the luu nhan vat vao Database." << std::endl;
                }
            }
        }
    }
}