#pragma once

#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

namespace core {
class ModInfo {
   public:
    std::string name;
    std::string description;
    std::string type;
    std::string author;
    std::string url;
    std::vector<uint64_t> supportedVersions;

   public:
    ModInfo() noexcept = default;

    ModInfo(std::string &&aName, std::string &&aDescription, std::string &&aType, std::string &&aAuthor,
            std::string &&aUrl, std::vector<uint64_t> &&aSupportedVersions) noexcept;
    ModInfo(const std::string &aName, const std::string &aDescription, const std::string &aType,
                     const std::string &aAuthor, const std::string &aUrl,
                     const std::vector<uint64_t> &aSupportedVersions) noexcept;

        static ModInfo fromJson(const nlohmann::json &j);
};
}  // namespace core
