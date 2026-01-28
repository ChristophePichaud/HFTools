#pragma once

#include <string>
#include <memory>

namespace hftools {
namespace database {

// Forward declarations
class Connection;
class ResultSet;

/**
 * @brief Generic database interface providing abstraction for different database systems
 */
class IDatabase {
public:
    virtual ~IDatabase() = default;

    /**
     * @brief Open a connection to the database
     * @param connectionString Connection string with host, port, database, user, password
     * @return Shared pointer to Connection object
     */
    virtual std::shared_ptr<Connection> openConnection(const std::string& connectionString) = 0;

    /**
     * @brief Get the database type name
     * @return String representing the database type (e.g., "PostgreSQL", "Sybase")
     */
    virtual std::string getDatabaseType() const = 0;
};

} // namespace database
} // namespace hftools
