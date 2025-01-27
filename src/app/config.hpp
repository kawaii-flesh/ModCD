#pragma once

#include <nlohmann/json_fwd.hpp>
#include <string>

namespace app {

class Config {
   public:
    std::string repositoryUrl;
    bool logging;
    std::string loggingMode;

   public:
    Config(const std::string &aRepositoryUrl, bool aLogging, std::string aLoggingMode) noexcept;

    nlohmann::json toJson() const;

    static Config fromJson(const std::string &jsonString);
};
}  // namespace app
