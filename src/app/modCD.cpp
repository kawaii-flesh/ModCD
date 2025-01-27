#include "modCD.hpp"

#include <algorithm>
#include <app/mcds.hpp>
#include <app/repositoryProviderOffline.hpp>
#include <app/repositoryProviderOnline.hpp>
#include <borealis.hpp>
#include <core/stub.hpp>
#include <filesystem>
#include <fslib.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <utils/http.hpp>
#include <utils/utils.hpp>

namespace {
const std::filesystem::path stubPath = "/modcd_stub.json";
const std::filesystem::path modcdDirectory = "/.modcd";
const std::string modArchiveName = "mod.zip";
const std::string descriptionName = "description";
const std::string mergedInfoName = "mod.info";
const std::string modArchDir = "mod";
const std::string screenshotsDirectory = "screenshots";
const std::string screenshotsArchiveName = "screenshots.zip";
const std::filesystem::path configFilePath = modcdDirectory / "config.json";
const std::filesystem::path logPath = modcdDirectory / "log.txt";
}  // namespace

namespace app {

ModCD::ModCD(int aArgc, char *aArgv[])
    : argc(aArgc),
      args(aArgv),
      config("http://194.226.49.62:1785/repository.json", true, "w+"),
      alreadyInited(false),
      onlineMode(false) {}

void ModCD::preInit() {
    utils::createDirectory(modcdDirectory);
    bool isOnline = utils::isConnectedToInternet();
    this->setIsOnlineMode(isOnline);
    if (!utils::exists(configFilePath)) {
        utils::saveJsonToFile(configFilePath, this->config.toJson());
    } else {
        this->config = Config::fromJson(utils::readFile(configFilePath));
    }
    if (this->config.logging) {
        brls::Logger::setLogLevel(brls::LogLevel::LOG_DEBUG);
        if (this->config.loggingMode == "nxlink" && isOnlineMode()) {
            nxlinkStdio();
        } else if (this->config.loggingMode != "nxlink") {
            brls::Logger::setLogOutput(std::fopen(logPath.c_str(), this->config.loggingMode.c_str()));
        }
    }
}

void ModCD::init() {
    if (alreadyInited) {
        return;
    }

    this->mergedInfoFiles = this->collectMergedInfoFiles(modcdDirectory);
    if (this->onlineMode) {
        repositoryProvider = std::make_unique<RepositoryProviderOnline>(this->config, this->httpRequester);
    } else {
        repositoryProvider = std::make_unique<RepositoryProviderOffline>(*this);
    }

    this->repository = this->repositoryProvider->getRepository();
    std::list<core::Game> games = utils::exists(stubPath) ? this->getGamesFromStub() : this->getInstalledGames();

    for (core::Game &game : games) {
        core::RepContent contents = this->getRepContentByTitleId(game.titleIDToString());
        if (contents.titleId == "not found") {
            continue;
        }
        bool isGameSupported = false;
        for (const core::ModInfo &modInfo : contents.mods) {
            for (uint64_t version : modInfo.supportedVersions) {
                if (version == game.version) {
                    auto [it, is_inserted] = this->supportedMods.try_emplace(game.titleId, std::list<core::ModInfo>());
                    it->second.push_back(modInfo);
                    isGameSupported = true;
                    break;
                }
            }
        }
        MODCD_LOG_DEBUG("Game reached {}", contents.titleId);
        if (isGameSupported) {
            this->supportedGames.push_back(std::move(game));
        }
    }
    MODCD_LOG_DEBUG("INITED SUCCESS");
    this->alreadyInited = true;
}

std::filesystem::path ModCD::getModCDNroPath() const noexcept {
    if (this->argc > 0) {
        return this->args[0];
    }
    return "";
}

std::list<core::Game> ModCD::getGamesFromStub() {
    std::list<core::Game> games;
    try {
        nlohmann::json jsonData = nlohmann::json::parse(utils::readFile(stubPath));
        auto stub = core::Stub::fromJson(jsonData);
        for (const auto &gameStub : stub.games) {
            MODCD_LOG_DEBUG("[{}]: stub game - titleId: {} | version: {}", __PRETTY_FUNCTION__, gameStub.titleId,
                            gameStub.version);
            games.push_back(gameStub.toGame());
        }
    } catch (const std::exception &e) {
        MODCD_LOG_DEBUG("[{}]: Error handled!", __PRETTY_FUNCTION__);
    }
    return games;
}

std::list<core::Game> ModCD::getInstalledGames() {
    std::list<core::Game> games;
    MODCD_LOG_DEBUG("[{}]: start", __PRETTY_FUNCTION__);
    for (int i = 0;; ++i) {
        try {
            NsApplicationRecord nsa;
            s32 count = 1;
            nsListApplicationRecord(&nsa, 1, i, &count);
            if (count == 0) {
                break;
            }

            NsApplicationControlData cdata;
            u64 actual_size;
            u64 appId = nsa.application_id;
            nsGetApplicationControlData(NsApplicationControlSource_Storage, appId, &cdata,
                                        sizeof(NsApplicationControlData), &actual_size);
            NacpLanguageEntry *langEntry;
            nacpGetLanguageEntry(&cdata.nacp, &langEntry);

            u64 version = 0;
            for (int j = 0;; ++j) {
                NsApplicationContentMetaStatus metaStatus;
                s32 metaCount = 1;
                nsListApplicationContentMetaStatus(appId, j, &metaStatus, 1, &metaCount);
                if (metaCount == 0) {
                    break;
                }

                if ((metaStatus.meta_type == NcmContentMetaType_Application ||
                     metaStatus.meta_type == NcmContentMetaType_Patch) &&
                    (metaStatus.version > version)) {
                    version = metaStatus.version;
                }
            }

            auto &game = games.emplace_back(!langEntry ? "" : langEntry->name, appId, cdata.icon, version);
            MODCD_LOG_DEBUG(
                "[{}]: The game was received - name: '{}' | titleId: {} "
                "| version: {}",
                __PRETTY_FUNCTION__, game.name, appId, version);
        } catch (...) {
            MODCD_LOG_ERROR("[{}]: The game receiving error", __PRETTY_FUNCTION__);
        }
    }
    MODCD_LOG_DEBUG("[{}]: end", __PRETTY_FUNCTION__);
    return games;
}

core::RepContent ModCD::getRepContentByTitleId(const std::string &titleId) noexcept {
    MODCD_LOG_DEBUG("[{}]: start", __PRETTY_FUNCTION__);

    const std::vector<core::RepContent> &contents = this->repository->contents;
    auto it = std::find_if(
        contents.begin(), contents.end(),
        [&titleId = std::as_const(titleId)](const core::RepContent &content) { return content.titleId == titleId; });
    if (it == contents.end()) {
        MODCD_LOG_DEBUG("[{}]: content by titleId was not found - titleId: {}", __PRETTY_FUNCTION__, titleId);
        return core::RepContent("not found", "not found", {});
    }

    MODCD_LOG_DEBUG("[{}]: content by titleId was found - titleId: {}", __PRETTY_FUNCTION__, titleId);
    return *it;
}

core::Mod ModCD::getModByModInfo(const core::ModInfo &modInfo) const noexcept {
    MODCD_LOG_DEBUG("[{}]: start", __PRETTY_FUNCTION__);
    const std::string modJSON =
        this->httpRequester.getText(utils::HttpRequester::getFullUrl(this->repository->baseUrl, modInfo.url));
    MODCD_LOG_DEBUG("[{}]: the mod has been received - url: {}", __PRETTY_FUNCTION__, modInfo.url);
    return core::Mod::fromJson(nlohmann::json::parse(modJSON));
}

utils::DownloadingResult ModCD::downloadMcds(utils::DownloadState *ds) const {
    const auto mcdsPathFile = this->getMcdsPath();
    utils::createDirectory(mcdsPathFile.parent_path());

    utils::DownloadingResult result = this->httpRequester.downloadFile(
        utils::HttpRequester::getFullUrl(this->repository->baseUrl, this->currentModEntry.mcds), mcdsPathFile, nullptr);
    MODCD_LOG_DEBUG("[{}]: The mod has been downloaded - url: {} | path: {}", __PRETTY_FUNCTION__,
                    this->currentModEntry.mcds, mcdsPathFile.c_str());
    return result;
}

utils::DownloadingResult ModCD::downloadMod(utils::DownloadState *ds) const {
    MODCD_LOG_DEBUG("[{}]: start", __PRETTY_FUNCTION__);
    const auto archPathFile = this->getDownloadModPathArchive();
    utils::createDirectory(archPathFile.parent_path());

    utils::DownloadingResult result = this->httpRequester.downloadFile(
        utils::HttpRequester::getFullUrl(this->repository->baseUrl, this->currentModEntry.mod), archPathFile, ds);
    MODCD_LOG_DEBUG("[{}]: The mod has been downloaded - url: {} | path: {}", __PRETTY_FUNCTION__,
                    this->currentModEntry.mod, archPathFile.c_str());
    return result;
}

utils::DownloadingResult ModCD::downloadScreenshots(utils::DownloadState *ds) const {
    MODCD_LOG_DEBUG("[{}]: start", __PRETTY_FUNCTION__);
    const auto screenshotsArchivePath = this->getSHDownloadArchivePath();
    utils::createDirectory(screenshotsArchivePath.parent_path());

    utils::DownloadingResult result = this->httpRequester.downloadFile(
        utils::HttpRequester::getFullUrl(this->repository->baseUrl, this->currentModEntry.screenshots),
        screenshotsArchivePath, ds);
    MODCD_LOG_DEBUG("[{}]: the screenshot has been downloaded - url: {} | path: {}", __PRETTY_FUNCTION__,
                    this->currentModEntry.screenshots, screenshotsArchivePath.c_str());
    return result;
}

long ModCD::getDownloadFileSize(const std::string &url) const {
    return this->httpRequester.getFileSize(utils::HttpRequester::getFullUrl(this->repository->baseUrl, url));
}

std::string ModCD::getDescription() const {
    MODCD_LOG_DEBUG("[{}]: start", __PRETTY_FUNCTION__);
    utils::createDirectory(this->getDownloadPath());
    const std::string descriptionPath = this->getDescriptionPath();
    if (this->onlineMode) {
        this->httpRequester.downloadFile(
            utils::HttpRequester::getFullUrl(this->repository->baseUrl, this->currentModEntry.description),
            descriptionPath, nullptr);
        MODCD_LOG_DEBUG("[{}]: The description has been downloaded - url: {}", __PRETTY_FUNCTION__,
                        this->currentModEntry.description);
    }
    MODCD_LOG_DEBUG("[{}]: end", __PRETTY_FUNCTION__);
    return utils::readFile(descriptionPath);
}

bool ModCD::isModDownloaded() const noexcept { return utils::exists(this->getDownloadModPathArchive()); }

bool ModCD::isScreenshotsDownloaded() const noexcept { return utils::exists(this->getSHDownloadArchivePath()); }

bool ModCD::isMcdsExists() const noexcept { return utils::exists(this->getMcdsPath()); }

std::list<std::filesystem::path> ModCD::extractScreenshots() const {
    MODCD_LOG_DEBUG("[{}]: start", __PRETTY_FUNCTION__);
    const std::filesystem::path downloadPath = this->getSHDownloadDirectoryPath();
    if (!utils::unzipFile(downloadPath, screenshotsArchiveName, downloadPath)) {
        MODCD_LOG_ERROR("[{}]: The screenshots is not unpacked - wd: {} | archPath: {} | dst: {}", __PRETTY_FUNCTION__,
                        downloadPath.c_str(), screenshotsArchiveName, downloadPath.c_str());
        return {};
    }

    MODCD_LOG_DEBUG("[{}]: The screenshots is unpacked - wd: {} | archPath: {} | dst: {}", __PRETTY_FUNCTION__,
                    downloadPath.c_str(), screenshotsArchiveName, downloadPath.c_str());

    std::list<std::filesystem::path> files;
    const std::filesystem::path scArch = downloadPath / screenshotsArchiveName;
    for (const auto &entry : std::filesystem::directory_iterator(downloadPath)) {
        if (entry.path() != scArch) {
            files.push_back(entry.path());
        }
    }
    MODCD_LOG_DEBUG("[{}]: end", __PRETTY_FUNCTION__);
    return files;
}

std::list<std::filesystem::path> ModCD::getExtractedScreenshots() const {
    MODCD_LOG_DEBUG("[{}]: start", __PRETTY_FUNCTION__);
    std::list<std::filesystem::path> files =
        utils::listFilesInZip(this->getSHDownloadDirectoryPath(), screenshotsArchiveName);
    for (const std::filesystem::path &filePath : files) {
        if (!utils::exists(filePath)) {
            MODCD_LOG_ERROR("[{}]: {} does not exists", __PRETTY_FUNCTION__, filePath);
            return {};
        }
        MODCD_LOG_DEBUG("[{}]: {} exists", __PRETTY_FUNCTION__, filePath);
    }
    MODCD_LOG_DEBUG("[{}]: end", __PRETTY_FUNCTION__);
    return files;
}

std::filesystem::path ModCD::getMcdsPath() const noexcept { return this->getDownloadModPathDir() / rulesFileName; }

std::filesystem::path ModCD::getDownloadPath() const noexcept {
    return modcdDirectory / this->currentTitleId / this->currentModInfo.author /
           std::to_string(this->currentModEntry.gameVersion) / this->currentModEntry.sha256;
}

std::filesystem::path ModCD::getDescriptionPath() const { return this->getDownloadPath() / descriptionName; }

std::filesystem::path ModCD::getMergedInfoPath() const { return this->getDownloadPath() / mergedInfoName; }

std::filesystem::path ModCD::getDownloadModPathDir() const noexcept { return this->getDownloadPath() / modArchDir; }

std::filesystem::path ModCD::getDownloadModPathArchive() const noexcept {
    return this->getDownloadModPathDir() / modArchiveName;
}

std::filesystem::path ModCD::getSHDownloadDirectoryPath() const noexcept {
    return this->getDownloadPath() / screenshotsDirectory;
}

std::filesystem::path ModCD::getSHDownloadArchivePath() const noexcept {
    return this->getSHDownloadDirectoryPath() / screenshotsArchiveName;
}

void ModCD::setCurrentModInfo(const core::ModInfo &modInfo) noexcept { this->currentModInfo = modInfo; }

void ModCD::setCurrentModEntry(const core::ModEntry &modEntry) noexcept { this->currentModEntry = modEntry; }

void ModCD::setCurrentTitleId(const std::string &titleId) noexcept { this->currentTitleId = titleId; }

std::filesystem::path ModCD::getModCDDirPath() const noexcept { return modcdDirectory; }

const std::string &ModCD::getMergedInfoName() const noexcept { return mergedInfoName; }

const core::ModInfo &ModCD::getCurrentModInfo() const noexcept { return this->currentModInfo; }

const core::ModEntry &ModCD::getCurrentModEntry() const noexcept { return this->currentModEntry; }

bool ModCD::isOnlineMode() const noexcept { return this->onlineMode; }

void ModCD::setIsOnlineMode(bool onlineMode) noexcept { this->onlineMode = onlineMode; }

const std::string &ModCD::getCurrentTitleId() const noexcept { return this->currentTitleId; }

std::list<std::filesystem::path> &ModCD::getMergedInfoFiles() noexcept { return this->mergedInfoFiles; }

const Config &ModCD::getConfig() noexcept { return this->config; }

void ModCD::collectMergedInfoFiles(const std::filesystem::path &directoryPath,
                                   std::list<std::filesystem::path> &result) {
    fslib::Directory directory(directoryPath);

    if (!directory.isOpen()) {
        return;
    }

    for (int i = 0; i < directory.getCount(); ++i) {
        if (!directory.isDirectory(i)) {
            std::string entryName = directory.getEntry(i);

            if (entryName == this->getMergedInfoName()) {
                std::string fullPath = directoryPath / entryName;
                result.push_back(fullPath);
                return;
            }
        }
    }

    for (int i = 0; i < directory.getCount(); ++i) {
        if (directory.isDirectory(i)) {
            std::string entryName = directory.getEntry(i);

            if (entryName == "." || entryName == "..") {
                continue;
            }

            std::filesystem::path fullPath = directoryPath / entryName;
            collectMergedInfoFiles(fullPath, result);
        }
    }
}

std::list<std::filesystem::path> ModCD::collectMergedInfoFiles(const std::filesystem::path &rootPath) {
    std::list<std::filesystem::path> result;
    collectMergedInfoFiles(rootPath, result);
    return result;
}
}  // namespace app
