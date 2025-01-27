#pragma once

#include <switch.h>

#include <borealis.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <utils/fmtFilesystem.hpp>

#define MODCD_LOG_COLOR "[0;35m"
#define MODCD_LOG_DEBUG(...) utils::logDebug(__VA_ARGS__)
#define MODCD_LOG_ERROR(...) utils::logError(__VA_ARGS__)
namespace utils {
bool unzipFile(const std::filesystem::path &workingDirectory, const std::filesystem::path &archPath,
               const std::filesystem::path &destDir);
std::list<std::filesystem::path> listFilesInZip(const std::filesystem::path &workingDirectory,
                                                const std::filesystem::path &archPath);

bool createDirectory(const std::filesystem::path &path);
bool exists(const std::filesystem::path &path);
bool remove(const std::filesystem::path &path);
bool isDirectoryEmpty(const std::filesystem::path &dirPath);
bool removeAndEmpty(const std::filesystem::path &path);
bool move(const std::filesystem::path &source, const std::filesystem::path &destination);
bool copy(const std::filesystem::path &source, const std::filesystem::path &destination);

std::string readFile(const std::filesystem::path &filePath);
std::string readFileOldWay(const std::filesystem::path &filePath);

std::string getFileNameFromUrl(const std::string &url);  // Здесь путь не задействован

std::string calculateProgress(const long totalSize, const long downloadedSize);
std::string convertToUnit(size_t size);

void trimLabelText(const std::string &currentText, brls::Label *lbl, size_t maxPixelWidth);

std::string to_string(const std::wstring &wstr);
std::wstring to_wstring(const std::string &str);

bool isConnectedToInternet();
bool isApplet();

std::pair<std::string, std::string> splitOnTwo(const std::string &str);

bool saveJsonToFile(const std::filesystem::path &filename, const nlohmann::json &j);

bool startsWith(const std::string &string, std::string_view stringBegin) noexcept;

template <typename... Args>
void logDebug(fmt::format_string<Args...> format, Args &&...args) {
    brls::Logger::log(brls::LogLevel::LOG_DEBUG, "ModCD", MODCD_LOG_COLOR, format, std::forward<Args>(args)...);
}
template <typename... Args>
void logError(fmt::format_string<Args...> format, Args &&...args) {
    brls::Logger::log(brls::LogLevel::LOG_ERROR, "ModCD", BRLS_ERROR_COLOR, format, std::forward<Args>(args)...);
}
}  // namespace utils
