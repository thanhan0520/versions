#pragma once
#include <string>
#include <windows.h>
#include <sqlext.h> // Thư viện kết nối SQL Server của Microsoft

class AuthManager
{
public:
    AuthManager();
    ~AuthManager();

    // Đăng ký tài khoản (Yêu cầu điền thêm Email)
    bool registerAccount(const std::string& username, const std::string& password, const std::string& email);

    // Đăng nhập
    bool login(const std::string& username, const std::string& password);

    std::string getCurrentUser() const;

    // --- CHỨC NĂNG LƯU/TẢI TIẾN TRÌNH OFFLINE XUỐNG SQL SERVER ---
    // Đã cập nhật: Thêm tham số int characterClass ở cuối để lưu loài vật
    bool saveGameProgress(int stage, int hp, int score, int characterClass);

    // Đã cập nhật: Thêm tham số int& characterClass ở cuối để tải loài vật lên
    bool loadGameProgress(int& stage, int& hp, int& score, int& characterClass);

private:
    std::string currentUser;
    std::string hashPassword(const std::string& password) const;

    // Các biến quản lý kết nối kết nối ODBC SQL Server
    SQLHENV hEnv;
    SQLHDBC hDbc;
    bool isConnected;

    void disconnect();
};