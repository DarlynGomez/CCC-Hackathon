#include "../include/crow.h"
#include <sqlite3.h>
#include <string>
#include <iostream>
#include <filesystem>

// Get the absolute path to the database
std::string getDatabasePath() {
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path dbPath = currentPath.parent_path() / "database" / "users.db";
    std::cout << "Database path: " << dbPath.string() << std::endl;
    return dbPath.string();
}

void setCorsHeaders(crow::response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

bool setupDatabase() {
    sqlite3* db;
    char* errMsg = 0;
    
    std::string dbPath = getDatabasePath();
    std::cout << "Attempting to open database at: " << dbPath << std::endl;
    
    // Create database directory if it doesn't exist
    std::filesystem::path dbDir = std::filesystem::path(dbPath).parent_path();
    std::filesystem::create_directories(dbDir);
    
    int rc = sqlite3_open(dbPath.c_str(), &db);  // Using c_str() here
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    std::cout << "Database opened successfully" << std::endl;
    
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "email TEXT NOT NULL UNIQUE,"
        "password TEXT NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP);";
    
    rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }
    
    std::cout << "Table created/verified successfully" << std::endl;
    sqlite3_close(db);
    return true;
}

int main() {
    crow::SimpleApp app;

    if (!setupDatabase()) {
        std::cerr << "Failed to setup database. Exiting..." << std::endl;
        return 1;
    }

    CROW_ROUTE(app, "/login")
    .methods("OPTIONS"_method)
    ([](const crow::request& req) {
        crow::response res;
        setCorsHeaders(res);
        res.code = 204;
        return res;
    });

    CROW_ROUTE(app, "/register")
    .methods("OPTIONS"_method)
    ([](const crow::request& req) {
        crow::response res;
        setCorsHeaders(res);
        res.code = 204;
        return res;
    });

    CROW_ROUTE(app, "/")
    ([](const crow::request& req) {
        crow::response res("BMCC Jobs Portal API Running");
        setCorsHeaders(res);
        return res;
    });

    CROW_ROUTE(app, "/register")
    .methods("POST"_method)
    ([](const crow::request& req) {
        crow::response res;
        setCorsHeaders(res);

        std::cout << "Received registration request" << std::endl;

        auto json = crow::json::load(req.body);
        if (!json) {
            std::cout << "Invalid JSON received" << std::endl;
            res.code = 400;
            res.write("{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
            return res;
        }

        std::string email = json["email"].s();
        std::string password = json["password"].s();

        std::cout << "Registration attempt for email: " << email << std::endl;

        if (email.find("@stu.bmcc.cuny.edu") == std::string::npos) {
            res.code = 400;
            res.write("{\"status\":\"error\",\"message\":\"Please use your BMCC student email\"}");
            return res;
        }

        sqlite3* db;
        std::string dbPath = getDatabasePath();
        int rc = sqlite3_open(dbPath.c_str(), &db);  // Using c_str() here
        if (rc) {
            std::cerr << "Can't open database during registration: " << sqlite3_errmsg(db) << std::endl;
            res.code = 500;
            res.write("{\"status\":\"error\",\"message\":\"Database error\"}");
            return res;
        }

        // Check if user exists
        std::string checkQuery = "SELECT email FROM users WHERE email = ?";
        sqlite3_stmt* checkStmt;
        rc = sqlite3_prepare_v2(db, checkQuery.c_str(), -1, &checkStmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare check statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            res.code = 500;
            res.write("{\"status\":\"error\",\"message\":\"Database error\"}");
            return res;
        }

        sqlite3_bind_text(checkStmt, 1, email.c_str(), -1, SQLITE_STATIC);
        
        if (sqlite3_step(checkStmt) == SQLITE_ROW) {
            std::cout << "Email already exists in database" << std::endl;
            sqlite3_finalize(checkStmt);
            sqlite3_close(db);
            res.code = 400;
            res.write("{\"status\":\"error\",\"message\":\"Email already registered\"}");
            return res;
        }
        sqlite3_finalize(checkStmt);

        // Insert new user
        std::string insertQuery = "INSERT INTO users (email, password) VALUES (?, ?)";
        sqlite3_stmt* insertStmt;
        rc = sqlite3_prepare_v2(db, insertQuery.c_str(), -1, &insertStmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare insert statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            res.code = 500;
            res.write("{\"status\":\"error\",\"message\":\"Database error\"}");
            return res;
        }

        sqlite3_bind_text(insertStmt, 1, email.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(insertStmt, 2, password.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(insertStmt) != SQLITE_DONE) {
            std::cerr << "Failed to insert user: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(insertStmt);
            sqlite3_close(db);
            res.code = 500;
            res.write("{\"status\":\"error\",\"message\":\"Registration failed\"}");
            return res;
        }

        std::cout << "User registered successfully" << std::endl;
        sqlite3_finalize(insertStmt);
        sqlite3_close(db);

        res.code = 201;
        res.write("{\"status\":\"success\",\"message\":\"Registration successful\"}");
        return res;
    });

    // Login route
    CROW_ROUTE(app, "/login")
    .methods("POST"_method)
    ([](const crow::request& req) {
        crow::response res;
        setCorsHeaders(res);

        auto json = crow::json::load(req.body);
        if (!json) {
            res.code = 400;
            res.write("{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
            return res;
        }

        std::string email = json["email"].s();
        std::string password = json["password"].s();

        sqlite3* db;
        std::string dbPath = getDatabasePath();
        int rc = sqlite3_open(dbPath.c_str(), &db);  // Using c_str() here
        if (rc) {
            res.code = 500;
            res.write("{\"status\":\"error\",\"message\":\"Database error\"}");
            return res;
        }
        
        std::string query = "SELECT * FROM users WHERE email = ? AND password = ?";
        sqlite3_stmt* stmt;
        rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            sqlite3_close(db);
            res.code = 500;
            res.write("{\"status\":\"error\",\"message\":\"Database error\"}");
            return res;
        }

        sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

        bool userFound = false;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            userFound = true;
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);

        if (userFound) {
            res.code = 200;
            res.write("{\"status\":\"success\",\"message\":\"Login successful\"}");
        } else {
            res.code = 401;
            res.write("{\"status\":\"error\",\"message\":\"Invalid credentials\"}");
        }
        return res;
    });

    std::cout << "Server starting on http://localhost:8080" << std::endl;
    app.port(8080).multithreaded().run();
    return 0;
}