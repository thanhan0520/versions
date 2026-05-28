#include "Player.h"
#include <iostream>
#include <cmath>
#include <algorithm>

Player::Player()
{
	shape.setSize(sf::Vector2f(50, 50));
	shape.setFillColor(sf::Color::Red);
	shape.setPosition(512, 384);
	speed = 300.0f;
	velocity = sf::Vector2f(0, 0);
	targetPosition = shape.getPosition();
	isMoving = false;
	lastMousePressed = false;
	lastKeyQ = false;
	lastKeyW = false;
	lastKeyE = false;
	lastKeyR = false;

	maxHealth = 100.0f;
	health = maxHealth; // Đảm bảo lượng máu ban đầu đầy cây khi khởi tạo
	baseDamage = 20.0f;
	defenseBoost = 0.0f;

	skillQ_cooldown = 0.0f;
	skillQ_cooldownMax = 0.5f;
	skillW_cooldown = 0.0f;
	skillW_cooldownMax = 1.0f;
	skillE_cooldown = 0.0f;
	skillE_cooldownMax = 3.0f;
	skillE_duration = 0.0f;
	skillR_cooldown = 0.0f;
	skillR_cooldownMax = 2.0f;

	lastProjectilePos = shape.getPosition();

	hasLockedTarget = false;
	lockedTargetPos = sf::Vector2f(0, 0);
	currentClass = CharacterClass::NONE;
}

bool Player::checkCollision(const sf::FloatRect& rect, const std::vector<std::vector<int>>& tiles, int tileSize)
{
	int leftTile = rect.left / tileSize;
	int rightTile = (rect.left + rect.width) / tileSize;
	int topTile = rect.top / tileSize;
	int bottomTile = (rect.top + rect.height) / tileSize;

	for (int y = topTile; y <= bottomTile; y++)
	{
		if (y < 0 || y >= (int)tiles.size()) return true;
		for (int x = leftTile; x <= rightTile; x++)
		{
			if (x < 0 || x >= (int)tiles[0].size()) return true;
			if (tiles[y][x] == 1) return true;
		}
	}
	return false;
}

void Player::update(float dt, const std::vector<std::vector<int>>& tiles, int tileSize, sf::RenderWindow& window)
{
	if (skillQ_cooldown > 0) skillQ_cooldown -= dt;
	if (skillW_cooldown > 0) skillW_cooldown -= dt;
	if (skillE_cooldown > 0) skillE_cooldown -= dt;
	if (skillR_cooldown > 0) skillR_cooldown -= dt;

	if (skillE_duration > 0) {
		skillE_duration -= dt;

		if (skillE_duration <= 0) {
			if (this->currentClass == CharacterClass::RABBIT) {
				this->speed = 500.0f;
				this->baseDamage = 28.0f;
				this->shape.setFillColor(sf::Color(0, 191, 255));
				std::cout << "==> Overclock Mode ended. Core cooled down! <==" << std::endl;
			}
			else if (this->currentClass == CharacterClass::FOX) {
				this->speed = 400.0f;
				this->shape.setFillColor(sf::Color(255, 69, 0));
				std::cout << "==> Shadow Roll ended. Fox returned to normal state! <==" << std::endl;
			}
		}
	}

	bool keyQ = sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
	bool keyW = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
	bool keyE = sf::Keyboard::isKeyPressed(sf::Keyboard::E);
	bool keyR = sf::Keyboard::isKeyPressed(sf::Keyboard::R);

	if (keyQ && !lastKeyQ && skillQ_cooldown <= 0)
	{
		if (hasLockedTarget)
		{
			sf::Vector2f direction = lockedTargetPos - shape.getPosition();

			skillQ(direction);

			skillQ_cooldown = skillQ_cooldownMax;
		}
	}
	lastKeyQ = keyQ;

	if (keyW && !lastKeyW && skillW_cooldown <= 0)
	{
		if (hasLockedTarget)
		{
			sf::Vector2f direction = lockedTargetPos - shape.getPosition();

			skillW(direction, tiles, tileSize);

			skillW_cooldown = skillW_cooldownMax;
		}
	}
	lastKeyW = keyW;

	if (keyE && !lastKeyE && skillE_cooldown <= 0)
	{
		if (hasLockedTarget)
		{
			sf::Vector2f direction = lockedTargetPos - shape.getPosition();

			skillE(direction);

			skillE_cooldown = skillE_cooldownMax;
		}
	}
	lastKeyE = keyE;

	if (keyR && !lastKeyR && skillR_cooldown <= 0)
	{
		if (hasLockedTarget)
		{
			sf::Vector2f direction = lockedTargetPos - shape.getPosition();

			skillR(direction);

			skillR_cooldown = skillR_cooldownMax;
		}
	}
	lastKeyR = keyR;

	for (auto& proj : projectiles)
	{
		proj.update(dt);
		lastProjectilePos = proj.getPosition();
	}

	projectiles.erase(
		std::remove_if(projectiles.begin(), projectiles.end(),
			[](const Bullet& p) { return !p.isAlive(); }),
		projectiles.end()
	);

	bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Right);
	if (mousePressed && !lastMousePressed)
	{
		sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
		targetPosition = window.mapPixelToCoords(mousePixelPos);
		isMoving = true;
	}
	lastMousePressed = mousePressed;

	velocity = sf::Vector2f(0, 0);

	if (isMoving)
	{
		sf::Vector2f currentPos = shape.getPosition();
		sf::Vector2f direction = targetPosition - currentPos;
		float distance = sqrt(direction.x * direction.x + direction.y * direction.y);

		if (distance < 2.0f)
		{
			shape.setPosition(targetPosition);
			isMoving = false;
		}
		else
		{
			velocity.x = direction.x / distance;
			velocity.y = direction.y / distance;
		}
	}

	sf::Vector2f newPos = shape.getPosition();
	newPos.x += velocity.x * speed * dt;
	shape.setPosition(newPos);

	sf::FloatRect playerRect = shape.getGlobalBounds();
	if (checkCollision(playerRect, tiles, tileSize))
	{
		newPos.x -= velocity.x * speed * dt;
		shape.setPosition(newPos);
	}

	newPos = shape.getPosition();
	newPos.y += velocity.y * speed * dt;
	shape.setPosition(newPos);

	playerRect = shape.getGlobalBounds();
	if (checkCollision(playerRect, tiles, tileSize))
	{
		newPos.y -= velocity.y * speed * dt;
		shape.setPosition(newPos);
	}

	static float printTimer = 0;
	printTimer += dt;
	if (printTimer >= 1.0f)
	{
		std::cout << "Health: " << health << "/" << maxHealth << " | ATK: " << baseDamage
			<< " | SPD: " << speed << " | Active Projectiles: " << projectiles.size() << std::endl;
		printTimer = 0;
	}
}

void Player::draw(sf::RenderWindow& window) { window.draw(shape); }
void Player::drawProjectiles(sf::RenderWindow& window) { for (auto& proj : projectiles) proj.draw(window); }
void Player::skillQ(sf::Vector2f direction) {}
void Player::skillW(sf::Vector2f direction,
	const std::vector<std::vector<int>>& tiles,
	int tileSize) {
}
void Player::skillE(sf::Vector2f direction) {}

void Player::skillR(sf::Vector2f direction) {}