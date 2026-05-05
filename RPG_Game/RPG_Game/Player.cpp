#include "Player.h"
#include <iostream>
#include <cmath>

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

	// Stats
	maxHealth = 100.0f;
	health = maxHealth;
	baseDamage = 20.0f;
	defenseBoost = 0.0f;

	// Cooldown
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

	// Target locking
	hasLockedTarget = false;
	lockedTargetPos = sf::Vector2f(0, 0);
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

            if (tiles[y][x] == 1) // tường
                return true;
        }
    }
    return false;
}

void Player::update(float dt, const std::vector<std::vector<int>>& tiles, int tileSize, sf::RenderWindow& window)
{
	// Cập nhật cooldown
	if (skillQ_cooldown > 0) skillQ_cooldown -= dt;
	if (skillW_cooldown > 0) skillW_cooldown -= dt;
	if (skillE_cooldown > 0) skillE_cooldown -= dt;
	if (skillE_duration > 0) skillE_duration -= dt;
	if (skillR_cooldown > 0) skillR_cooldown -= dt;

	// Kiểm tra input skill
	bool keyQ = sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
	bool keyW = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
	bool keyE = sf::Keyboard::isKeyPressed(sf::Keyboard::E);
	bool keyR = sf::Keyboard::isKeyPressed(sf::Keyboard::R);

	// Q - Bắn projectile
	if (keyQ && !lastKeyQ && skillQ_cooldown <= 0)
	{
		sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
		sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixelPos);
		sf::Vector2f direction = mouseWorldPos - shape.getPosition();
		skillQ(direction);
		skillQ_cooldown = skillQ_cooldownMax;
		std::cout << "Q skill used!" << std::endl;
	}
	lastKeyQ = keyQ;

	// W - Dịch chuyển đến projectile + dame
	if (keyW && !lastKeyW && skillW_cooldown <= 0 && !projectiles.empty())
	{
		skillW();
		skillW_cooldown = skillW_cooldownMax;
		std::cout << "W skill used!" << std::endl;
	}
	lastKeyW = keyW;

	// E - Cường hóa
	if (keyE && !lastKeyE && skillE_cooldown <= 0)
	{
		skillE();
		skillE_cooldown = skillE_cooldownMax;
		std::cout << "E skill used! Damage boosted!" << std::endl;
	}
	lastKeyE = keyE;

	// R - Damage liên tiếp 3 lần
	if (keyR && !lastKeyR && skillR_cooldown <= 0)
	{
		skillR();
		skillR_cooldown = skillR_cooldownMax;
		std::cout << "R skill used! 3x damage!" << std::endl;
	}
	lastKeyR = keyR;

	// Cập nhật projectiles
	for (auto& proj : projectiles)
	{
		proj.update(dt);
		lastProjectilePos = proj.getPosition();
	}

	// Xóa projectiles đã chết
	projectiles.erase(
		std::remove_if(projectiles.begin(), projectiles.end(),
			[](const nv1& p) { return !p.isAlive(); }),
		projectiles.end()
	);

	// Kiểm tra click chuột phải (chỉ khi nhấn xuống)
	bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Right);
	if (mousePressed && !lastMousePressed)
	{
		// Chuyển đổi từ window coordinates sang world coordinates
		sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
		targetPosition = window.mapPixelToCoords(mousePixelPos);
		isMoving = true;
		std::cout << "Moving to: (" << targetPosition.x << ", " << targetPosition.y << ")" << std::endl;
	}
	lastMousePressed = mousePressed;

	velocity = sf::Vector2f(0, 0);

	// Nếu đang di chuyển, tính hướng đến đích
	if (isMoving)
	{
		sf::Vector2f currentPos = shape.getPosition();
		sf::Vector2f direction = targetPosition - currentPos;
		float distance = sqrt(direction.x * direction.x + direction.y * direction.y);

		// Nếu đến gần đích, đặt vị trí chính xác và dừng lại
		if (distance < 2.0f)
		{
			shape.setPosition(targetPosition);
			isMoving = false;
			std::cout << "Reached target at: (" << targetPosition.x << ", " << targetPosition.y << ")" << std::endl;
		}
		else
		{
			// Chuẩn hóa hướng di chuyển
			velocity.x = direction.x / distance;
			velocity.y = direction.y / distance;
		}
	}

	// Di chuyển theo X trước
	sf::Vector2f newPos = shape.getPosition();
	newPos.x += velocity.x * speed * dt;
	shape.setPosition(newPos);

	sf::FloatRect playerRect = shape.getGlobalBounds();
	if (checkCollision(playerRect, tiles, tileSize))
	{
		newPos.x -= velocity.x * speed * dt;
		shape.setPosition(newPos);
	}

	// Di chuyển theo Y sau
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
		std::cout << "Health: " << health << "/" << maxHealth << " | Damage: " << baseDamage 
			<< " | Defense: " << defenseBoost << " | Q cooldown: " << skillQ_cooldown 
			<< " | Projectiles: " << projectiles.size() << std::endl;
		if (skillE_duration > 0)
			std::cout << "E buff active! (" << skillE_duration << "s remaining)" << std::endl;
		printTimer = 0;
	}
}

void Player::draw(sf::RenderWindow& window)
{
	window.draw(shape);
}

void Player::drawProjectiles(sf::RenderWindow& window)
{
	for (auto& proj : projectiles)
	{
		proj.draw(window);
	}
}

// Q: Bắn projectile thẳng gây damage
void Player::skillQ(sf::Vector2f direction)
{
	float damage = baseDamage;

	// Nếu có target locked, bắn vào target
	if (hasLockedTarget)
	{
		direction = lockedTargetPos - shape.getPosition();
		std::cout << "Shooting locked target!" << std::endl;
	}

	projectiles.emplace_back(shape.getPosition(), direction, 600.0f, damage);
}

// W: Dịch chuyển đến projectile + gây damage
void Player::skillW()
{
	if (projectiles.empty()) return;

	// Di chuyển đến projectile đầu tiên
	sf::Vector2f projPos = projectiles[0].getPosition();
	shape.setPosition(projPos);
	lastProjectilePos = projPos;

	// Gây damage
	float damage = baseDamage * 1.5f;
	std::cout << "W damage dealt: " << damage << std::endl;

	// Xóa projectile
	projectiles[0].setAlive(false);
}

// E: Cường hóa - tăng damage + defense trong 5 giây
void Player::skillE()
{
	if (skillE_duration <= 0)  // Chỉ apply buff nếu chưa có
	{
		baseDamage *= 1.5f;
		defenseBoost = 10.0f;
		skillE_duration = 5.0f;
		std::cout << "E active! Damage: " << baseDamage << ", Defense: " << defenseBoost << std::endl;
	}
	else
	{
		// Nếu buff còn, reset lại thời gian
		skillE_duration = 5.0f;
		std::cout << "E buff refreshed!" << std::endl;
	}
}

// R: Gây damage liên tiếp 3 lần
void Player::skillR()
{
	float damage = baseDamage * 0.8f;
	for (int i = 0; i < 3; i++)
	{
		std::cout << "R hit " << (i + 1) << "/3 - Damage: " << damage << std::endl;
	}
	std::cout << "Total R damage: " << (damage * 3) << std::endl;
}