#pragma once

#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

namespace core {
class ModEntry {
   public:
    std::string lastUpdated;
    uint64_t gameVersion = 0;
    std::string mod;
    std::string mcds;
    std::string description;
    std::string screenshots;
    std::string sha256;

   public:
    ModEntry(std::string &&aLastUpdated, uint64_t aGameVersion, std::string &&aMod, std::string &&aMcds,
             std::string &&aDescription, std::string &&aScreenshots, std::string &&aSha256) noexcept;

    ModEntry() noexcept = default;

    static ModEntry fromJson(const nlohmann::json &j);
};

class Mod {
   public:
    const std::string lastUpdated;
    const std::vector<ModEntry> files;

   public:
    Mod(std::string &&aLastUpdated, std::vector<ModEntry> &&aFiles) noexcept;

    static Mod fromJson(const nlohmann::json &j);
};
}  // namespace core
