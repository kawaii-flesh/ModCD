#pragma once

#include <core/modInfo.hpp>
#include <string>
#include <vector>

namespace core {

class RepContent {
   public:
    const std::string titleId;
    const std::string gameName;
    std::vector<ModInfo> mods;

   public:
    RepContent(std::string &&aTitleId, std::string &&aGameName, std::vector<ModInfo> &&aMods) noexcept;

    static RepContent fromJson(const nlohmann::json &j);
};

class Repository {
   public:
    const std::string baseUrl;
    const std::string lastUpdated;
    const std::vector<RepContent> contents;

   public:
    Repository(std::string &&aBaseUrl, std::string &&aLastUpdated, std::vector<RepContent> &&aContents) noexcept;

    static std::unique_ptr<Repository> fromJson(const nlohmann::json &j);
};
}  // namespace core
