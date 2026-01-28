#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace hftools {
namespace model {

/**
 * @brief Trade POCO class
 */
class Trade {
public:
    Trade() = default;
    Trade(int id, int userId, int instrumentId, const std::string& side,
          double quantity, double price, const std::string& timestamp);

    // Getters
    int getId() const { return id_; }
    int getUserId() const { return userId_; }
    int getInstrumentId() const { return instrumentId_; }
    std::string getSide() const { return side_; }
    double getQuantity() const { return quantity_; }
    double getPrice() const { return price_; }
    std::string getTimestamp() const { return timestamp_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setUserId(int userId) { userId_ = userId; }
    void setInstrumentId(int instrumentId) { instrumentId_ = instrumentId; }
    void setSide(const std::string& side) { side_ = side; }
    void setQuantity(double quantity) { quantity_ = quantity; }
    void setPrice(double price) { price_ = price; }
    void setTimestamp(const std::string& timestamp) { timestamp_ = timestamp; }

    // JSON conversion
    nlohmann::json toJson() const;
    static Trade fromJson(const nlohmann::json& j);

    // Friend functions for JSON serialization
    friend void to_json(nlohmann::json& j, const Trade& t);
    friend void from_json(const nlohmann::json& j, Trade& t);

private:
    int id_ = 0;
    int userId_ = 0;
    int instrumentId_ = 0;
    std::string side_; // "BUY" or "SELL"
    double quantity_ = 0.0;
    double price_ = 0.0;
    std::string timestamp_;
};

// JSON serialization functions
void to_json(nlohmann::json& j, const Trade& t);
void from_json(const nlohmann::json& j, Trade& t);

} // namespace model
} // namespace hftools
