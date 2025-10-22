#include <core/mergedInfo.hpp>
#include <nlohmann/json.hpp>

namespace core {

static std::string EnvironmentStatusToString(EnvironmentStatus status) noexcept {
    switch (status) {
        case EnvironmentStatus::MOD_DOWNLOADED:
            return "MOD_DOWNLOADED";
        case EnvironmentStatus::INSTALLED:
            return "INSTALLED";
        case EnvironmentStatus::SCREENSHOTS_DOWNLOADED:
            return "SCREENSHOTS_DOWNLOADED";
        case EnvironmentStatus::NONE:
        default:
            return "NONE";
    }
}

static EnvironmentStatus stringToEnvironmentStatus(const std::string& s) noexcept {
    if (s == "MOD_DOWNLOADED") {
        return EnvironmentStatus::MOD_DOWNLOADED;
    }
    if (s == "INSTALLED") {
        return EnvironmentStatus::INSTALLED;
    }
    if (s == "SCREENSHOTS_DOWNLOADED") {
        return EnvironmentStatus::SCREENSHOTS_DOWNLOADED;
    }
    return EnvironmentStatus::NONE;
}

MergedInfo::MergedInfo(const std::string& modName, const std::string& description, const std::string& modType,
                       const std::string& modAuthor, const uint64_t supportedVersion, const std::string& gameTitleId,
                       const std::string& hash, EnvironmentStatus status) noexcept
    : name(modName),
      description(description),
      type(modType),
      author(modAuthor),
      supportedVersion(supportedVersion),
      titleId(gameTitleId),
      hash(hash),
      status(status) {}

nlohmann::json MergedInfo::toJson() const noexcept {
    return nlohmann::json{
        {"name", this->name},     {"description", this->description},           {"type", this->type},
        {"author", this->author}, {"supportedVersion", this->supportedVersion}, {"titleId", this->titleId},
        {"hash", this->hash},     {"status", EnvironmentStatusToString(this->status)}};
}

MergedInfo MergedInfo::fromJson(const std::string& jsonString) noexcept {
    nlohmann::json j = nlohmann::json::parse(jsonString, nullptr, false);
    if (j.is_discarded()) {
        return MergedInfo("", "", "", "", 0, "", "", EnvironmentStatus::NONE);
    }

    return MergedInfo(j.value("name", ""), j.value("description", ""), j.value("type", ""), j.value("author", ""),
                      j.value("supportedVersion", 0ull), j.value("titleId", ""), j.value("hash", ""),
                      stringToEnvironmentStatus(j.value("status", "NONE")));
}

}  // namespace core
