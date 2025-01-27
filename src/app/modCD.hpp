#pragma once

#include <app/config.hpp>
#include <app/repositoryProvider.hpp>
#include <core/game.hpp>
#include <core/mod.hpp>
#include <core/repository.hpp>
#include <filesystem>
#include <list>
#include <memory>
#include <string>
#include <utils/http.hpp>

namespace app {
class ModCD {
   private:
    std::unique_ptr<core::Repository> repository;
    std::unique_ptr<RepositoryProvider> repositoryProvider;
    std::string currentTitleId;
    core::ModInfo currentModInfo;
    core::ModEntry currentModEntry;
    Config config;
    bool alreadyInited;
    bool onlineMode;
    std::list<std::filesystem::path> mergedInfoFiles;
    int argc;
    char **args;

   public:
    utils::HttpRequester httpRequester;
    std::list<core::Game> supportedGames;
    std::map<uint64_t, std::list<core::ModInfo>> supportedMods;

   public:
    ModCD(int aArgc, char *aArgv[]);

    void preInit();
    void init();

    std::filesystem::path getModCDNroPath() const noexcept;
    core::RepContent getRepContentByTitleId(const std::string &titleId) noexcept;
    core::Mod getModByModInfo(const core::ModInfo &modInfo) const noexcept;
    utils::DownloadingResult downloadMcds(utils::DownloadState *ds) const;
    utils::DownloadingResult downloadMod(utils::DownloadState *ds) const;
    utils::DownloadingResult downloadScreenshots(utils::DownloadState *ds) const;
    long getDownloadFileSize(const std::string &url) const;
    std::string getDescription() const;
    bool isModDownloaded() const noexcept;
    bool isScreenshotsDownloaded() const noexcept;
    bool isMcdsExists() const noexcept;
    std::list<std::filesystem::path> extractScreenshots() const;
    std::list<std::filesystem::path> getExtractedScreenshots() const;
    std::filesystem::path getMcdsPath() const noexcept;
    std::filesystem::path getDownloadPath() const noexcept;
    std::filesystem::path getDownloadModPathDir() const noexcept;
    std::filesystem::path getDownloadModPathArchive() const noexcept;
    std::filesystem::path getSHDownloadDirectoryPath() const noexcept;
    std::filesystem::path getSHDownloadArchivePath() const noexcept;
    void setCurrentModInfo(const core::ModInfo &modInfo) noexcept;
    void setCurrentModEntry(const core::ModEntry &modEntry) noexcept;
    void setCurrentTitleId(const std::string &titleId) noexcept;
    std::filesystem::path getModCDDirPath() const noexcept;
    const std::string &getMergedInfoName() const noexcept;
    std::filesystem::path getDescriptionPath() const;
    std::filesystem::path getMergedInfoPath() const;
    const core::ModInfo &getCurrentModInfo() const noexcept;
    const core::ModEntry &getCurrentModEntry() const noexcept;
    bool isOnlineMode() const noexcept;
    const std::string &getCurrentTitleId() const noexcept;
    std::list<std::filesystem::path> &getMergedInfoFiles() noexcept;
    const Config &getConfig() noexcept;

   private:
    core::Repository &getRepository();
    std::list<core::Game> getGamesFromStub();
    std::list<core::Game> getInstalledGames();
    void collectMergedInfoFiles(const std::filesystem::path &directoryPath, std::list<std::filesystem::path> &result);
    std::list<std::filesystem::path> collectMergedInfoFiles(const std::filesystem::path &rootPath);
    void setIsOnlineMode(bool onlineMode) noexcept;
};

}  // namespace app
