#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace hftools {
namespace model_old {

class BaseEntity {
public:
    BaseEntity() = default;
    virtual ~BaseEntity() = default;

    // JSON conversion
    virtual nlohmann::json toJson() const = 0;

private:
    int _uniqueId = 0;
    std::string _internalName;
};

class FXInstrument : public BaseEntity {
public:
    FXInstrument() = default;

    // JSON conversion
    virtual nlohmann::json toJson() const;

    private:
    int _id = 0;
    int _userId = 0;
    int _instrumentId = 0;
    std::string _side; // "BUY" or "SELL"
    double _quantity = 0.0;
    double _price = 0.0;
    std::string _timestamp;
};

} // namespace model_old
} // namespace hftools
