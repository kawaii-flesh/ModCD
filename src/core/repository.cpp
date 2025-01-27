#include "repository.hpp"

#include <nlohmann/json.hpp>

namespace core {

RepContent::RepContent(std::string &&aTitleId, std::string &&aGameName, std::vector<ModInfo> &&aMods) noexcept
    : titleId(std::move(aTitleId)), gameName(std::move(aGameName)), mods(std::move(aMods)) {}

RepContent RepContent::fromJson(const nlohmann::json &j) {
    std::vector<ModInfo> mods;
    const auto &modsJson = j.at("mods");
    mods.reserve(modsJson.size());
    for (const nlohmann::json &contentEntry : modsJson) {
        mods.push_back(core::ModInfo::fromJson(contentEntry));
    }
    return RepContent(j.at("titleId"), j.at("gameName"), std::move(mods));
}

Repository::Repository(std::string &&aBaseUrl, std::string &&aLastUpdated, std::vector<RepContent> &&aContents) noexcept
    : baseUrl(std::move(aBaseUrl)), lastUpdated(std::move(aLastUpdated)), contents(std::move(aContents)) {}

std::unique_ptr<Repository> Repository::fromJson(const nlohmann::json &j) {
    std::vector<RepContent> contents;
    const auto &repositoryContentsJson = j.at("contents");
    contents.reserve(repositoryContentsJson.size());
    for (const nlohmann::json &contentEntry : repositoryContentsJson) {
        contents.push_back(RepContent::fromJson(contentEntry));
    }
    return std::make_unique<Repository>(j.at("baseUrl"), j.at("lastUpdated"), std::move(contents));
}

}  // namespace core
