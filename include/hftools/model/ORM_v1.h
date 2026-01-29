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

class FXInstrument : public BaseEntity {
public:
    FXInstrument() = default;

    nlohmann::json toJson() const override;
    static FXInstrument fromJson(const nlohmann::json& j);

private:
    int _id = 0;
    int _userId = 0;
    int _instrumentId = 0;
    std::string _side; // "BUY" or "SELL"
    double _quantity = 0.0;
    double _price = 0.0;
    std::string _timestamp;

    template<typename T> friend struct EntityTraits;
};

} // namespace model
} // namespace hftools

//
// =======================
// 2. Generic DB interface (prepared only)
// =======================
//

class IDatabase {
public:
    virtual ~IDatabase() = default;

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

//
// =======================
// 3. Column & ColumnList
// =======================
//

template <typename T, typename FieldType>
struct Column {
    std::string_view name;
    FieldType T::* member;
};

template <typename T, typename... Cols>
struct ColumnList {
    static constexpr auto columns = std::tuple<Cols...>{};
};

//
// =======================
// 4. EntityTraits<FXInstrument>
// =======================
//

template<typename T>
struct EntityTraits; // primary template

template<>
struct EntityTraits<hftools::model::FXInstrument> {
    using Entity = hftools::model::FXInstrument;

    static constexpr std::string_view tableName  = "employees";
    static constexpr std::string_view primaryKey = "id";

    static constexpr auto columns = ColumnList<
        Entity,
        Column<Entity, int>{ "id", &Entity::_id },
        Column<Entity, int>{ "userId", &Entity::_userId },
        Column<Entity, int>{ "instrumentId", &Entity::_instrumentId },
        Column<Entity, std::string>{ "side", &Entity::_side },
        Column<Entity, double>{ "quantity", &Entity::_quantity },
        Column<Entity, double>{ "price", &Entity::_price },
        Column<Entity, std::string>{ "timestamp", &Entity::_timestamp }
    >::columns;
};

//
// =======================
// 5. Tuple for_each helper
// =======================
//

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
T autoFromJson(const nlohmann::json& j) {
    T obj;
    for_each(EntityTraits<T>::columns, [&](auto col) {
        using FieldType = decltype(obj.*(col.member));
        obj.*(col.member) = j.at(col.name).template get<FieldType>();
    });
    return obj;
}

inline nlohmann::json hftools::model::FXInstrument::toJson() const {
    return autoToJson(*this);
}

inline hftools::model::FXInstrument
hftools::model::FXInstrument::fromJson(const nlohmann::json& j) {
    return autoFromJson<hftools::model::FXInstrument>(j);
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
    explicit Repository(IDatabase& db) : db_(db) {}

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
    IDatabase& db_;
};

//
// =======================
// 9. Example usage (sketch)
// =======================
//
// class MyDatabase : public IDatabase {
//     // implement queryOnePrepared / queryManyPrepared / executePrepared
// };
//
// MyDatabase db;
// Repository<hftools::model::FXInstrument> repo(db);
//
// auto e  = repo.getById(42);
// auto all = repo.getAll();
// repo.insert(e);
// repo.update(e);
// repo.remove(e);
//
