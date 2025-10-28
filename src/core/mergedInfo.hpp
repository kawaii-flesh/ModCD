#pragma once

#include <cstdint>
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace core {

enum class EnvironmentStatus {
    NONE,
    MOD_DOWNLOADED,
    INSTALLED,
    SCREENSHOTS_DOWNLOADED,
    UNINSTALLED,
    MOD_CLEANED,
    SCREENSHOTS_CLEANED
};

class MergedInfo {
   public:
    std::string name;
    std::string description;
    std::string type;
    std::string author;
    std::string hash;
    uint64_t supportedVersion;
    std::string titleId;
    EnvironmentStatus status;

   public:
    MergedInfo(const std::string& modName, const std::string& description, const std::string& modType,
               const std::string& modAuthor, const uint64_t supportedVersion, const std::string& gameTitleId,
               const std::string& hash, EnvironmentStatus status = EnvironmentStatus::NONE) noexcept;

    nlohmann::json toJson() const noexcept;

    static MergedInfo fromJson(const std::string& jsonString) noexcept;
};
}  // namespace core
