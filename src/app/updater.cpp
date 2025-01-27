#include "updater.hpp"

#include <fslib.hpp>
#include <nlohmann/json.hpp>
#include <utils/http.hpp>
#include <utils/utils.hpp>

namespace {
constexpr char fullVersion[] = MODCD_FULL_VERSION;
void fillReleaseData(app::ReleaseData& releaseData, const nlohmann::json& jsonData) {
    releaseData.newVersion = jsonData.at("tag_name").get<std::string>();
    MODCD_LOG_DEBUG("[{}]: tag_name - {}", __PRETTY_FUNCTION__, releaseData.newVersion);

    if (releaseData.newVersion == fullVersion) {
        MODCD_LOG_DEBUG("[{}]: No update required - current version is the same.", __PRETTY_FUNCTION__);
        return;
    }

    if (!jsonData.contains("assets") || jsonData["assets"].empty()) {
        MODCD_LOG_ERROR("[{}]: No assets found in the release.", __PRETTY_FUNCTION__);
        return;
    }

    for (const nlohmann::json& asset : jsonData["assets"]) {
        std::string name = asset.at("name").get<std::string>();
        MODCD_LOG_DEBUG("[{}]: asset name - {}", __PRETTY_FUNCTION__, name);

        if (name == "ModCD.nro") {
            releaseData.url = asset.at("browser_download_url").get<std::string>();
            releaseData.fileSize = asset.at("size").get<size_t>();
            MODCD_LOG_DEBUG("[{}]: Found download URL for ModCD.nro - {}", __PRETTY_FUNCTION__, releaseData.url);
            MODCD_LOG_DEBUG("[{}]: Asset size - {} bytes", __PRETTY_FUNCTION__, releaseData.fileSize);
            return;
        }
    }

    MODCD_LOG_ERROR("[{}]: ModCD.nro asset not found in the release.", __PRETTY_FUNCTION__);
}
}  // namespace

namespace app {
ReleaseData::ReleaseData(const std::string& currentVersion) noexcept : currentVersion(currentVersion), fileSize(0) {}

namespace Updater {

ReleaseData getLatestReleaseInfo(const utils::HttpRequester& requester) {
    ReleaseData releaseData(fullVersion);

    std::string apiUrl = "https://api.github.com/repos/kawaii-flesh/ModCD/releases/latest";
    std::string response = requester.getText(apiUrl, 3L);
    MODCD_LOG_DEBUG("[{}]: start - API URL: {}", __PRETTY_FUNCTION__, apiUrl);

    try {
        fillReleaseData(releaseData, nlohmann::json::parse(response));
    } catch (const nlohmann::json::exception& e) {
        MODCD_LOG_ERROR("[{}]: JSON parsing error: {}", __PRETTY_FUNCTION__, e.what());
    }
    return releaseData;
}

bool update(const utils::HttpRequester& requester, const std::filesystem::path& modCDNroPath, const std::string& url,
            const std::filesystem::path& workingDir, utils::DownloadState* ds) {
    const std::string updateName = "ModCD.nro_new";
    const std::string updateFilePath = workingDir / updateName;
    const std::string backup = "ModCD.nro_backup";
    const std::string backupFilePath = workingDir / backup;
    romfsExit();
    bool result = (requester.downloadFile(url, updateFilePath, ds) == utils::DownloadingResult::OK) &&
                  utils::copy(modCDNroPath, backupFilePath) && utils::remove(modCDNroPath) &&
                  utils::copy(updateFilePath, modCDNroPath) && utils::remove(updateFilePath) &&
                  utils::remove(backupFilePath);
    return result;
}
}  // namespace Updater

}  // namespace app
