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

ModInfo::ModInfo(const std::string &aName, const std::string &aDescription, const std::string &aType,
                 const std::string &aAuthor, const std::string &aUrl,
                 const std::vector<uint64_t> &aSupportedVersions) noexcept
    : name(aName),
      description(aDescription),
      type(aType),
      author(aAuthor),
      url(aUrl),
      supportedVersions(aSupportedVersions) {}

ModInfo ModInfo::fromJson(const nlohmann::json &j) {
    std::string name = j.value("name", "");
    std::string description = j.value("description", "");
    std::string type = j.value("type", "");
    std::string author = j.value("author", "");
    std::string url = j.value("url", "");
    std::vector<uint64_t> supportedVersions = j.value("supportedVersions", std::vector<uint64_t>{});

    return ModInfo(std::move(name), std::move(description), std::move(type), std::move(author), std::move(url),
                   std::move(supportedVersions));
}
}  // namespace core
