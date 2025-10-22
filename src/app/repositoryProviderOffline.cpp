#include "repositoryProviderOffline.hpp"

#include <core/mergedInfo.hpp>
#include <fslib.hpp>
#include <nlohmann/json.hpp>
#include <utils/utils.hpp>

namespace app {
RepositoryProviderOffline::RepositoryProviderOffline(app::ModCD &aModCD) noexcept : modCD(aModCD) {}

std::unique_ptr<core::Repository> RepositoryProviderOffline::getRepository() {
    MODCD_LOG_DEBUG("[{}]: start", __PRETTY_FUNCTION__);

    std::string offlineString = "offline";
    std::vector<core::RepContent> contents;

    for (core::MergedInfo &mergedInfo : this->modCD.getMergedInfoObjects()) {
        auto existingRepContent = std::find_if(
            contents.begin(), contents.end(),
            [&mergedInfo](const core::RepContent &repContent) { return repContent.titleId == mergedInfo.titleId; });

        core::ModInfo modInfo(mergedInfo.name, mergedInfo.description, mergedInfo.type, mergedInfo.author,
                              mergedInfo.toJson().dump(), std::vector<uint64_t>{mergedInfo.supportedVersion});
        if (existingRepContent != contents.end()) {
            existingRepContent->mods.push_back(std::move(modInfo));
        } else {
            contents.emplace_back(std::string(mergedInfo.titleId), std::string(offlineString),
                                  std::vector<core::ModInfo>{modInfo});
        }
    }

    MODCD_LOG_DEBUG("[{}]: end", __PRETTY_FUNCTION__);
    std::string offlineStringCopy = offlineString;
    return std::make_unique<core::Repository>(std::move(offlineString), std::move(offlineStringCopy),
                                              std::move(contents));
}
};  // namespace app
