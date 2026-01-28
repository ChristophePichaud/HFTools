#include "hftools/database/Connection.h"
#include "hftools/database/ResultSet.h"
#include <iostream>

namespace hftools {
namespace database {

Connection::Connection(const std::string& dbType, const std::string& connectionString)
    : dbType_(dbType), connectionString_(connectionString), connected_(false) {
}

Connection::~Connection() {
    if (connected_) {
        close();
    }
}

std::shared_ptr<ResultSet> Connection::execQuery(const std::string& query) {
    std::cout << "[" << dbType_ << "] Executing query: " << query << std::endl;
    
    // Mock implementation - return empty result set
    auto rs = std::make_shared<ResultSet>();
    return rs;
}

int Connection::execCommand(const std::string& command) {
    std::cout << "[" << dbType_ << "] Executing command: " << command << std::endl;
    
    // Mock implementation - return 1 row affected
    return 1;
}

bool Connection::isConnected() const {
    return connected_;
}

void Connection::close() {
    if (connected_) {
        std::cout << "[" << dbType_ << "] Closing connection" << std::endl;
        connected_ = false;
    }
}

} // namespace database
} // namespace hftools
