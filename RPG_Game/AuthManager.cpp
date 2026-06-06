#pragma execution_character_set("utf-8")
#include "AuthManager.h"
#include <iostream>
#include <sstream>
#include <algorithm>

// Simple helper to escape single quotes for SQL string literals
static std::string escapeForSQL(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c == '\'') out.push_back('\'');
        out.push_back(c);
    }
    return out;
}

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
                // Ensure PlayerProgress table and necessary columns exist
                SQLHSTMT hStmtSchema;
                if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmtSchema) == SQL_SUCCESS) {
                    // Create table if not exists
                    std::string createTable =
                        "IF NOT EXISTS (SELECT * FROM sys.tables WHERE name = 'PlayerProgress') BEGIN "
                        "CREATE TABLE dbo.PlayerProgress (username NVARCHAR(255) PRIMARY KEY, current_stage INT, player_hp INT, player_x FLOAT, player_y FLOAT, score INT, character_class INT, monster_count INT, full_state NVARCHAR(MAX)); END;";
                    SQLExecDirectA(hStmtSchema, (SQLCHAR*)createTable.c_str(), SQL_NTS);

                    // Add missing columns if they don't exist (safe to run repeatedly)
                    std::string addCols[] = {
                        "IF NOT EXISTS (SELECT * FROM sys.columns WHERE Name = N'player_x' AND Object_ID = Object_ID(N'dbo.PlayerProgress')) BEGIN ALTER TABLE dbo.PlayerProgress ADD player_x FLOAT NULL; END;",
                        "IF NOT EXISTS (SELECT * FROM sys.columns WHERE Name = N'player_y' AND Object_ID = Object_ID(N'dbo.PlayerProgress')) BEGIN ALTER TABLE dbo.PlayerProgress ADD player_y FLOAT NULL; END;",
                        "IF NOT EXISTS (SELECT * FROM sys.columns WHERE Name = N'monster_count' AND Object_ID = Object_ID(N'dbo.PlayerProgress')) BEGIN ALTER TABLE dbo.PlayerProgress ADD monster_count INT NULL; END;",
                        "IF NOT EXISTS (SELECT * FROM sys.columns WHERE Name = N'full_state' AND Object_ID = Object_ID(N'dbo.PlayerProgress')) BEGIN ALTER TABLE dbo.PlayerProgress ADD full_state NVARCHAR(MAX) NULL; END;"
                    };

                    for (auto &q : addCols) {
                        SQLExecDirectA(hStmtSchema, (SQLCHAR*)q.c_str(), SQL_NTS);
                    }

                    SQLFreeHandle(SQL_HANDLE_STMT, hStmtSchema);
                }
            } else {
                std::cerr << "Loi: Khong the ket noi den SQL Server" << std::endl;
            }
        }
    }
}

bool AuthManager::saveFullGameState(const std::string& serializedState)
{
    if (!isConnected || currentUser.empty()) return false;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    // Sử dụng cột full_state để lưu JSON / chuỗi tuần tự hóa
    std::string escUser = escapeForSQL(currentUser);
    std::string escState = escapeForSQL(serializedState);
    std::string query =
        "IF EXISTS (SELECT 1 FROM PlayerProgress WHERE username = '" + escUser + "') "
        "UPDATE PlayerProgress SET full_state = '" + escState + "' WHERE username = '" + escUser + "'; "
        "ELSE "
        "INSERT INTO PlayerProgress (username, current_stage, player_hp, score, character_class, full_state) "
        "VALUES ('" + escUser + "', 1, 100, 0, 0, '" + escState + "');";

    SQLRETURN ret = SQLExecDirectA(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

    bool ok = (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
    if (ok) {
        std::cout << "AuthManager: Luu trang thai luu tru thanh cong cho user='" << currentUser << "', len=" << serializedState.size() << std::endl;
    }
    else {
        std::cerr << "AuthManager: Luu trang thai that bai cho user='" << currentUser << "'" << std::endl;
    }
    return ok;
}

bool AuthManager::loadFullGameState(std::string& outSerializedState)
{
    if (!isConnected || currentUser.empty()) return false;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    std::string escUser = escapeForSQL(currentUser);
    std::string query = "SELECT full_state FROM PlayerProgress WHERE username = '" + escUser + "';";
    SQLExecDirectA(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);

    bool found = false;
    if (SQLFetch(hStmt) == SQL_SUCCESS) {
        char buf[4096];
        SQLLEN cb;
        SQLGetData(hStmt, 1, SQL_C_CHAR, buf, sizeof(buf), &cb);
        if (cb == SQL_NULL_DATA) {
            // full_state is NULL, we will try fallback to individual columns
            outSerializedState.clear();
            found = true; // still found a row
        }
        else {
            outSerializedState = std::string(buf);
            found = true;
        }
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

    if (found && outSerializedState.empty()) {
        // Fallback: try to read player_x, player_y, player_hp, monster_count
        SQLHSTMT fStmt;
        if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &fStmt) == SQL_SUCCESS) {
            std::string q2 = "SELECT player_x, player_y, player_hp, monster_count FROM PlayerProgress WHERE username = '" + escUser + "';";
            SQLExecDirectA(fStmt, (SQLCHAR*)q2.c_str(), SQL_NTS);
            if (SQLFetch(fStmt) == SQL_SUCCESS) {
                double px = 512.0, py = 384.0;
                SQLLEN cb1, cb2, cb3, cb4;
                SQLGetData(fStmt, 1, SQL_C_DOUBLE, &px, 0, &cb1);
                SQLGetData(fStmt, 2, SQL_C_DOUBLE, &py, 0, &cb2);
                int php = 100;
                int mcount = 0;
                SQLGetData(fStmt, 3, SQL_C_LONG, &php, 0, &cb3);
                SQLGetData(fStmt, 4, SQL_C_LONG, &mcount, 0, &cb4);

                // Build a simple serialized format: playerX,playerY,playerHP;monsterCount;[placeholder monsters]
                std::ostringstream ss;
                ss << px << "," << py << "," << php << ";" << mcount;

                // If there are monsters but no detailed data, create placeholder monsters around player
                for (int i = 0; i < mcount; ++i) {
                    // simple offset
                    double ox = ((i % 5) - 2) * 50.0 + ((i % 3) * 10);
                    double oy = ((i / 5) - 2) * 50.0 + ((i % 2) * 7);
                    double mx = px + ox;
                    double my = py + oy;
                    double mhp = 30.0;
                    ss << ";" << mx << "," << my << "," << mhp;
                }

                outSerializedState = ss.str();
            }
            SQLFreeHandle(SQL_HANDLE_STMT, fStmt);
        }
    }

    if (found) {
        std::string preview = outSerializedState.substr(0, std::min<size_t>(outSerializedState.size(), 200));
        std::cout << "AuthManager: Tai trang thai thanh cong cho user='" << currentUser << "', len=" << outSerializedState.size() << " preview='" << preview << "'" << std::endl;
    }
    else {
        std::cout << "AuthManager: Khong tim thay trang thai cho user='" << currentUser << "'" << std::endl;
    }

    return found;
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

bool AuthManager::saveGameProgress(int stage, int hp, int score, int characterClass, float playerX, float playerY, int monsterCount)
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
        ", player_x = " + std::to_string(playerX) +
        ", player_y = " + std::to_string(playerY) +
        ", monster_count = " + std::to_string(monsterCount) +
        " WHERE username = '" + currentUser + "'; "
        "ELSE "
        "INSERT INTO PlayerProgress (username, current_stage, player_hp, player_x, player_y, score, character_class, monster_count) "
        "VALUES ('" + currentUser + "', " + std::to_string(stage) + ", " + std::to_string(hp) + ", " + std::to_string(playerX) + ", " + std::to_string(playerY) + ", " + std::to_string(score) + ", " + std::to_string(characterClass) + ", " + std::to_string(monsterCount) + ");";

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