#include "utils.hpp"

#include <dirent.h>
#include <minizip/unzip.h>
#include <switch.h>

#include <fslib.hpp>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <utils/constants.hpp>

using ZipFilePtr = std::unique_ptr<void, decltype(&unzClose)>;
namespace {
constexpr size_t BUFFERS_SIZE = 1024 * 1024 * 10;
}
namespace utils {

bool createDirectory(const std::filesystem::path &path) {
    MODCD_LOG_DEBUG("[{}]: start - path: {}", __PRETTY_FUNCTION__, path);

    bool res = fslib::createDirectoriesRecursively(path);

    if (res) {
        MODCD_LOG_DEBUG("[{}]: end - path: {}", __PRETTY_FUNCTION__, path);
    } else {
        MODCD_LOG_ERROR("[{}][LibFS]: {}", __PRETTY_FUNCTION__, fslib::getErrorString());
        MODCD_LOG_ERROR("[{}]: end - path: {}", __PRETTY_FUNCTION__, path);
    }

    return res;
}

bool unzipFile(const std::filesystem::path &workingDirectory, const std::filesystem::path &archPath,
               const std::filesystem::path &destDir) {
    MODCD_LOG_DEBUG("[{}]: start - wd: {} | archPath: {} | dstDir: {}", __PRETTY_FUNCTION__, workingDirectory, archPath,
                    destDir);

    ZipFilePtr zipFile(unzOpen((workingDirectory / archPath).c_str()), unzClose);
    if (!zipFile) {
        MODCD_LOG_ERROR("[{}]: Failed to open zip file: {}", __PRETTY_FUNCTION__, (workingDirectory / archPath));
        return false;
    }

    if (!createDirectory(workingDirectory) && errno != EEXIST) {
        MODCD_LOG_ERROR("[{}]: Failed to create working directory: {} (errno: {})", __PRETTY_FUNCTION__,
                        workingDirectory, errno);
        return false;
    }

    const auto &zipFilePtr = zipFile.get();
    if (unzGoToFirstFile(zipFilePtr) != UNZ_OK) {
        MODCD_LOG_ERROR("[{}]: Failed to go to the first file in zip archive: {}", __PRETTY_FUNCTION__, archPath);
        return false;
    }

    std::unique_ptr<char[]> buffer(new char[BUFFERS_SIZE]);

    do {
        char filename[256];
        if (unzGetCurrentFileInfo(zipFilePtr, nullptr, filename, sizeof(filename), nullptr, 0, nullptr, 0) != UNZ_OK) {
            MODCD_LOG_ERROR("[{}]: Failed to get file info for the current file in zip archive: {}",
                            __PRETTY_FUNCTION__, archPath);
            break;
        }

        const std::filesystem::path fullPath = destDir / std::string(filename);
        MODCD_LOG_DEBUG("[{}]: Processing file: {}", __PRETTY_FUNCTION__, fullPath);

        if (filename[strlen(filename) - 1] == '/') {
            if (!createDirectory(fullPath)) {
                MODCD_LOG_ERROR("[{}]: Failed to create directory: {}", __PRETTY_FUNCTION__, fullPath);
                break;
            }
            MODCD_LOG_DEBUG("[{}]: Directory created: {}", __PRETTY_FUNCTION__, fullPath);
            continue;
        }

        if (unzOpenCurrentFile(zipFilePtr) != UNZ_OK) {
            MODCD_LOG_ERROR("[{}]: Failed to open current file in zip archive: {}", __PRETTY_FUNCTION__, filename);
            break;
        }

        fslib::File outFile(fullPath, FsOpenMode_Write | FsOpenMode_Create);
        if (!outFile.isOpen()) {
            MODCD_LOG_ERROR("[{}]: Failed to open output file for writing: {}", __PRETTY_FUNCTION__, fullPath);
            unzCloseCurrentFile(zipFilePtr);
            break;
        }

        int bytesRead;
        while ((bytesRead = unzReadCurrentFile(zipFilePtr, buffer.get(), BUFFERS_SIZE)) > 0) {
            outFile.write(buffer.get(), bytesRead);
            MODCD_LOG_DEBUG("[{}]: Written {} bytes to file: {}", __PRETTY_FUNCTION__, bytesRead, fullPath);
        }

        if (bytesRead < 0) {
            MODCD_LOG_ERROR("[{}]: Error reading current file in zip archive: {}", __PRETTY_FUNCTION__, filename);
        }

        unzCloseCurrentFile(zipFilePtr);
        MODCD_LOG_DEBUG("[{}]: Successfully processed file: {}", __PRETTY_FUNCTION__, fullPath);

    } while (unzGoToNextFile(zipFilePtr) == UNZ_OK);

    MODCD_LOG_DEBUG("[{}]: end - wd: {} | archPath: {} | dstDir: {}", __PRETTY_FUNCTION__, workingDirectory, archPath,
                    destDir);
    return true;
}

std::list<std::filesystem::path> listFilesInZip(const std::filesystem::path &workingDirectory,
                                                const std::filesystem::path &archPath) {
    std::list<std::filesystem::path> fileList;

    ZipFilePtr zipFile(unzOpen((workingDirectory / archPath).c_str()), unzClose);
    if (!zipFile) {
        MODCD_LOG_DEBUG("[{}]: Failed to open zip file: {}", __PRETTY_FUNCTION__, archPath);
        return fileList;
    }

    const auto &zipFilePtr = zipFile.get();
    if (unzGoToFirstFile(zipFilePtr) != UNZ_OK) {
        MODCD_LOG_DEBUG("[{}]: Failed to go to first file in the zip archive.", __PRETTY_FUNCTION__);
        return fileList;
    }

    do {
        char filename[256];
        if (unzGetCurrentFileInfo(zipFilePtr, nullptr, filename, sizeof(filename), nullptr, 0, nullptr, 0) != UNZ_OK) {
            MODCD_LOG_DEBUG("[{}]: Failed to get file info.", __PRETTY_FUNCTION__);
            break;
        }

        fileList.push_back(workingDirectory / filename);

    } while (unzGoToNextFile(zipFilePtr) == UNZ_OK);

    MODCD_LOG_DEBUG("[{}]: Successfully listed files in zip archive: {}", __PRETTY_FUNCTION__, archPath);
    return fileList;
}

bool exists(const std::filesystem::path &path) { return std::filesystem::exists(path); }

bool isFile(const std::filesystem::path &path) {
    if (fslib::fileExists(path)) {
        return true;
    }
    if (fslib::directoryExists(path)) {
        return false;
    }
    throw std::runtime_error("FS Object doesn't exist: " + path.string());
}

bool remove(const std::filesystem::path &path) {
    MODCD_LOG_DEBUG("[{}]: start - path: {}", __PRETTY_FUNCTION__, path);
    if (utils::exists(path)) {
        if (isFile(path)) {
            if (!fslib::deleteFile(path)) {
                MODCD_LOG_ERROR("[{}]: failed to delete file - path: {}", __PRETTY_FUNCTION__, path);
                return false;
            }
        } else {
            if (!fslib::deleteDirectoryRecursively(path)) {
                MODCD_LOG_ERROR("[{}]: failed to delete directory recursively - path: {}", __PRETTY_FUNCTION__, path);
                return false;
            }
        }
    }
    MODCD_LOG_DEBUG("[{}]: end - path: {}", __PRETTY_FUNCTION__, path);
    return true;
}

bool isDirectoryEmpty(const std::filesystem::path &dirpath) {
    fslib::Directory dir(dirpath);

    if (!dir.isOpen()) {
        MODCD_LOG_ERROR("[{}]: Failed to open directory - path: {}", __PRETTY_FUNCTION__, dirpath);
        return false;
    }

    for (int i = 0; i < dir.getCount(); ++i) {
        const char *entryName = dir[i];
        if (entryName && (strcmp(entryName, ".") != 0) && (strcmp(entryName, "..") != 0)) {
            return false;
        }
    }

    return true;
}

bool removeAndEmpty(const std::filesystem::path &path) {
    MODCD_LOG_DEBUG("[{}]: start - path: {}", __PRETTY_FUNCTION__, path);

    if (utils::exists(path)) {
        std::filesystem::path parentDir = path.parent_path();

        if (!utils::remove(path)) {
            MODCD_LOG_ERROR("[{}]: failed to remove - path: {}", __PRETTY_FUNCTION__, path);
            return false;
        }

        if (isDirectoryEmpty(parentDir) && parentDir != "/atmosphere" && parentDir != "/atmosphere/contents" &&
            parentDir != "/.modcd" && parentDir != "/") {
            if (!removeAndEmpty(parentDir)) {
                MODCD_LOG_ERROR("[{}]: failed to remove empty directory - path: {}", __PRETTY_FUNCTION__, parentDir);
                return false;
            }
        }
    }

    MODCD_LOG_DEBUG("[{}]: end - path: {}", __PRETTY_FUNCTION__, path);
    return true;
}

bool copy(const std::filesystem::path &source, const std::filesystem::path &destination) {
    MODCD_LOG_DEBUG("[{}]: start - src: {} | dst: {}", __PRETTY_FUNCTION__, source, destination);

    std::vector<std::pair<std::filesystem::path, std::filesystem::path>> stack;
    stack.emplace_back(source, destination);

    while (!stack.empty()) {
        const auto [src, dst] = stack.back();
        stack.pop_back();

        if (!utils::exists(src)) {
            MODCD_LOG_ERROR("[{}]: Source does not exist - src: {} | dst: {}", __PRETTY_FUNCTION__, src, dst);
            return false;
        }

        if (!isFile(src)) {
            if (!createDirectory(dst)) {
                MODCD_LOG_ERROR("[{}]: Failed to create directory - src: {} | dst: {}", __PRETTY_FUNCTION__, src, dst);
                return false;
            }

            fslib::Directory dir(src);
            if (!dir.isOpen()) {
                MODCD_LOG_ERROR("[{}]: Failed to open source directory - src: {} | dst: {}", __PRETTY_FUNCTION__, src,
                                dst);
                return false;
            }

            for (int i = 0; i < dir.getCount(); ++i) {
                const char *entryName = dir[i];
                if (entryName != nullptr && strcmp(entryName, ".") != 0 && strcmp(entryName, "..") != 0) {
                    stack.emplace_back(src / entryName, dst / entryName);
                }
            }
        } else {
            fslib::File srcFile(src, FsOpenMode::FsOpenMode_Read);
            if (!srcFile.isOpen()) {
                MODCD_LOG_ERROR("[{}]: Failed to open source file - src: {} | dst: {}", __PRETTY_FUNCTION__, src, dst);
                return false;
            }

            fslib::File dstFile(dst, FsOpenMode::FsOpenMode_Write | FsOpenMode_Create);
            if (!dstFile.isOpen()) {
                MODCD_LOG_ERROR("[{}]: Failed to open destination file - src: {} | dst: {}", __PRETTY_FUNCTION__, src,
                                dst);
                return false;
            }

            std::unique_ptr<char[]> buffer(new char[BUFFERS_SIZE]);
            ssize_t bytesRead;
            while ((bytesRead = srcFile.read(buffer.get(), BUFFERS_SIZE)) > 0) {
                if (dstFile.write(buffer.get(), bytesRead) != bytesRead) {
                    MODCD_LOG_ERROR("[{}]: Error writing to destination file - src: {} | dst: {}", __PRETTY_FUNCTION__,
                                    src, dst);
                    return false;
                }
            }
        }
    }

    MODCD_LOG_DEBUG("[{}]: end - src: {} | dst: {}", __PRETTY_FUNCTION__, source, destination);
    return true;
}

bool move(const std::filesystem::path &source, const std::filesystem::path &destination) {
    MODCD_LOG_DEBUG("[{}]: start - src: {} | dst: {}", __PRETTY_FUNCTION__, source, destination);
    if (!utils::exists(source)) {
        return false;
    }

    bool result = false;
    if (isFile(source)) {
        result = fslib::renameFile(source, destination);
    } else {
        result = fslib::renameDirectory(source, destination);
    }

    if (!result) {
        MODCD_LOG_ERROR("[{}]: end - src: {} | dst: {}", __PRETTY_FUNCTION__, source, destination);
        return false;
    }
    MODCD_LOG_DEBUG("[{}]: end - src: {} | dst: {}", __PRETTY_FUNCTION__, source, destination);
    return true;
}

std::string readFile(const std::filesystem::path &filePath) {
    MODCD_LOG_DEBUG("[{}]: start - path: {}", __PRETTY_FUNCTION__, filePath);

    fslib::File file(filePath, FsOpenMode_Read);

    if (!file.isOpen()) {
        MODCD_LOG_ERROR("[{}]: Failed to open file - path: {}", __PRETTY_FUNCTION__, filePath);
        return {};
    }

    int64_t fileSize = file.getSize();
    if (fileSize <= 0) {
        MODCD_LOG_ERROR("[{}]: Invalid file size - path: {}", __PRETTY_FUNCTION__, filePath);
        return {};
    }

    std::string content(fileSize, '\0');

    ssize_t bytesRead = file.read(&content[0], fileSize);
    if (bytesRead != fileSize) {
        MODCD_LOG_ERROR("[{}]: Read size mismatch - expected: {} | actual: {} | path: {}", __PRETTY_FUNCTION__,
                        fileSize, bytesRead, filePath);
        return {};
    }

    MODCD_LOG_DEBUG("[{}]: end - path: {}", __PRETTY_FUNCTION__, filePath);
    return content;
}

std::string readFileOldWay(const std::filesystem::path &filePath) {
    MODCD_LOG_DEBUG("[{}]: start - path: {}", __PRETTY_FUNCTION__, filePath);
    std::ifstream file(filePath, std::ios::in | std::ios::binary);

    std::string content;

    file.seekg(0, std::ios::end);
    content.resize(file.tellg());

    file.seekg(0, std::ios::beg);
    file.read(&content[0], content.size());

    file.close();
    MODCD_LOG_DEBUG("[{}]: end - path: {}", __PRETTY_FUNCTION__, filePath);
    return content;
}

std::string getFileNameFromUrl(const std::string &url) {
    size_t lastSlashPos = url.rfind('/');

    if (lastSlashPos != std::string::npos) {
        return url.substr(lastSlashPos + 1);
    }

    return url;
}

std::string calculateProgress(const long totalSize, const long downloadedSize) {
    if (totalSize == 0) {
        return "0.0%";
    }

    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << (static_cast<double>(downloadedSize) * 100.0 / totalSize) << '%';

    return stream.str();
}

std::string convertToUnit(size_t size) {
    constexpr double KB = 1024.0;
    constexpr double MB = KB * 1024.0;

    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2);

    if (size < MB) {
        stream << size / KB << " KB";
    } else {
        stream << size / MB << " MB";
    }

    return stream.str();
}

void trimLabelText(const std::string &currentText, brls::Label *lbl, size_t maxPixelWidth) {
    lbl->setText(currentText);
    std::wstring wcurrentText = to_wstring(currentText);
    if (lbl->getWidth() <= maxPixelWidth) {
        return;
    }

    size_t textLength = wcurrentText.length();
    if (textLength <= 5) {
        lbl->setText("...");
        return;
    }

    size_t mid = textLength / 2;
    size_t leftPartLength = (textLength - 3) / 2;
    size_t rightPartLength = textLength - 3 - leftPartLength;

    do {
        std::wstring trimmedText = wcurrentText.substr(0, leftPartLength) + to_wstring("...") +
                                   wcurrentText.substr(textLength - rightPartLength);
        lbl->setText(to_string(trimmedText));

        --leftPartLength;
        --rightPartLength;
    } while (lbl->getWidth() > maxPixelWidth && leftPartLength > 1 && rightPartLength > 1);
}

std::wstring to_wstring(const std::string &str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

std::string to_string(const std::wstring &wstr) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

bool isConnectedToInternet() {
    NifmInternetConnectionStatus status;
    Result rc = nifmGetInternetConnectionStatus(nullptr, nullptr, &status);

    return R_SUCCEEDED(rc) && (status == NifmInternetConnectionStatus_Connected);
}

bool isApplet() { return appletGetAppletType() == AppletType_LibraryApplet; }

bool saveJsonToFile(const std::filesystem::path &filename, const nlohmann::json &j) {
    MODCD_LOG_DEBUG("[{}]: start - path: {}", __PRETTY_FUNCTION__, filename);
    try {
        fslib::File file(filename, FsOpenMode_Write | FsOpenMode_Create);

        if (!file.isOpen()) {
            MODCD_LOG_ERROR("[{}]: Failed to open file - path: {}", __PRETTY_FUNCTION__, filename);
            return false;
        }

        std::string jsonString = j.dump(4);

        ssize_t bytesWritten = file.write(jsonString.c_str(), jsonString.size());
        if (bytesWritten != static_cast<ssize_t>(jsonString.size())) {
            MODCD_LOG_ERROR("[{}]: Write size mismatch - expected: {} | actual: {} | path: {}", __PRETTY_FUNCTION__,
                            jsonString.size(), bytesWritten, filename);
            return false;
        }

        file.flush();
        MODCD_LOG_DEBUG("[{}]: end - path: {}", __PRETTY_FUNCTION__, filename);
        return true;
    } catch (const std::exception &e) {
        MODCD_LOG_ERROR("[{}]: Exception caught - path: {} | error: {}", __PRETTY_FUNCTION__, filename, e.what());
        return false;
    }
}

bool startsWith(const std::string &string, std::string_view stringBegin) noexcept {
    return string.rfind(stringBegin, 0) == 0;
}

std::pair<std::string, std::string> splitOnTwo(const std::string &str) {
    std::size_t pos = str.find(modcd_constants::DELIMITER);

    if (pos == std::string::npos) {
        return {str, ""};
    }

    std::string first_part = str.substr(0, pos);
    std::string second_part = str.substr(pos + 1);

    return {first_part, second_part};
}
}  // namespace utils
