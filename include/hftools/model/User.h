#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace hftools {
namespace model {

/**
 * @brief User POCO class
 */
class User {
public:
    User() = default;
    User(int id, const std::string& username, const std::string& email, const std::string& role);

    // Getters
    int getId() const { return id_; }
    std::string getUsername() const { return username_; }
    std::string getEmail() const { return email_; }
    std::string getRole() const { return role_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setUsername(const std::string& username) { username_ = username; }
    void setEmail(const std::string& email) { email_ = email; }
    void setRole(const std::string& role) { role_ = role; }

    // JSON conversion
    nlohmann::json toJson() const;
    static User fromJson(const nlohmann::json& j);

    // Friend functions for JSON serialization
    friend void to_json(nlohmann::json& j, const User& u);
    friend void from_json(const nlohmann::json& j, User& u);

private:
    int id_ = 0;
    std::string username_;
    std::string email_;
    std::string role_;
};

// JSON serialization functions
void to_json(nlohmann::json& j, const User& u);
void from_json(const nlohmann::json& j, User& u);

} // namespace model
} // namespace hftools
