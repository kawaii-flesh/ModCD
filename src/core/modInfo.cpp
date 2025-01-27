#include "modInfo.hpp"

#include <nlohmann/json.hpp>

namespace core {
ModInfo::ModInfo(std::string &&aName, std::string &&aDescription, std::string &&aType, std::string &&aAuthor,
                 std::string &&aUrl, std::vector<uint64_t> &&aSupportedVersions) noexcept
    : name(std::move(aName)),
      description(std::move(aDescription)),
      type(std::move(aType)),
      author(std::move(aAuthor)),
      url(std::move(aUrl)),
      supportedVersions(std::move(aSupportedVersions)) {}

ModInfo ModInfo::fromJson(const nlohmann::json &j) {
    return ModInfo(j.at("name"), j.at("description"), j.at("type"), j.at("author"), j.at("url"),
                   j.at("supportedVersions"));
}
}  // namespace core
