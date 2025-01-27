#include "http.hpp"

#include <curl/curl.h>
#include <curl/easy.h>

#include <borealis.hpp>
#include <fstream>
#include <utils/utils.hpp>

namespace {
constexpr size_t CURL_BUFFERS_SIZE = 1024 * 1024 * 10;
constexpr char agent[] = MODCD_AGENT;

struct InnerData {
    utils::DownloadState *ds;
    std::ofstream *file;
    char *buffer;
    size_t bufferSize;
    size_t bufferPos;
};

size_t writeCallback(void *contents, size_t size, size_t nmemb, std::string *data) {
    const auto readSize = size * nmemb;
    data->append(reinterpret_cast<const char *>(contents), readSize);
    return readSize;
}

size_t writeCallbackToFile(void *contents, size_t size, size_t nmemb, void *userp) {
    InnerData *innerData = static_cast<InnerData *>(userp);
    utils::DownloadState *ds = innerData->ds;
    std::ofstream *file = innerData->file;
    const size_t totalSize = size * nmemb;

    if (!file || (ds && ds->toStop)) {
        return 0;
    }

    if (innerData->bufferPos + totalSize > innerData->bufferSize) {
        file->write(innerData->buffer, innerData->bufferPos);
        innerData->bufferPos = 0;
    }

    memcpy(innerData->buffer + innerData->bufferPos, contents, totalSize);
    innerData->bufferPos += totalSize;

    if (innerData->bufferPos == innerData->bufferSize) {
        file->write(innerData->buffer, innerData->bufferSize);
        innerData->bufferPos = 0;
    }

    if (ds) {
        ds->alreadyDownloaded += totalSize;
    }
    return totalSize;
}
}  // namespace

namespace utils {

std::mutex HttpRequester::curlMutex;

HttpRequester::HttpRequester() noexcept {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
}

HttpRequester::~HttpRequester() {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

std::string HttpRequester::getFullUrl(const std::string &baseUrl, const std::string &url) {
    if (utils::startsWith(url, "http://") || utils::startsWith(url, "https://")) {
        return url;
    }

    return (baseUrl.back() == '/' ? baseUrl.substr(0, baseUrl.size() - 1) : baseUrl) +
           (url.front() == '/' ? url : "/" + url);
}

std::string HttpRequester::getText(const std::string &url, long timeout) const {
    std::lock_guard<std::mutex> guard(curlMutex);
    std::string response;

    constexpr size_t expectedSize = 1024 * 1024;
    response.reserve(expectedSize);

    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, agent);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
    curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, CURL_BUFFERS_SIZE);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        MODCD_LOG_ERROR("[{}]: error - {} | url: {}", __PRETTY_FUNCTION__, curl_easy_strerror(res), url);
        throw CURLException();
    }

    return response;
}

DownloadingResult HttpRequester::downloadFile(const std::string &url, const std::string &filePath,
                                              DownloadState *ds) const {
    if (ds) {
        ds->alreadyDownloaded = 0;
        ds->totalSize = getFileSize(url);
    }

    std::lock_guard<std::mutex> guard(curlMutex);

    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        MODCD_LOG_ERROR("[{}]: Can't open file - path: {}", __PRETTY_FUNCTION__, filePath);
        return DownloadingResult::ERROR;
    }

    size_t bufferSize = 128000;
    auto buffer = std::make_unique<char[]>(bufferSize);

    curl_easy_reset(curl);

    InnerData innerData = {.ds = ds, .file = &file, .buffer = buffer.get(), .bufferSize = bufferSize, .bufferPos = 0};

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, agent);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallbackToFile);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &innerData);
    curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, bufferSize);

    CURLcode res = curl_easy_perform(curl);

    if (innerData.bufferPos > 0) {
        file.write(innerData.buffer, innerData.bufferPos);
    }
    file.close();

    DownloadingResult result = DownloadingResult::OK;
    if (res == CURLE_WRITE_ERROR) {
        MODCD_LOG_ERROR("[{}]: Stopping", __PRETTY_FUNCTION__);
        if (ds) {
            ds->toStop = false;
        }
        result = DownloadingResult::STOP;
    } else if (res != CURLE_OK) {
        result = DownloadingResult::ERROR;
    }

    if (result != DownloadingResult::OK) {
        utils::remove(filePath);
        MODCD_LOG_ERROR("[{}]: error - {} | url: {}", __PRETTY_FUNCTION__, curl_easy_strerror(res), url);
    }

    return result;
}

long HttpRequester::getFileSize(const std::string &url) const {
    std::lock_guard<std::mutex> guard(curlMutex);
    CURLcode res;
    long file_size = -1;

    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, agent);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallbackToFile);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
        res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &file_size);
        if (res != CURLE_OK || file_size < 0) {
            file_size = -1;
        }
    }

    return file_size;
}

}  // namespace utils
