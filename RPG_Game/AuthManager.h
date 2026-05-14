#pragma once
#include <string>
#include <map>
#include <fstream>
#include <sstream>

class AuthManager
{
public:
    AuthManager();
    ~AuthManager();

    // Đăng ký tài khoản mới
    bool registerAccount(const std::string& username, const std::string& password);

    // Đăng nhập với tài khoản
    bool login(const std::string& username, const std::string& password);

    // Lấy tên người dùng hiện tại
    std::string getCurrentUser() const;

    // Kiểm tra tài khoản có tồn tại không
    bool accountExists(const std::string& username) const;

private:
    void loadAccounts();
    void saveAccounts();
    std::string hashPassword(const std::string& password) const;

    std::map<std::string, std::string> accounts;  // username -> password
    std::string currentUser;
    std::string accountsFilePath;
};
