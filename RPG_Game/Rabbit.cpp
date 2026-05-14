#include<iostream>
#include "Rabbit.h"

Rabbit::Rabbit() : Player() // Gọi Constructor của Player để khởi tạo bộ khung
{
    // 1. Chỉnh sửa chỉ số đặc trưng của Thỏ
    speed = 400.0f;           // Thỏ chạy nhanh hơn (Player gốc là 300)
    maxHealth = 80.0f;        // Máu ít hơn một chút để cân bằng
    health = maxHealth;
    baseDamage = 25.0f;       // Sát thương cơ bản cao hơn

    // 2. Nạp hình ảnh cho Thỏ (Thay vì hình vuông đỏ)
    // Giả sử bạn có file "rabbit.png"
    /*
    if (texture.loadFromFile("assets/rabbit.png")) {
        sprite.setTexture(texture);
        // Ẩn shape hình vuông đi nếu bạn dùng Sprite
    }
    */

    // 3. Nạp hình ảnh cho đạn của Thỏ (Ví dụ: Mũi tên)
    // bulletTexture là biến chúng ta đã thêm vào Player.h ở các bước trước
    if (!bulletTexture.loadFromFile("arrow.png")) {
        // Nếu chưa có ảnh, tạo đạn màu Xanh Lá cho Thỏ dễ phân biệt
        sf::Image img;
        img.create(15, 5, sf::Color::Green);
        bulletTexture.loadFromImage(img);
    }

    std::cout << "Rabbit character initialized!" << std::endl;
}