#pragma once

#include <string>
#include <memory>

namespace hftools {
namespace database {

// Forward declaration
class ResultSet;

/**
 * @brief Represents a database connection
 */
class Connection {
public:
    Connection(const std::string& dbType, const std::string& connectionString);
    virtual ~Connection();

    /**
     * @brief Execute a SQL query and return results
     * @param query SQL query string
     * @return Shared pointer to ResultSet
     */
    virtual std::shared_ptr<ResultSet> execQuery(const std::string& query);

    /**
     * @brief Execute a SQL command (INSERT, UPDATE, DELETE)
     * @param command SQL command string
     * @return Number of rows affected
     */
    virtual int execCommand(const std::string& command);

    /**
     * @brief Check if connection is open
     * @return true if connected, false otherwise
     */
    virtual bool isConnected() const;

    /**
     * @brief Close the connection
     */
    virtual void close();

    /**
     * @brief Get the database type
     */
    std::string getDatabaseType() const { return dbType_; }

    /**
     * @brief Get the connection string
     */
    std::string getConnectionString() const { return connectionString_; }

protected:
    std::string dbType_;
    std::string connectionString_;
    bool connected_;
};

} // namespace database
} // namespace hftools
