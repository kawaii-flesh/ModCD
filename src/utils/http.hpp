#pragma once

#include <atomic>
#include <mutex>
#include <string>

namespace utils {
struct DownloadState {
    long totalSize;
    std::size_t alreadyDownloaded;
    std::atomic<bool> toStop;
};

enum class DownloadingResult { OK, ERROR, STOP };

class CURLException : public std::exception {
   public:
    const char *what() const noexcept override { return "CURL Exception"; }
};

class HttpRequester {
   private:
    void *curl;
    static std::once_flag curlInitFlag;
    static std::mutex curlMutex;

   public:
    HttpRequester();
    ~HttpRequester();

    std::string getText(const std::string &url, long timeout = 5L) const;

    DownloadingResult downloadFile(const std::string &url, const std::string &filePath, DownloadState *ds) const;

    long getFileSize(const std::string &url) const;

    static std::string getFullUrl(const std::string &baseUrl, const std::string &url);
};
}  // namespace utils
