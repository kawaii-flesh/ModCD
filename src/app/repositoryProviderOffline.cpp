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

    for (const std::string &filePath : this->modCD.getMergedInfoFiles()) {
        try {
            std::string jsonContent = utils::readFile(filePath);

            core::MergedInfo mergedInfo = core::MergedInfo::fromJson(jsonContent);

            auto existingRepContent = std::find_if(
                contents.begin(), contents.end(),
                [&mergedInfo](const core::RepContent &repContent) { return repContent.titleId == mergedInfo.titleId; });

            core::ModInfo modInfo(std::move(mergedInfo.name), std::move(mergedInfo.description),
                                  std::move(mergedInfo.type), std::move(mergedInfo.author), mergedInfo.toJson().dump(),
                                  std::vector<uint64_t>{mergedInfo.supportedVersion});
            if (existingRepContent != contents.end()) {
                existingRepContent->mods.push_back(std::move(modInfo));
            } else {
                contents.emplace_back(std::move(mergedInfo.titleId), std::string(offlineString),
                                      std::vector<core::ModInfo>{modInfo});
            }
        } catch (const std::exception &ex) {
            MODCD_LOG_ERROR("[{}]: Error processing file '{}': {}", __PRETTY_FUNCTION__, filePath, ex.what());
        }
    }

    MODCD_LOG_DEBUG("[{}]: end", __PRETTY_FUNCTION__);
    std::string offlineStringCopy = offlineString;
    return std::make_unique<core::Repository>(std::move(offlineString), std::move(offlineStringCopy),
                                              std::move(contents));
}
};  // namespace app
