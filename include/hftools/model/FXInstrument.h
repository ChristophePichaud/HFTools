#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace hftools {
namespace model {

/**
 * @brief FX Instrument POCO class
 */
class FXInstrument {
public:
    FXInstrument() = default;
    FXInstrument(int id, const std::string& symbol, const std::string& baseCurrency,
                 const std::string& quoteCurrency, double tickSize);

    // Getters
    int getId() const { return id_; }
    std::string getSymbol() const { return symbol_; }
    std::string getBaseCurrency() const { return baseCurrency_; }
    std::string getQuoteCurrency() const { return quoteCurrency_; }
    double getTickSize() const { return tickSize_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setSymbol(const std::string& symbol) { symbol_ = symbol; }
    void setBaseCurrency(const std::string& baseCurrency) { baseCurrency_ = baseCurrency; }
    void setQuoteCurrency(const std::string& quoteCurrency) { quoteCurrency_ = quoteCurrency; }
    void setTickSize(double tickSize) { tickSize_ = tickSize; }

    // JSON conversion
    nlohmann::json toJson() const;
    static FXInstrument fromJson(const nlohmann::json& j);

    // Friend functions for JSON serialization
    friend void to_json(nlohmann::json& j, const FXInstrument& fx);
    friend void from_json(const nlohmann::json& j, FXInstrument& fx);

private:
    int id_ = 0;
    std::string symbol_;
    std::string baseCurrency_;
    std::string quoteCurrency_;
    double tickSize_ = 0.0001;
};

// JSON serialization functions
void to_json(nlohmann::json& j, const FXInstrument& fx);
void from_json(const nlohmann::json& j, FXInstrument& fx);

} // namespace model
} // namespace hftools
