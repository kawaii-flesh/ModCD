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

    std::string repoUrl = j.at("repositoryUrl").get<std::string>();
    bool logging = j.at("logging").get<bool>();
    std::string loggingMode = j.at("loggingMode").get<std::string>();

    // TODO remove
    if (repoUrl == "http://194.226.49.62:1785/repository.json") {
        repoUrl = "http://thub.ddns.net:1785/repository.json";
    }

    return Config(repoUrl, logging, loggingMode);
}
}  // namespace app
