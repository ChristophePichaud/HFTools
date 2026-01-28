#include "hftools/model/User.h"

namespace hftools {
namespace model {

User::User(int id, const std::string& username, const std::string& email, const std::string& role)
    : id_(id), username_(username), email_(email), role_(role) {
}

nlohmann::json User::toJson() const {
    nlohmann::json j;
    to_json(j, *this);
    return j;
}

User User::fromJson(const nlohmann::json& j) {
    User u;
    from_json(j, u);
    return u;
}

void to_json(nlohmann::json& j, const User& u) {
    j = nlohmann::json{
        {"id", u.getId()},
        {"username", u.getUsername()},
        {"email", u.getEmail()},
        {"role", u.getRole()}
    };
}

void from_json(const nlohmann::json& j, User& u) {
    j.at("id").get_to(u.id_);
    j.at("username").get_to(u.username_);
    j.at("email").get_to(u.email_);
    j.at("role").get_to(u.role_);
}

} // namespace model
} // namespace hftools
