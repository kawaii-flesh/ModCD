#pragma once

#include <filesystem>
#include <string>

namespace utils {
class DownloadState;
class HttpRequester;
}  // namespace utils

namespace app {

class ReleaseData {
   public:
    std::string url;
    std::string currentVersion;
    std::string newVersion;
    size_t fileSize;

    ReleaseData(const std::string& currentVersion) noexcept;
};

namespace Updater {
ReleaseData getLatestReleaseInfo(const utils::HttpRequester& requester);
bool update(const utils::HttpRequester& requester, const std::filesystem::path& modCDNroPath, const std::string& url,
            const std::filesystem::path& workingDir, utils::DownloadState* ds);
}  // namespace Updater

}  // namespace app
