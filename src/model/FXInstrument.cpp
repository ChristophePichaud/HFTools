#include "hftools/model/FXInstrument.h"

namespace hftools {
namespace model {

FXInstrument::FXInstrument(int id, const std::string& symbol, const std::string& baseCurrency,
                           const std::string& quoteCurrency, double tickSize)
    : id_(id), symbol_(symbol), baseCurrency_(baseCurrency),
      quoteCurrency_(quoteCurrency), tickSize_(tickSize) {
}

nlohmann::json FXInstrument::toJson() const {
    nlohmann::json j;
    to_json(j, *this);
    return j;
}

FXInstrument FXInstrument::fromJson(const nlohmann::json& j) {
    FXInstrument fx;
    from_json(j, fx);
    return fx;
}

void to_json(nlohmann::json& j, const FXInstrument& fx) {
    j = nlohmann::json{
        {"id", fx.getId()},
        {"symbol", fx.getSymbol()},
        {"baseCurrency", fx.getBaseCurrency()},
        {"quoteCurrency", fx.getQuoteCurrency()},
        {"tickSize", fx.getTickSize()}
    };
}

void from_json(const nlohmann::json& j, FXInstrument& fx) {
    j.at("id").get_to(fx.id_);
    j.at("symbol").get_to(fx.symbol_);
    j.at("baseCurrency").get_to(fx.baseCurrency_);
    j.at("quoteCurrency").get_to(fx.quoteCurrency_);
    j.at("tickSize").get_to(fx.tickSize_);
}

} // namespace model
} // namespace hftools
