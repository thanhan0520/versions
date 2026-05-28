#pragma execution_character_set("utf-8")
#include "AuthManager.h"
#include <iostream>

AuthManager::AuthManager() : currentUser(""), hEnv(NULL), hDbc(NULL), isConnected(false)
{
    // 1. Cấp phát môi trường kết nối ODBC
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) == SQL_SUCCESS) {
        SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

        // 2. Cấp phát Handle kết nối
        if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) == SQL_SUCCESS) {

            // 3. Chuỗi kết nối cấu hình chính xác theo SQL Server Express của bạn
            std::string connStr = "Driver={ODBC Driver 17 for SQL Server};Server=localhost;Database=Game_db;Trusted_Connection=yes;";

            SQLCHAR outConnStr[1024];
            SQLSMALLINT outConnStrLen;

            // Thực hiện kết nối hệ thống
            SQLRETURN ret = SQLDriverConnectA(hDbc, NULL, (SQLCHAR*)connStr.c_str(), SQL_NTS,
                outConnStr, sizeof(outConnStr), &outConnStrLen, SQL_DRIVER_NOPROMPT);

            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                std::cout << "==> KET NOI DEN MICROSOFT SQL SERVER THANH CONG! <==" << std::endl;
                isConnected = true;
            }
            else {
                std::cerr << "Loi: Khong the ket noi den SQL Server" << std::endl;
            }
        }
    }
}

AuthManager::~AuthManager()
{
    disconnect();
}

void AuthManager::disconnect()
{
    if (isConnected) {
        if (hDbc) { SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); }
        if (hEnv) { SQLFreeHandle(SQL_HANDLE_ENV, hEnv); }
        isConnected = false;
    }
}

bool AuthManager::registerAccount(const std::string& username, const std::string& password, const std::string& email)
{
    if (!isConnected || username.empty() || password.empty() || email.empty()) return false;

    std::string hashed = hashPassword(password);
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    // Câu lệnh INSERT dữ liệu đăng ký vào SQL Server
    std::string query = "INSERT INTO Users (username, password, email) VALUES ('" + username + "', '" + hashed + "', '" + email + "');";

    SQLRETURN ret = SQLExecDirectA(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        std::cout << "Dang ky tai khoan " << username << " thanh cong!" << std::endl;
        return true;
    }
    std::cout << "Dang ky that bai! Ten tai khoan co the da co nguoi su dung." << std::endl;
    return false;
}

bool AuthManager::login(const std::string& username, const std::string& password)
{
    if (!isConnected) return false;

    std::string hashed = hashPassword(password);
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    std::string query = "SELECT password FROM Users WHERE username = '" + username + "';";
    SQLExecDirectA(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);

    bool success = false;
    if (SQLFetch(hStmt) == SQL_SUCCESS) {
        char dbPassword[255];
        SQLLEN cbPassword;
        SQLGetData(hStmt, 1, SQL_C_CHAR, dbPassword, sizeof(dbPassword), &cbPassword);

        if (hashed == std::string(dbPassword)) {
            currentUser = username;
            success = true;
            std::cout << "Dang nhap thanh cong! Chao mung " << username << std::endl;
        }
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    if (!success) std::cout << "Thong tin dang nhap khong chinh xac!" << std::endl;
    return success;
}

bool AuthManager::saveGameProgress(int stage, int hp, int score, int characterClass)
{
    if (!isConnected || currentUser.empty()) return false;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    // Đã cập nhật: Bổ sung lưu thêm thông tin hệ phái nhân vật (character_class)
    std::string query =
        "IF EXISTS (SELECT 1 FROM PlayerProgress WHERE username = '" + currentUser + "') "
        "UPDATE PlayerProgress SET current_stage = " + std::to_string(stage) +
        ", player_hp = " + std::to_string(hp) +
        ", score = " + std::to_string(score) +
        ", character_class = " + std::to_string(characterClass) +
        " WHERE username = '" + currentUser + "'; "
        "ELSE "
        "INSERT INTO PlayerProgress (username, current_stage, player_hp, score, character_class) "
        "VALUES ('" + currentUser + "', " + std::to_string(stage) + ", " + std::to_string(hp) + ", " + std::to_string(score) + ", " + std::to_string(characterClass) + ");";

    SQLRETURN ret = SQLExecDirectA(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

bool AuthManager::loadGameProgress(int& stage, int& hp, int& score, int& characterClass)
{
    if (!isConnected || currentUser.empty()) return false;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    // Đã cập nhật: SELECT thêm cột thứ 4 là character_class
    std::string query = "SELECT current_stage, player_hp, score, character_class FROM PlayerProgress WHERE username = '" + currentUser + "';";
    SQLExecDirectA(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);

    bool found = false;
    if (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLLEN cbStage, cbHp, cbScore, cbClass;
        SQLGetData(hStmt, 1, SQL_C_LONG, &stage, 0, &cbStage);
        SQLGetData(hStmt, 2, SQL_C_LONG, &hp, 0, &cbHp);
        SQLGetData(hStmt, 3, SQL_C_LONG, &score, 0, &cbScore);

        // Đã cập nhật: Lấy dữ liệu cột số 4 đổ vào biến characterClass
        SQLGetData(hStmt, 4, SQL_C_LONG, &characterClass, 0, &cbClass);
        found = true;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return found;
}

std::string AuthManager::getCurrentUser() const { return currentUser; }
std::string AuthManager::hashPassword(const std::string& password) const { return "hash_" + password; }