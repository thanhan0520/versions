#pragma execution_character_set("utf-8")
#include "AuthManager.h"
#include <iostream>

AuthManager::AuthManager() : currentUser(""), accountsFilePath("accounts.txt")
{
    loadAccounts();
}

AuthManager::~AuthManager()
{
    saveAccounts();
}

bool AuthManager::registerAccount(const std::string& username, const std::string& password)
{
    // Kiểm tra tài khoản đã tồn tại
    if (accounts.find(username) != accounts.end())
    {
        std::cout << "Tài khoản '" << username << "' đã tồn tại!" << std::endl;
        return false;
    }

    // Kiểm tra username/password không để trống
    if (username.empty() || password.empty())
    {
        std::cout << "Tên đăng nhập và mật khẩu không được để trống!" << std::endl;
        return false;
    }

    // Thêm tài khoản mới
    accounts[username] = hashPassword(password);
    saveAccounts();
    std::cout << "Đăng ký tài khoản '" << username << "' thành công!" << std::endl;
    return true;
}

bool AuthManager::login(const std::string& username, const std::string& password)
{
    // Kiểm tra tài khoản tồn tại
    if (accounts.find(username) == accounts.end())
    {
        std::cout << "Tài khoản '" << username << "' không tồn tại!" << std::endl;
        return false;
    }

    // Kiểm tra mật khẩu
    if (accounts[username] != hashPassword(password))
    {
        std::cout << "Mật khẩu không chính xác!" << std::endl;
        return false;
    }

    currentUser = username;
    std::cout << "Đăng nhập thành công! Chào mừng " << username << std::endl;
    return true;
}

std::string AuthManager::getCurrentUser() const
{
    return currentUser;
}

bool AuthManager::accountExists(const std::string& username) const
{
    return accounts.find(username) != accounts.end();
}

void AuthManager::loadAccounts()
{
    std::ifstream file(accountsFilePath);
    if (!file.is_open())
    {
        std::cout << "Không thể mở file accounts.txt. Tạo file mới." << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        // Format: username:password
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos)
        {
            std::string username = line.substr(0, colonPos);
            std::string password = line.substr(colonPos + 1);
            accounts[username] = password;
        }
    }

    file.close();
    std::cout << "Đã tải " << accounts.size() << " tài khoản từ file." << std::endl;
}

void AuthManager::saveAccounts()
{
    std::ofstream file(accountsFilePath);
    if (!file.is_open())
    {
        std::cout << "Lỗi: Không thể lưu file accounts.txt!" << std::endl;
        return;
    }

    for (const auto& pair : accounts)
    {
        file << pair.first << ":" << pair.second << "\n";
    }

    file.close();
}

std::string AuthManager::hashPassword(const std::string& password) const
{
    // Đây là hashing đơn giản (chỉ thêm tiền tố)
    // Trong ứng dụng thực tế, nên dùng bcrypt hoặc SHA-256
    return "hash_" + password;
}