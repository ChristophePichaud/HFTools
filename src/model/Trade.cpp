#include "hftools/model/Trade.h"

namespace hftools {
namespace model {

Trade::Trade(int id, int userId, int instrumentId, const std::string& side,
             double quantity, double price, const std::string& timestamp)
    : id_(id), userId_(userId), instrumentId_(instrumentId), side_(side),
      quantity_(quantity), price_(price), timestamp_(timestamp) {
}

nlohmann::json Trade::toJson() const {
    nlohmann::json j;
    to_json(j, *this);
    return j;
}

Trade Trade::fromJson(const nlohmann::json& j) {
    Trade t;
    from_json(j, t);
    return t;
}

void to_json(nlohmann::json& j, const Trade& t) {
    j = nlohmann::json{
        {"id", t.getId()},
        {"userId", t.getUserId()},
        {"instrumentId", t.getInstrumentId()},
        {"side", t.getSide()},
        {"quantity", t.getQuantity()},
        {"price", t.getPrice()},
        {"timestamp", t.getTimestamp()}
    };
}

void from_json(const nlohmann::json& j, Trade& t) {
    j.at("id").get_to(t.id_);
    j.at("userId").get_to(t.userId_);
    j.at("instrumentId").get_to(t.instrumentId_);
    j.at("side").get_to(t.side_);
    j.at("quantity").get_to(t.quantity_);
    j.at("price").get_to(t.price_);
    j.at("timestamp").get_to(t.timestamp_);
}

} // namespace model
} // namespace hftools
