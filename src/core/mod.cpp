#include "mod.hpp"

#include <nlohmann/json.hpp>
#include <utils/utils.hpp>

namespace core {

ModEntry::ModEntry(std::string &&aLastUpdated, uint64_t aGameVersion, std::string &&aMod, std::string &&aMcds,
                   std::string &&aDescription, std::string &&aScreenshots, std::string &&aSha256) noexcept
    : lastUpdated(std::move(aLastUpdated)),
      gameVersion(aGameVersion),
      mod(std::move(aMod)),
      mcds(std::move(aMcds)),
      description(std::move(aDescription)),
      screenshots(std::move(aScreenshots)),
      sha256(std::move(aSha256)) {}

ModEntry ModEntry::fromJson(const nlohmann::json &j) {
    return ModEntry(j.at("lastUpdated"), j.at("gameVersion"), j.at("mod"), j.at("mcds"), j.at("description"),
                    j.at("screenshots"), j.at("sha256"));
}

Mod::Mod(std::string &&aLastUpdated, std::vector<ModEntry> &&aFiles) noexcept
    : lastUpdated(std::move(aLastUpdated)), files(std::move(aFiles)) {}

Mod Mod::fromJson(const nlohmann::json &j) {
    std::vector<ModEntry> files;
    const auto &filesJson = j.at("files");
    files.reserve(filesJson.size());
    for (const nlohmann::json &modEntry : filesJson) {
        files.push_back(ModEntry::fromJson(modEntry));
    }
    return Mod(j.at("lastUpdated"), std::move(files));
}

}  // namespace core
