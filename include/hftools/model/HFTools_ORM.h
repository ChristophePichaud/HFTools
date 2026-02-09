#pragma once

#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <type_traits>
#include <utility>
#include <optional>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include <map>
#include <set>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

namespace hftools {

// =============================================================================
// 1. UTILS: Timestamp Handling
// =============================================================================
namespace utils {
    using Timestamp = std::chrono::system_clock::time_point;

    inline std::string timePointToString(Timestamp tp) {
        auto tt = std::chrono::system_clock::to_time_t(tp);
        std::tm tm = *std::gmtime(&tt);
        std::stringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    inline Timestamp stringToTimePoint(const std::string& str) {
        if (str.empty()) return {};
        std::tm tm = {};
        std::stringstream ss(str);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        return std::chrono::system_clock::from_time_t(std::mktime(&tm));
    }
}

// =============================================================================
// 2. MODEL: Static Reflection & Metadata
// =============================================================================
namespace model {

    template <typename T, typename FieldType>
    struct Column {
        std::string_view name;
        FieldType T::* member;
    };

    class BaseEntity {
    public:
        virtual ~BaseEntity() = default;
        virtual nlohmann::json toJson() const = 0;
    };

    // Metaprogramming helper for tuple iteration
    template <typename Tuple, typename Func, std::size_t... I>
    constexpr void for_each_impl(Tuple&& t, Func&& f, std::index_sequence<I...>) {
        (f(std::get<I>(t)), ...);
    }

    template <typename Tuple, typename Func>
    constexpr void for_each(Tuple&& t, Func&& f) {
        using T = std::remove_reference_t<Tuple>;
        for_each_impl(std::forward<Tuple>(t), std::forward<Func>(f), std::make_index_sequence<std::tuple_size_v<T>>{});
    }

    template<typename T> struct EntityTraits; // Specialization required per entity

    template<typename T>
    inline nlohmann::json autoToJson(const T& obj) {
        nlohmann::json j;
        for_each(EntityTraits<T>::columns, [&](auto&& col) {
            auto& val = obj.*(col.member);
            if constexpr (std::is_same_v<std::decay_t<decltype(val)>, utils::Timestamp>) {
                j[std::string(col.name)] = utils::timePointToString(val);
            } else {
                j[std::string(col.name)] = val;
            }
        });
        return j;
    }

    template<typename T>
    inline T autoFromJson(const nlohmann::json& j) {
        T obj;
        for_each(EntityTraits<T>::columns, [&](auto&& col) {
            std::string key(col.name);
            if (j.contains(key) && !j.at(key).is_null()) {
                auto& val = obj.*(col.member);
                if constexpr (std::is_same_v<std::decay_t<decltype(val)>, utils::Timestamp>) {
                    val = utils::stringToTimePoint(j.at(key).get<std::string>());
                } else {
                    j.at(key).get_to(val);
                }
            }
        });
        return obj;
    }
}

// =============================================================================
// 3. DB: Data Abstraction Layer (Value, Row, Reader)
// =============================================================================
namespace db {

    class DBValue {
        std::string data_;
        bool isNull_;
    public:
        explicit DBValue(std::string val, bool isNull = false) : data_(std::move(val)), isNull_(isNull) {}
        
        template <typename T>
        T as() const {
            if (isNull_) return T{};
            if constexpr (std::is_same_v<T, int>) return std::stoi(data_);
            else if constexpr (std::is_same_v<T, double>) return std::stod(data_);
            else if constexpr (std::is_same_v<T, std::string>) return data_;
            else if constexpr (std::is_same_v<T, utils::Timestamp>) return utils::stringToTimePoint(data_);
            else return T{};
        }
    };

    class DBRow {
        std::vector<DBValue> columns_;
    public:
        explicit DBRow(std::vector<DBValue> cols) : columns_(std::move(cols)) {}
        const DBValue& operator[](size_t i) const { return columns_.at(i); }
        size_t size() const { return columns_.size(); }
    };

    class DBReader {
        std::vector<std::string> columnNames_;
        std::vector<DBRow> rows_;
        int currentRow_ = -1;
        size_t currentCol_ = 0;

    public:
        DBReader(std::vector<std::string> names, std::vector<DBRow> rows) 
            : columnNames_(std::move(names)), rows_(std::move(rows)) {}

        bool next() {
            if (currentRow_ + 1 < (int)rows_.size()) {
                currentRow_++;
                currentCol_ = 0;
                return true;
            }
            return false;
        }

        template <typename T>
        void validate() const {
            using Traits = hftools::model::EntityTraits<T>;
            if (columnNames_.size() != std::tuple_size_v<decltype(Traits::columns)>)
                throw std::runtime_error("Column count mismatch in DBReader");
        }

        template <typename T>
        DBReader& operator>>(T& val) {
            val = rows_[currentRow_][currentCol_++].as<T>();
            return *this;
        }

        template <typename T>
        friend DBReader& operator>>(DBReader& reader, T& obj) {
            if (reader.currentRow_ == 0 && reader.currentCol_ == 0) reader.validate<T>();
            hftools::model::for_each(hftools::model::EntityTraits<T>::columns, [&](auto&& col) {
                reader >> (obj.*(col.member));
            });
            return reader;
        }
    };

    // =============================================================================
    // 4. DB: Connection Pooling
    // =============================================================================
    class PostgresConnectionPool {
        std::queue<std::unique_ptr<pqxx::connection>> pool_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::string connStr_;

    public:
        PostgresConnectionPool(const std::string& str, size_t size) : connStr_(str) {
            for (size_t i = 0; i < size; ++i) pool_.push(std::make_unique<pqxx::connection>(str));
        }

        auto borrow() {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return !pool_.empty(); });
            auto conn = std::move(pool_.front());
            pool_.pop();
            return conn;
        }

        void release(std::unique_ptr<pqxx::connection> conn) {
            std::lock_guard<std::mutex> lock(mutex_);
            pool_.push(std::move(conn));
            cv_.notify_one();
        }
    };

    struct PooledConnGuard {
        std::unique_ptr<pqxx::connection> conn;
        PostgresConnectionPool& pool;
        ~PooledConnGuard() { pool.release(std::move(conn)); }
    };

    // =============================================================================
    // 5. DB: Database Interfaces & Implementations
    // =============================================================================
    class IDatabase {
    public:
        virtual ~IDatabase() = default;
        virtual DBReader executeQuery(const std::string& sql, const std::vector<std::string>& params) = 0;
        virtual void execute(const std::string& sql, const std::vector<std::string>& params) = 0;
    };

    class PostgresDatabase : public IDatabase {
        PostgresConnectionPool pool_;
    public:
        PostgresDatabase(const std::string& str, size_t size) : pool_(str, size) {}

        DBReader executeQuery(const std::string& sql, const std::vector<std::string>& params) override {
            PooledConnGuard guard{ pool_.borrow(), pool_ };
            pqxx::work txn(*guard.conn);
            auto res = txn.exec_params(sql, pqxx::prepare::make_dynamic_params(params));
            
            std::vector<std::string> names;
            for (int i = 0; i < res.columns(); ++i) names.push_back(res.column_name(i));

            std::vector<DBRow> rows;
            for (const auto& r : res) {
                std::vector<DBValue> vals;
                for (const auto& f : r) vals.emplace_back(f.c_str(), f.is_null());
                rows.emplace_back(std::move(vals));
            }
            txn.commit();
            return DBReader(std::move(names), std::move(rows));
        }

        void execute(const std::string& sql, const std::vector<std::string>& params) override {
            PooledConnGuard guard{ pool_.borrow(), pool_ };
            pqxx::work txn(*guard.conn);
            txn.exec_params(sql, pqxx::prepare::make_dynamic_params(params));
            txn.commit();
        }
    };
}

// =============================================================================
// 6. TOOLS: Repository & Migration & Catalog
// =============================================================================
template<typename T>
class Repository {
    db::IDatabase& db_;
public:
    explicit Repository(db::IDatabase& db) : db_(db) {}

    std::optional<T> getById(int id) {
        std::string sql = "SELECT * FROM " + std::string(model::EntityTraits<T>::tableName) + " WHERE " + std::string(model::EntityTraits<T>::primaryKey) + " = $1";
        auto reader = db_.executeQuery(sql, { std::to_string(id) });
        if (reader.next()) { T obj; reader >> obj; return obj; }
        return std::nullopt;
    }
};

class SchemaMigrator {
    db::IDatabase& db_;
public:
    explicit SchemaMigrator(db::IDatabase& db) : db_(db) {}
    
    template <typename T>
    void syncTable() {
        // Logic for CREATE TABLE / ALTER TABLE based on EntityTraits...
        // (Implementation omitted for brevity, see previous chat for full logic)
    }
};

class DBCatalog {
    db::IDatabase& db_;
public:
    explicit DBCatalog(db::IDatabase& db) : db_(db) {}
    // Logic for scanning information_schema and generating C++ code...
};

} // namespace hftools
