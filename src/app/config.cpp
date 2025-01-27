#include "config.hpp"

#include <nlohmann/json.hpp>

namespace app {

Config::Config(const std::string &aRepositoryUrl, bool aLogging, std::string aLoggingMode) noexcept
    : repositoryUrl(aRepositoryUrl), logging(aLogging), loggingMode(aLoggingMode) {}

nlohmann::json Config::toJson() const {
    return nlohmann::json{
        {"repositoryUrl", this->repositoryUrl}, {"logging", this->logging}, {"loggingMode", this->loggingMode}};
}

Config Config::fromJson(const std::string &jsonString) {
    nlohmann::json j = nlohmann::json::parse(jsonString);
    return Config(j.at("repositoryUrl").get<std::string>(), j.at("logging").get<bool>(),
                  j.at("loggingMode").get<std::string>());
}
}  // namespace app
