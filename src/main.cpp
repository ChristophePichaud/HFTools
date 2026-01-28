/*
 * HFTools - Cross-platform Database Library Demo Application
 * 
 * SECURITY NOTE: This is a demonstration application with mock database implementations.
 * For production use, consider the following security best practices:
 * - Never hardcode credentials in source code
 * - Use parameterized queries to prevent SQL injection
 * - Redact sensitive information (passwords) from logs
 * - Use secure credential management systems (environment variables, vaults)
 * - Implement proper input validation and sanitization
 * - Add comprehensive error handling for all operations
 */

#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <getopt.h>

#include "hftools/database/IDatabase.h"
#include "hftools/database/Connection.h"
#include "hftools/database/ResultSet.h"
#include "hftools/database/PostgreSQLDatabase.h"
#include "hftools/database/SybaseDatabase.h"
#include "hftools/model/User.h"
#include "hftools/model/FXInstrument.h"
#include "hftools/model/Trade.h"

using namespace hftools;
using namespace hftools::database;
using namespace hftools::model;

void printUsage(const char* progName) {
    std::cout << "Usage: " << progName << " [OPTIONS]\n"
              << "Options:\n"
              << "  -d, --database TYPE     Database type (postgresql or sybase)\n"
              << "  -c, --connection STR    Connection string\n"
              << "  -q, --query QUERY       Execute SQL query\n"
              << "  -j, --json FILE         Load JSON file and display POCO objects\n"
              << "  -t, --test              Run test demonstration\n"
              << "  -h, --help              Display this help message\n"
              << std::endl;
}

void demonstrateJsonSerialization() {
    std::cout << "\n=== JSON Serialization Demonstration ===\n" << std::endl;
    
    // Create sample objects
    User user(1, "trader1", "trader1@example.com", "TRADER");
    FXInstrument fx(1, "EUR/USD", "EUR", "USD", 0.0001);
    Trade trade(1, 1, 1, "BUY", 100000.0, 1.0850, "2024-01-28T10:30:00Z");
    
    // Convert to JSON
    std::cout << "User as JSON:\n" << user.toJson().dump(2) << "\n" << std::endl;
    std::cout << "FXInstrument as JSON:\n" << fx.toJson().dump(2) << "\n" << std::endl;
    std::cout << "Trade as JSON:\n" << trade.toJson().dump(2) << "\n" << std::endl;
    
    // Demonstrate deserialization
    nlohmann::json userJson = {
        {"id", 2},
        {"username", "admin1"},
        {"email", "admin1@example.com"},
        {"role", "ADMIN"}
    };
    User user2 = User::fromJson(userJson);
    std::cout << "User deserialized from JSON:\n"
              << "  ID: " << user2.getId() << "\n"
              << "  Username: " << user2.getUsername() << "\n"
              << "  Email: " << user2.getEmail() << "\n"
              << "  Role: " << user2.getRole() << std::endl;
}

void loadAndDisplayJson(const std::string& filename) {
    std::cout << "\n=== Loading JSON file: " << filename << " ===\n" << std::endl;
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }
    
    nlohmann::json j;
    try {
        file >> j;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Error: Failed to parse JSON file: " << e.what() << std::endl;
        file.close();
        return;
    }
    file.close();
    
    // Determine type based on filename
    try {
        if (filename.find("users") != std::string::npos) {
            std::cout << "Loading users:\n";
            for (const auto& userJson : j) {
                User user = User::fromJson(userJson);
                std::cout << "  - " << user.getUsername() << " (" << user.getEmail() << ") - " << user.getRole() << "\n";
            }
        } else if (filename.find("fxinstruments") != std::string::npos) {
            std::cout << "Loading FX Instruments:\n";
            for (const auto& fxJson : j) {
                FXInstrument fx = FXInstrument::fromJson(fxJson);
                std::cout << "  - " << fx.getSymbol() << " (" << fx.getBaseCurrency() 
                          << "/" << fx.getQuoteCurrency() << ") - Tick: " << fx.getTickSize() << "\n";
            }
        } else if (filename.find("trades") != std::string::npos) {
            std::cout << "Loading Trades:\n";
            for (const auto& tradeJson : j) {
                Trade trade = Trade::fromJson(tradeJson);
                std::cout << "  - Trade #" << trade.getId() << ": " << trade.getSide() 
                          << " " << trade.getQuantity() << " @ " << trade.getPrice() 
                          << " (" << trade.getTimestamp() << ")\n";
            }
        }
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Error: Failed to parse object from JSON: " << e.what() << std::endl;
        return;
    }
    
    std::cout << std::endl;
}

void testDatabaseConnection(const std::string& dbType, const std::string& connStr) {
    std::cout << "\n=== Testing Database Connection ===\n" << std::endl;
    
    std::shared_ptr<IDatabase> db;
    
    if (dbType == "postgresql") {
        db = std::make_shared<PostgreSQLDatabase>();
    } else if (dbType == "sybase") {
        db = std::make_shared<SybaseDatabase>();
    } else {
        std::cerr << "Error: Unknown database type: " << dbType << std::endl;
        return;
    }
    
    std::cout << "Database type: " << db->getDatabaseType() << std::endl;
    
    // Open connection
    auto conn = db->openConnection(connStr);
    
    if (conn && conn->isConnected()) {
        std::cout << "Connection established successfully!\n" << std::endl;
        
        // Execute some sample queries
        std::cout << "Querying users table..." << std::endl;
        auto rs = conn->execQuery("SELECT * FROM users");
        
        if (rs) {
            std::cout << "Query returned " << rs->getRowCount() << " rows" << std::endl;
            std::cout << "Columns: ";
            for (const auto& col : rs->getColumnNames()) {
                std::cout << col << " ";
            }
            std::cout << "\n";
            
            while (rs->next()) {
                std::cout << "  User: " << rs->getField("username") 
                          << " (" << rs->getField("email") << ") - " 
                          << rs->getField("role") << std::endl;
            }
        }
        
        std::cout << "\nQuerying fxinstruments table..." << std::endl;
        rs = conn->execQuery("SELECT * FROM fxinstruments");
        
        if (rs) {
            std::cout << "Query returned " << rs->getRowCount() << " rows\n";
            while (rs->next()) {
                std::cout << "  Instrument: " << rs->getField("symbol") << std::endl;
            }
        }
        
        std::cout << "\nQuerying trades table..." << std::endl;
        rs = conn->execQuery("SELECT * FROM trades");
        
        if (rs) {
            std::cout << "Query returned " << rs->getRowCount() << " rows\n";
            while (rs->next()) {
                std::cout << "  Trade: " << rs->getField("side") 
                          << " " << rs->getField("quantity") 
                          << " @ " << rs->getField("price") << std::endl;
            }
        }
        
        conn->close();
    } else {
        std::cerr << "Failed to connect to database!" << std::endl;
    }
}

void runTestDemonstration() {
    std::cout << "\n======================================" << std::endl;
    std::cout << "HFTools - Financial System Demo" << std::endl;
    std::cout << "======================================\n" << std::endl;
    
    // Test JSON serialization
    demonstrateJsonSerialization();
    
    // Test PostgreSQL
    testDatabaseConnection("postgresql", "host=localhost port=5432 dbname=hftools_db user=postgres password=pass");
    
    // Test Sybase
    testDatabaseConnection("sybase", "server=localhost;database=hftools_db;user=sa;password=pass");
    
    // Load JSON files
    std::cout << "\n=== Loading JSON Data Files ===\n" << std::endl;
    loadAndDisplayJson("data/users.json");
    loadAndDisplayJson("data/fxinstruments.json");
    loadAndDisplayJson("data/trades.json");
}

int main(int argc, char* argv[]) {
    std::string dbType;
    std::string connStr;
    std::string query;
    std::string jsonFile;
    bool runTest = false;
    
    // Define long options
    static struct option long_options[] = {
        {"database",   required_argument, 0, 'd'},
        {"connection", required_argument, 0, 'c'},
        {"query",      required_argument, 0, 'q'},
        {"json",       required_argument, 0, 'j'},
        {"test",       no_argument,       0, 't'},
        {"help",       no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    // Parse command line arguments
    while ((opt = getopt_long(argc, argv, "d:c:q:j:th", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'd':
                dbType = optarg;
                break;
            case 'c':
                connStr = optarg;
                break;
            case 'q':
                query = optarg;
                break;
            case 'j':
                jsonFile = optarg;
                break;
            case 't':
                runTest = true;
                break;
            case 'h':
                printUsage(argv[0]);
                return 0;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }
    
    // If no arguments provided, show help
    if (argc == 1) {
        printUsage(argv[0]);
        std::cout << "\nTip: Run with --test to see a demonstration of all features.\n" << std::endl;
        return 0;
    }
    
    // Run test demonstration
    if (runTest) {
        runTestDemonstration();
        return 0;
    }
    
    // Handle JSON file loading
    if (!jsonFile.empty()) {
        loadAndDisplayJson(jsonFile);
        return 0;
    }
    
    // Handle database operations
    if (!dbType.empty() && !connStr.empty()) {
        if (!query.empty()) {
            // Execute custom query
            std::shared_ptr<IDatabase> db;
            
            if (dbType == "postgresql") {
                db = std::make_shared<PostgreSQLDatabase>();
            } else if (dbType == "sybase") {
                db = std::make_shared<SybaseDatabase>();
            } else {
                std::cerr << "Error: Unknown database type: " << dbType << std::endl;
                return 1;
            }
            
            auto conn = db->openConnection(connStr);
            if (conn && conn->isConnected()) {
                auto rs = conn->execQuery(query);
                if (rs) {
                    std::cout << "Query returned " << rs->getRowCount() << " rows" << std::endl;
                    while (rs->next()) {
                        for (const auto& col : rs->getColumnNames()) {
                            std::cout << col << ": " << rs->getField(col) << " ";
                        }
                        std::cout << std::endl;
                    }
                }
                conn->close();
            }
        } else {
            // Just test connection
            testDatabaseConnection(dbType, connStr);
        }
        return 0;
    }
    
    std::cerr << "Error: Invalid arguments. Use --help for usage information." << std::endl;
    return 1;
}
