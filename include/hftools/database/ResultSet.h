#pragma once

#include <string>
#include <vector>
#include <map>

namespace hftools {
namespace database {

/**
 * @brief Represents a result set from a database query
 */
class ResultSet {
public:
    ResultSet();
    virtual ~ResultSet();

    /**
     * @brief Move to the next row in the result set
     * @return true if there is a next row, false otherwise
     */
    virtual bool next();

    /**
     * @brief Get a field value as string
     * @param columnName Name of the column
     * @return Field value as string
     */
    virtual std::string getField(const std::string& columnName) const;

    /**
     * @brief Get a field value as integer
     * @param columnName Name of the column
     * @return Field value as int
     */
    virtual int getInt(const std::string& columnName) const;

    /**
     * @brief Get a field value as double
     * @param columnName Name of the column
     * @return Field value as double
     */
    virtual double getDouble(const std::string& columnName) const;

    /**
     * @brief Check if field is null
     * @param columnName Name of the column
     * @return true if field is null, false otherwise
     */
    virtual bool isNull(const std::string& columnName) const;

    /**
     * @brief Get the number of rows in the result set
     * @return Number of rows
     */
    virtual int getRowCount() const;

    /**
     * @brief Get the number of columns in the result set
     * @return Number of columns
     */
    virtual int getColumnCount() const;

    /**
     * @brief Get column names
     * @return Vector of column names
     */
    virtual std::vector<std::string> getColumnNames() const;

    // For testing/mock implementation
    void addRow(const std::map<std::string, std::string>& row);
    void setColumnNames(const std::vector<std::string>& names);

protected:
    std::vector<std::map<std::string, std::string>> rows_;
    std::vector<std::string> columnNames_;
    int currentRow_;
};

} // namespace database
} // namespace hftools
