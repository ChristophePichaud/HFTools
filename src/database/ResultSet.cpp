#include "hftools/database/ResultSet.h"
#include <stdexcept>

namespace hftools {
namespace database {

ResultSet::ResultSet() : currentRow_(-1) {
}

ResultSet::~ResultSet() {
}

bool ResultSet::next() {
    currentRow_++;
    return currentRow_ < static_cast<int>(rows_.size());
}

std::string ResultSet::getField(const std::string& columnName) const {
    if (currentRow_ < 0 || currentRow_ >= static_cast<int>(rows_.size())) {
        throw std::runtime_error("No current row");
    }
    
    const auto& row = rows_[currentRow_];
    auto it = row.find(columnName);
    if (it == row.end()) {
        throw std::runtime_error("Column not found: " + columnName);
    }
    
    return it->second;
}

int ResultSet::getInt(const std::string& columnName) const {
    std::string value = getField(columnName);
    return std::stoi(value);
}

double ResultSet::getDouble(const std::string& columnName) const {
    std::string value = getField(columnName);
    return std::stod(value);
}

bool ResultSet::isNull(const std::string& columnName) const {
    if (currentRow_ < 0 || currentRow_ >= static_cast<int>(rows_.size())) {
        return true;
    }
    
    const auto& row = rows_[currentRow_];
    auto it = row.find(columnName);
    return it == row.end() || it->second.empty();
}

int ResultSet::getRowCount() const {
    return static_cast<int>(rows_.size());
}

int ResultSet::getColumnCount() const {
    return static_cast<int>(columnNames_.size());
}

std::vector<std::string> ResultSet::getColumnNames() const {
    return columnNames_;
}

void ResultSet::addRow(const std::map<std::string, std::string>& row) {
    rows_.push_back(row);
}

void ResultSet::setColumnNames(const std::vector<std::string>& names) {
    columnNames_ = names;
}

} // namespace database
} // namespace hftools
