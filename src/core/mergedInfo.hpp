#pragma once

#include <cstdint>
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace core {
class MergedInfo {
   public:
    std::string name;
    std::string description;
    std::string type;
    std::string author;
    std::string hash;
    uint64_t supportedVersion;
    std::string titleId;

   public:
    MergedInfo(std::string&& modName, std::string&& description, std::string&& modType, std::string&& modAuthor,
               const uint64_t supportedVersion, std::string&& gameTitleId, std::string&& hash) noexcept;

    MergedInfo(const std::string& modName, const std::string& description, const std::string& modType,
               const std::string& modAuthor, const uint64_t supportedVersion, const std::string& gameTitleId,
               const std::string& hash) noexcept;

    nlohmann::json toJson() const noexcept;

    static MergedInfo fromJson(const std::string& jsonString) noexcept;
};
};  // namespace core
