#pragma once

#include "IDatabase.h"
#include "Connection.h"
#include <memory>
#include <string>

namespace hftools {
namespace database {

/**
 * @brief Sybase database implementation
 */
class SybaseDatabase : public IDatabase {
public:
    SybaseDatabase() = default;
    virtual ~SybaseDatabase() = default;

    /**
     * @brief Open a connection to Sybase
     * @param connectionString Format: "server=myserver;database=mydb;user=sa;password=pass"
     * @return Shared pointer to Connection object
     */
    std::shared_ptr<Connection> openConnection(const std::string& connectionString) override;

    /**
     * @brief Get database type
     * @return "Sybase"
     */
    std::string getDatabaseType() const override;
};

/**
 * @brief Sybase-specific connection
 */
class SybaseConnection : public Connection {
public:
    SybaseConnection(const std::string& connectionString);
    virtual ~SybaseConnection();

    std::shared_ptr<ResultSet> execQuery(const std::string& query) override;
    int execCommand(const std::string& command) override;
    bool isConnected() const override;
    void close() override;

private:
    // In a real implementation, this would hold Sybase connection handle
    void* sybaseConn_; // DBPROCESS* in real implementation
};

} // namespace database
} // namespace hftools
