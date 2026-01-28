#pragma once

#include "IDatabase.h"
#include "Connection.h"
#include <memory>
#include <string>

namespace hftools {
namespace database {

/**
 * @brief PostgreSQL database implementation
 */
class PostgreSQLDatabase : public IDatabase {
public:
    PostgreSQLDatabase() = default;
    virtual ~PostgreSQLDatabase() = default;

    /**
     * @brief Open a connection to PostgreSQL
     * @param connectionString Format: "host=localhost port=5432 dbname=mydb user=postgres password=pass"
     * @return Shared pointer to Connection object
     */
    std::shared_ptr<Connection> openConnection(const std::string& connectionString) override;

    /**
     * @brief Get database type
     * @return "PostgreSQL"
     */
    std::string getDatabaseType() const override;
};

/**
 * @brief PostgreSQL-specific connection
 */
class PostgreSQLConnection : public Connection {
public:
    PostgreSQLConnection(const std::string& connectionString);
    virtual ~PostgreSQLConnection();

    std::shared_ptr<ResultSet> execQuery(const std::string& query) override;
    int execCommand(const std::string& command) override;
    bool isConnected() const override;
    void close() override;

private:
    // In a real implementation, this would hold libpq connection handle
    void* pgConn_; // PGconn* in real implementation
};

} // namespace database
} // namespace hftools
