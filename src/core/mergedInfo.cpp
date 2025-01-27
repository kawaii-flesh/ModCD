#include <core/mergedInfo.hpp>
#include <nlohmann/json.hpp>

namespace core {
MergedInfo::MergedInfo(std::string&& modName, std::string&& description, std::string&& modType, std::string&& modAuthor,
                       const uint64_t supportedVersion, std::string&& gameTitleId, std::string&& hash) noexcept
    : name(std::move(modName)),
      description(std::move(description)),
      type(std::move(modType)),
      author(std::move(modAuthor)),
      supportedVersion(supportedVersion),
      titleId(std::move(gameTitleId)),
      hash(std::move(hash)) {}

MergedInfo::MergedInfo(const std::string& modName, const std::string& description, const std::string& modType,
                       const std::string& modAuthor, const uint64_t supportedVersion, const std::string& gameTitleId,
                       const std::string& hash) noexcept
    : name(modName),
      description(description),
      type(modType),
      author(modAuthor),
      supportedVersion(supportedVersion),
      titleId(gameTitleId),
      hash(hash) {}

nlohmann::json MergedInfo::toJson() const noexcept {
    return nlohmann::json{
        {"name", this->name},     {"description", this->description},           {"type", this->type},
        {"author", this->author}, {"supportedVersion", this->supportedVersion}, {"titleId", this->titleId},
        {"hash", this->hash}};
}

MergedInfo MergedInfo::fromJson(const std::string& jsonString) noexcept {
    nlohmann::json j = nlohmann::json::parse(jsonString);

    return MergedInfo(j.at("name").get<std::string>(), j.at("description").get<std::string>(),
                      j.at("type").get<std::string>(), j.at("author").get<std::string>(),
                      j.at("supportedVersion").get<uint64_t>(), j.at("titleId").get<std::string>(),
                      j.at("hash").get<std::string>());
}
}  // namespace core
