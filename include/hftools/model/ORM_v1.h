#pragma once

#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <type_traits>
#include <utility>
#include <nlohmann/json.hpp>

//
// =======================
// 1. Domain model
// =======================
//

namespace hftools {
namespace model {

// Column must be defined before EntityTraits

template <typename T, typename FieldType>
struct Column {
    std::string_view name;
    FieldType T::* member;
};

class BaseEntity {
public:
    BaseEntity() = default;
    virtual ~BaseEntity() = default;

    // JSON conversion
    virtual nlohmann::json toJson() const = 0;

protected:
    int _uniqueId = 0;
    std::string _internalName;
};

class FXInstrument2 : public BaseEntity {
public:
    FXInstrument2() = default;

    nlohmann::json toJson() const override;
    static FXInstrument2 fromJson(const nlohmann::json& j);

    int _id = 0;
    int _userId = 0;
    int _instrumentId = 0;
    std::string _side; // "BUY" or "SELL"
    double _quantity = 0.0;
    double _price = 0.0;
    std::string _timestamp;

    template<typename T> friend struct EntityTraits;
};

template<typename T>
struct EntityTraits; // primary template

template<>
struct EntityTraits<hftools::model::FXInstrument2> {
    using Entity = hftools::model::FXInstrument2;

    static constexpr std::string_view tableName  = "FXInstrument2";
    static constexpr std::string_view primaryKey = "id";

    static constexpr auto columns = std::make_tuple(
        Column<Entity, int>{ "id", &Entity::_id },
        Column<Entity, int>{ "userId", &Entity::_userId },
        Column<Entity, int>{ "instrumentId", &Entity::_instrumentId },
        Column<Entity, std::string>{ "side", &Entity::_side },
        Column<Entity, double>{ "quantity", &Entity::_quantity },
        Column<Entity, double>{ "price", &Entity::_price },
        Column<Entity, std::string>{ "timestamp", &Entity::_timestamp }
    );
};

} // namespace model
} // namespace hftools

//
// =======================
// 2. Generic DB interface (prepared only)
// =======================
//

// Generic DB interface (prepared only)
class IDatabase2 {
public:
    virtual ~IDatabase2() = default;

    virtual nlohmann::json queryOnePrepared(
        const std::string& sql,
        const std::vector<nlohmann::json>& params) = 0;

    virtual std::vector<nlohmann::json> queryManyPrepared(
        const std::string& sql,
        const std::vector<nlohmann::json>& params) = 0;

    virtual int executePrepared(
        const std::string& sql,
        const std::vector<nlohmann::json>& params) = 0;
};

class MyDatabase : public IDatabase2
{
    // implement queryOnePrepared / queryManyPrepared / executePrepared
    virtual nlohmann::json queryOnePrepared(
        const std::string& sql,
        const std::vector<nlohmann::json>& params);

    virtual std::vector<nlohmann::json> queryManyPrepared(
        const std::string& sql,
        const std::vector<nlohmann::json>& params);

    virtual int executePrepared(
        const std::string& sql,
        const std::vector<nlohmann::json>& params);
};

// Tuple for_each helper (moved up so MyDatabase can use it)
template <typename Tuple, typename Func, std::size_t... I>
constexpr void for_each_impl(Tuple&& t, Func&& f, std::index_sequence<I...>) {
    (f(std::get<I>(t)), ...);
}

template <typename Tuple, typename Func>
constexpr void for_each(Tuple&& t, Func&& f) {
    using T = std::remove_reference_t<Tuple>;
    constexpr auto N = std::tuple_size_v<T>;
    for_each_impl(std::forward<Tuple>(t),
                  std::forward<Func>(f),
                  std::make_index_sequence<N>{});
}

// Simple helper utilities for the mock database. Marked inline to avoid ODR issues in header.
inline std::string toLowerCopy(const std::string& s) {
    std::string r = s;
    for (auto& c : r) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
    return r;
}

inline std::string parseTableFromSelect(const std::string& sql) {
    auto low = toLowerCopy(sql);
    auto pos = low.find(" from ");
    if (pos == std::string::npos) pos = low.find("from ");
    if (pos == std::string::npos) return std::string();
    pos += (low.substr(pos,6) == " from " ? 6 : 5);
    // extract next token
    auto end = low.find_first_of(" \t\n(;", pos);
    if (end == std::string::npos) end = low.size();
    return sql.substr(pos, end - pos);
}

inline std::vector<std::string> parseInsertColumns(const std::string& sql) {
    // find '(' after table name
    auto posVals = sql.find("values");
    auto posOpen = sql.find('(');
    auto posClose = sql.find(')');
    std::vector<std::string> cols;
    if (posOpen == std::string::npos || posClose == std::string::npos || posClose <= posOpen) return cols;
    std::string inside = sql.substr(posOpen + 1, posClose - posOpen - 1);
    // split by commas
    size_t start = 0;
    while (start < inside.size()) {
        auto comma = inside.find(',', start);
        std::string token = (comma == std::string::npos) ? inside.substr(start) : inside.substr(start, comma - start);
        // trim
        size_t a = token.find_first_not_of(" \t\n\r\'");
        size_t b = token.find_last_not_of(" \t\n\r\'");
        if (a != std::string::npos && b != std::string::npos)
            cols.push_back(token.substr(a, b - a + 1));
        else
            cols.push_back(std::string());
        if (comma == std::string::npos) break;
        start = comma + 1;
    }
    return cols;
}

// Implementations for MyDatabase: create simple mock responses by using EntityTraits when possible.

// implement queryOnePrepared / queryManyPrepared / executePrepared
nlohmann::json MyDatabase::queryOnePrepared(
    const std::string& sql,
    const std::vector<nlohmann::json>& params)
{
    // Try to detect table name
    std::string table = parseTableFromSelect(sql);
    nlohmann::json row;

    if (toLowerCopy(table) == toLowerCopy(std::string(hftools::model::EntityTraits<hftools::model::FXInstrument2>::tableName))) {
        // Build a row containing all columns defined in EntityTraits<FXInstrument2>
        hftools::model::FXInstrument2 tmp;
        for_each(hftools::model::EntityTraits<hftools::model::FXInstrument2>::columns, [&](auto col) {
            using FieldType = std::remove_reference_t<decltype(tmp.*(col.member))>;
            std::string name = std::string(col.name);
            // If SQL contains a WHERE with primaryKey, assume first param maps to PK
            auto low = toLowerCopy(sql);
            if (low.find(std::string("where ") + std::string(hftools::model::EntityTraits<hftools::model::FXInstrument2>::primaryKey)) != std::string::npos) {
                if (name == std::string(hftools::model::EntityTraits<hftools::model::FXInstrument2>::primaryKey) && !params.empty()) {
                    // assign provided param (let json convert)
                    row[name] = params[0];
                    return;
                }
            }
            // default values based on type
            if (std::is_same<FieldType, int>::value) row[name] = FieldType{};
            else if (std::is_same<FieldType, double>::value) row[name] = FieldType{};
            else row[name] = FieldType{}; // works for std::string as well
        });
    } else {
        // Generic fallback: if there is a WHERE with $1, try to put provided params into a single "id" field
        if (!params.empty()) {
            row["id"] = params[0];
        }
    }

    return row;
}

std::vector<nlohmann::json> MyDatabase::queryManyPrepared(
    const std::string& sql,
    const std::vector<nlohmann::json>& params)
{
    std::vector<nlohmann::json> rows;
    std::string table = parseTableFromSelect(sql);

    if (toLowerCopy(table) == toLowerCopy(std::string(hftools::model::EntityTraits<hftools::model::FXInstrument2>::tableName))) {
        // Return a couple of sample rows
        for (int i = 1; i <= 2; ++i) {
            hftools::model::FXInstrument2 tmp;
            nlohmann::json row;
            for_each(hftools::model::EntityTraits<hftools::model::FXInstrument2>::columns, [&](auto col) {
                using FieldType = std::remove_reference_t<decltype(tmp.*(col.member))>;
                std::string name = std::string(col.name);
                if (name == std::string(hftools::model::EntityTraits<hftools::model::FXInstrument2>::primaryKey))
                    row[name] = i; // simple ids 1,2
                else if (std::is_same<FieldType, int>::value) row[name] = FieldType{};
                else if (std::is_same<FieldType, double>::value) row[name] = FieldType{};
                else row[name] = FieldType{};
            });
            rows.push_back(row);
        }
    } else {
        // generic fallback: return nothing or a single row with provided params mapped to $1,$2... fields
        if (!params.empty()) {
            nlohmann::json row;
            for (size_t i = 0; i < params.size(); ++i)
                row["$" + std::to_string(i+1)] = params[i];
            rows.push_back(row);
        }
    }

    return rows;
}

int MyDatabase::executePrepared(
    const std::string& sql,
    const std::vector<nlohmann::json>& params)
{
    // Support simple INSERT handling: map provided params to the column names in the SQL
    auto low = toLowerCopy(sql);
    if (low.find("insert into") != std::string::npos) {
        // try to parse column list
        auto cols = parseInsertColumns(sql);
        nlohmann::json row;
        for (size_t i = 0; i < cols.size() && i < params.size(); ++i) {
            std::string colName = cols[i];
            // strip possible quotes
            if (!colName.empty() && colName.front() == '\'') colName = colName.substr(1, colName.size()-2);
            row[colName] = params[i];
        }
        // In a real DB we'd insert the row; here we just return success
        return 1;
    }

    // For UPDATE/DELETE or others, return a generic success
    if (low.find("update") != std::string::npos || low.find("delete") != std::string::npos) return 1;

    return 0;
}

//
// =======================
// 3. Column & ColumnList
// =======================
//

//
// =======================
// 4. EntityTraits<FXInstrument2>
// =======================
//

//
// =======================
// 5. Tuple for_each helper
// =======================
//

//
// =======================
// 6. Auto JSON (to/from) from metadata
// =======================
//

template<typename T>
nlohmann::json autoToJson(const T& obj) {
    nlohmann::json j;
    for_each(EntityTraits<T>::columns, [&](auto col) {
        j[col.name] = obj.*(col.member);
    });
    return j;
}

template<typename T>
T autoFromJson(const nlohmann::json& j) 
{
    T obj;
    for_each(EntityTraits<T>::columns, [&](auto col) {
        using FieldType = std::remove_reference_t<decltype(obj.*(col.member))>;
        // Ensure we use a string key (string_view may not be accepted by older json APIs)
        j.at(std::string(col.name)).get_to(obj.*(col.member));
    });
    return obj;
}

inline nlohmann::json hftools::model::FXInstrument2::toJson() const {
    return autoToJson(*this);
}

inline hftools::model::FXInstrument2
hftools::model::FXInstrument2::fromJson(const nlohmann::json& j) 
{
    return autoFromJson<hftools::model::FXInstrument2>(j);
}

//
// =======================
// 7. SQL builders (prepared statements)
// =======================
//

// INSERT INTO table (a,b,c) VALUES ($1,$2,$3)
template <typename T>
std::string buildInsertSQL() {
    std::string sql = "INSERT INTO ";
    sql += EntityTraits<T>::tableName;
    sql += " (";

    bool first = true;
    for_each(EntityTraits<T>::columns, [&](auto col) {
        if (!first) sql += ", ";
        sql += col.name;
        first = false;
    });

    sql += ") VALUES (";

    first = true;
    int index = 1;
    for_each(EntityTraits<T>::columns, [&](auto /*col*/) {
        if (!first) sql += ", ";
        sql += "$" + std::to_string(index++);
        first = false;
    });

    sql += ")";
    return sql;
}

template<typename T>
std::vector<nlohmann::json> buildInsertParams(const T& obj) {
    std::vector<nlohmann::json> params;
    for_each(EntityTraits<T>::columns, [&](auto col) {
        params.push_back(obj.*(col.member));
    });
    return params;
}

// UPDATE table SET a=$1,b=$2 WHERE id=$N
template <typename T>
std::string buildUpdateSQL() {
    std::string sql = "UPDATE ";
    sql += EntityTraits<T>::tableName;
    sql += " SET ";

    bool first = true;
    int index = 1;

    for_each(EntityTraits<T>::columns, [&](auto col) {
        if (col.name == EntityTraits<T>::primaryKey)
            return;

        if (!first) sql += ", ";
        sql += col.name;
        sql += "=$" + std::to_string(index++);
        first = false;
    });

    sql += " WHERE ";
    sql += EntityTraits<T>::primaryKey;
    sql += "=$" + std::to_string(index);

    return sql;
}

template<typename T>
std::vector<nlohmann::json> buildUpdateParams(const T& obj) {
    std::vector<nlohmann::json> params;

    // non-PK fields first
    for_each(EntityTraits<T>::columns, [&](auto col) {
        if (col.name != EntityTraits<T>::primaryKey)
            params.push_back(obj.*(col.member));
    });

    // PK last
    for_each(EntityTraits<T>::columns, [&](auto col) {
        if (col.name == EntityTraits<T>::primaryKey)
            params.push_back(obj.*(col.member));
    });

    return params;
}

// DELETE FROM table WHERE id=$1
template <typename T>
std::string buildDeleteSQL() {
    std::string sql = "DELETE FROM ";
    sql += EntityTraits<T>::tableName;
    sql += " WHERE ";
    sql += EntityTraits<T>::primaryKey;
    sql += "=$1";
    return sql;
}

template<typename T>
std::vector<nlohmann::json> buildDeleteParams(const T& obj) {
    std::vector<nlohmann::json> params;
    for_each(EntityTraits<T>::columns, [&](auto col) {
        if (col.name == EntityTraits<T>::primaryKey)
            params.push_back(obj.*(col.member));
    });
    return params;
}

//
// =======================
// 8. Generic Repository<T> (prepared only)
// =======================
//

template<typename T>
class Repository {
public:
    explicit Repository(IDatabase2& db) : db_(db) {}

    T getById(int id) {
        std::string sql =
            "SELECT * FROM " +
            std::string(EntityTraits<T>::tableName) +
            " WHERE " +
            std::string(EntityTraits<T>::primaryKey) +
            "=$1";

        auto row = db_.queryOnePrepared(sql, { nlohmann::json(id) });
        return T::fromJson(row);
    }

    std::vector<T> getAll() {
        std::string sql =
            "SELECT * FROM " +
            std::string(EntityTraits<T>::tableName);

        auto rows = db_.queryManyPrepared(sql, {});
        std::vector<T> result;
        result.reserve(rows.size());
        for (auto& r : rows)
            result.push_back(T::fromJson(r));
        return result;
    }

    void insert(const T& obj) {
        db_.executePrepared(
            buildInsertSQL<T>(),
            buildInsertParams(obj)
        );
    }

    void update(const T& obj) {
        db_.executePrepared(
            buildUpdateSQL<T>(),
            buildUpdateParams(obj)
        );
    }

    void remove(const T& obj) {
        db_.executePrepared(
            buildDeleteSQL<T>(),
            buildDeleteParams(obj)
        );
    }

private:
    IDatabase2& db_;
};

//
// =======================
// 9. Example usage (sketch)
// =======================
//
// class MyDatabase : public IDatabase2 {
//     // implement queryOnePrepared / queryManyPrepared / executePrepared
// };
//
// MyDatabase db;
// Repository<hftools::model::FXInstrument2> repo(db);
//
// auto e  = repo.getById(42);
// auto all = repo.getAll();
// repo.insert(e);
// repo.update(e);
// repo.remove(e);
//
