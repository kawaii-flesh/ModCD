#include "modView.hpp"

#include <app/mcds.hpp>
#include <app/modCD.hpp>
#include <borealis.hpp>
#include <core/game.hpp>
#include <core/mergedInfo.hpp>
#include <front/activity/screenshotsActivity.hpp>
#include <front/asyncLock.hpp>
#include <front/styles/buttons.hpp>
#include <front/view/toastView.hpp>
#include <nlohmann/json.hpp>
#include <regex>
#include <utils/constants.hpp>
#include <utils/localization.hpp>
#include <utils/utils.hpp>

namespace front {
void baImagePaths(std::list<std::filesystem::path> &paths) {
    MODCD_LOG_DEBUG("[{}]: start", __PRETTY_FUNCTION__);
    std::regex before_regex(R"(.*before_(\d+).*)");
    std::regex after_regex(R"(.*after_(\d+).*)");
    std::smatch match;
    for (auto &before_path : paths) {
        std::string before_path_str = before_path.string();

        if (std::regex_match(before_path_str, match, before_regex)) {
            std::string before_path_number = match[1];

            auto after_it = std::find_if(paths.begin(), paths.end(), [&](const std::filesystem::path &path) {
                std::string path_str = path.string();
                return std::regex_match(path_str, match, after_regex) && match[1] == before_path_number;
            });

            if (after_it != paths.end()) {
                before_path += modcd_constants::DELIMITER + after_it->string();
                paths.erase(after_it);
            }
        }
    }
    MODCD_LOG_DEBUG("[{}]: end", __PRETTY_FUNCTION__);
}

bool ModView::xAction() {
    bool isNeedDescriptionFocus = !this->flags.areFlagsSet(Flags::SCROLL_MODE);
    this->description->setFocusable(isNeedDescriptionFocus);
    this->screenshotsBtn->setFocusable(!isNeedDescriptionFocus);
    this->downloadModBtn->setFocusable(!isNeedDescriptionFocus);
    this->clearModBtn->setFocusable(!isNeedDescriptionFocus);
    this->clearScreenshotsBtn->setFocusable(!isNeedDescriptionFocus);
    this->installBtn->setFocusable(!isNeedDescriptionFocus);
    this->uninstallBtn->setFocusable(!isNeedDescriptionFocus);
    if (isNeedDescriptionFocus) {
        this->flags.setFlags(Flags::SCROLL_MODE);
        brls::Application::giveFocus(this->description);
    } else {
        this->flags.clearFlags(Flags::SCROLL_MODE);
        brls::Application::giveFocus(this->downloadModBtn);
    }
    return true;
}

bool ModView::canStopModDownloading() const noexcept {
    return this->flags.areFlagsSet(Flags::MOD_DOWNLOADING_IN_PROGRESS | Flags::MOD_PROCESSING) &&
           !this->flags.areFlagsSet(Flags::STOPPING_IN_PROGRESS);
}

void ModView::stopModDownloading() {
    this->setFlagsAndUpdateButtons(Flags::STOPPING_IN_PROGRESS);
    this->dsMod.toStop = true;
}

bool ModView::isModProcessingState() const noexcept {
    return this->flags.isAnyFlagSet(Flags::MOD_PROCESSING | Flags::MOD_DOWNLOADING_IN_PROGRESS |
                                    Flags::STOPPING_IN_PROGRESS);
}

void ModView::setModProcessingState() {
    this->setFlagsAndUpdateButtons(Flags::MOD_PROCESSING);
    this->downloadModBtn->setText(this->loadingText);
}

void ModView::setModDownloadingState() {
    this->dsMod.totalSize = 0;
    this->dsMod.alreadyDownloaded = 0;
    this->setFlagsAndUpdateButtons(Flags::MOD_DOWNLOADING_IN_PROGRESS);
}

utils::DownloadingResult ModView::downloadModAndMCDS() {
    utils::DownloadingResult result = this->modCD.downloadMcds(&this->dsMod);
    if (result == utils::DownloadingResult::OK) {
        result = this->modCD.downloadMod(&this->dsMod);
        if (result != utils::DownloadingResult::OK) {
            utils::removeAndEmpty(this->modCD.getMcdsPath());
        } else {
            this->saveMergedInfo();
        }
    }

    const std::filesystem::path modPath = this->modCD.getDownloadModPathArchive();
    const std::filesystem::path mcdsPath = this->modCD.getMcdsPath();
    std::error_code err;
    if (std::filesystem::file_size(modPath, err) != this->modSize ||
        std::filesystem::file_size(mcdsPath, err) != this->mcdsSize) {
        utils::removeAndEmpty(modPath);
        utils::removeAndEmpty(mcdsPath);
    }

    return result;
}

void ModView::showModDownloadingResult(utils::DownloadingResult result) {
    if (result == utils::DownloadingResult::STOP) {
        this->toast->start(utils::Localization::get("ModView.Toast.DownloadOfTheModArchiveHasBeenStopped"),
                           ToastColor::ERROR);
    } else if (result == utils::DownloadingResult::OK) {
        this->toast->start(utils::Localization::get("ModView.Toast.ModArchiveHasBeenDownloaded"), ToastColor::OK);
    } else {
        this->toast->start(utils::Localization::get("ModView.Toast.DownloadErrorHasOccurred"), ToastColor::ERROR);
    }
}

void ModView::unsetModDownloadingState() {
    this->clearFlagsAndUpdateButtons(Flags::MOD_DOWNLOADING_IN_PROGRESS | Flags::MOD_PROCESSING |
                                     Flags::STOPPING_IN_PROGRESS);
}

bool ModView::downloadModAction() {
    if (canStopModDownloading()) {
        stopModDownloading();
        return true;
    }
    if (isModProcessingState()) {
        return false;
    }
    setModProcessingState();
    brls::async([this]() {
        std::lock_guard<std::mutex> guard(asyncMutex);
        brls::sync([this]() { setModDownloadingState(); });
        utils::DownloadingResult result = downloadModAndMCDS();

        brls::sync([this, result]() {
            showModDownloadingResult(result);
            unsetModDownloadingState();
        });
    });
    return true;
}

void ModView::setFSOState(brls::Button *button, const std::string &buttonText) {
    this->setFlagsAndUpdateButtons(Flags::FSO_IN_PROGRESS);
    button->setText(buttonText);
}

void ModView::unsetFSOState(brls::Button *button, const std::string &buttonText) {
    this->clearFlagsAndUpdateButtons(Flags::FSO_IN_PROGRESS);
    button->setText(buttonText);
}

bool ModView::clearModAction() {
    brls::async([this]() {
        std::lock_guard<std::mutex> guard(asyncMutex);
        brls::sync([this]() { setFSOState(this->clearModBtn, this->clearingText); });
        utils::removeAndEmpty(this->modCD.getDownloadModPathArchive());
        if (!this->mcds->isModInstalledByUninstallPaths()) {
            utils::removeAndEmpty(this->modCD.getMcdsPath());
        }
        brls::sync([this]() {
            unsetFSOState(this->clearModBtn, this->clearText);

            this->toast->start(utils::Localization::get("ModView.Toast.ModArchiveHasBeenDeleted"), ToastColor::OK);
        });
    });
    return true;
}

bool ModView::canStopScreenshotsDownloading() const noexcept {
    return this->flags.areFlagsSet(Flags::SCREENSHOTS_DOWNLOADING_IN_PROGRESS) &&
           !this->flags.areFlagsSet(Flags::STOPPING_IN_PROGRESS);
}

void ModView::setScreenshotsStoppingState() {
    this->setFlagsAndUpdateButtons(Flags::STOPPING_IN_PROGRESS);
    this->dsScreenshots.toStop = true;
}

bool ModView::isScreenshotsProcessingInProgress() const noexcept {
    return this->flags.isAnyFlagSet(Flags::SCREENSHOTS_PROCESSING | Flags::SCREENSHOTS_DOWNLOADING_IN_PROGRESS |
                                    Flags::STOPPING_IN_PROGRESS);
}

void ModView::setScreenshotsProcessingState() {
    this->setFlagsAndUpdateButtons(Flags::SCREENSHOTS_PROCESSING);
    this->screenshotsBtn->setText(this->loadingText);
}

void ModView::setScreenshotsDownloadingState() {
    this->flags.setFlags(Flags::SCREENSHOTS_DOWNLOADING_IN_PROGRESS);
    this->flags.clearFlags(Flags::SCREENSHOTS_PROCESSING);
    this->updateButtonsByFlags();
    this->dsScreenshots.totalSize = 0;
    this->dsScreenshots.alreadyDownloaded = 0;
}

void ModView::handleScreenshotsDownloadingError(utils::DownloadingResult result) {
    if (result == utils::DownloadingResult::ERROR) {
        this->toast->start(utils::Localization::get("ModView.Toast.DownloadErrorHasOccurred"), ToastColor::ERROR);
    } else if (result == utils::DownloadingResult::STOP) {
        this->toast->start(utils::Localization::get("ModView.Toast.DownloadOfTheScreenshotsArchiveHasBeenStopped"),
                           ToastColor::ERROR);
    }
    this->flags.clearAll();
    this->updateButtonsByFlags();
}

utils::DownloadingResult ModView::checkScreenshotsArchive() {
    const std::string screenshotsPath = this->modCD.getSHDownloadArchivePath();
    std::error_code err;
    if (std::filesystem::file_size(screenshotsPath, err) != this->screenshotsSize) {
        utils::removeAndEmpty(screenshotsPath);
        return utils::DownloadingResult::ERROR;
    } else {
        this->saveMergedInfo();
        return utils::DownloadingResult::OK;
    }
}

std::list<std::filesystem::path> ModView::extractScreenshots() {
    std::list<std::filesystem::path> files = this->modCD.getExtractedScreenshots();
    if (files.empty()) {
        brls::sync([this]() { this->screenshotsBtn->setText(this->extractingText); });
        files = this->modCD.extractScreenshots();
    }
    return files;
}

void ModView::showScreenshots(std::list<std::filesystem::path> screenshots) {
    this->clearFlagsAndUpdateButtons(Flags::SCREENSHOTS_PROCESSING);
    if (!screenshots.empty()) {
        baImagePaths(screenshots);
        brls::Application::pushActivity(new ScreenshotsActivity(std::move(screenshots)),
                                        brls::TransitionAnimation::NONE);
    } else {
        this->toast->start(utils::Localization::get("ModView.Toast.ErrorWhenUnpackingScreenshots"), ToastColor::ERROR);
    }
}

bool ModView::screenshotsAction() {
    if (canStopScreenshotsDownloading()) {
        setScreenshotsStoppingState();
    }
    if (isScreenshotsProcessingInProgress()) {
        return false;
    }
    setScreenshotsProcessingState();
    brls::async([this]() {
        std::lock_guard<std::mutex> guard(asyncMutex);
        utils::DownloadingResult result = utils::DownloadingResult::OK;
        if (!this->modCD.isScreenshotsDownloaded()) {
            brls::sync([this]() { setScreenshotsDownloadingState(); });
            result = this->modCD.downloadScreenshots(&this->dsScreenshots);
            this->flags.clearFlags(Flags::SCREENSHOTS_DOWNLOADING_IN_PROGRESS);
            if (result == utils::DownloadingResult::OK) {
                brls::sync([this]() {
                    this->toast->start(utils::Localization::get("ModView.Toast.ScreenshotsArchiveHasBeenDownloaded"),
                                       ToastColor::OK);
                });
            }
        }
        if (this->modCD.isScreenshotsDownloaded()) {
            result = checkScreenshotsArchive();
        }
        const std::string scrDir = this->modCD.getSHDownloadDirectoryPath();
        if (result != utils::DownloadingResult::OK) {
            utils::removeAndEmpty(scrDir);
            brls::sync([this, result]() { handleScreenshotsDownloadingError(result); });
        } else {
            brls::sync([this]() { setScreenshotsProcessingState(); });
            std::list<std::filesystem::path> screenshots = extractScreenshots();
            brls::sync([this, screenshots]() { showScreenshots(screenshots); });
        }
    });
    return true;
}

bool ModView::screenshotsActionOffline() {
    if (isScreenshotsProcessingInProgress()) {
        return false;
    }
    setScreenshotsProcessingState();
    brls::async([this]() {
        std::list<std::filesystem::path> screenshots = extractScreenshots();
        brls::sync([this, screenshots]() { showScreenshots(screenshots); });
    });
    return true;
}

bool ModView::clearScreenshotsAction() {
    brls::async([this]() {
        std::lock_guard<std::mutex> guard(asyncMutex);
        brls::sync([this]() { setFSOState(this->clearScreenshotsBtn, this->clearingText); });
        utils::removeAndEmpty(this->modCD.getSHDownloadDirectoryPath());
        brls::sync([this]() {
            unsetFSOState(this->clearScreenshotsBtn, this->clearText);

            this->toast->start(utils::Localization::get("ModView.Toast.ScreenshotsHaveBeenDeleted"), ToastColor::OK);
        });
    });
    return true;
}

bool ModView::installAction() {
    brls::async([this]() {
        std::lock_guard<std::mutex> guard(asyncMutex);
        brls::sync([this]() { setFSOState(this->installBtn, this->installingText); });
        this->mcds->executeRule("install");
        brls::sync([this]() {
            unsetFSOState(this->installBtn, this->installText);

            this->toast->start(utils::Localization::get("ModView.Toast.ModHasBeenInstalled"), ToastColor::OK);
        });
    });
    return true;
}

bool ModView::uninstallAction() {
    brls::async([this]() {
        std::lock_guard<std::mutex> guard(asyncMutex);
        brls::sync([this]() { setFSOState(this->uninstallBtn, this->uninstallingText); });
        this->mcds->executeRule("uninstall");
        brls::sync([this]() {
            bool isModDownloaded = this->modCD.isModDownloaded();
            if (!isModDownloaded) {
                utils::removeAndEmpty(this->modCD.getMcdsPath());
            }
            unsetFSOState(this->uninstallBtn, this->uninstall);
            this->toast->start(utils::Localization::get("ModView.Toast.ModHasBeenUninstalled"), ToastColor::OK);
        });
    });
    return true;
}

void ModView::saveMergedInfo() {
    const core::ModInfo &modInfo = this->modCD.getCurrentModInfo();
    const core::ModEntry &modEntry = this->modCD.getCurrentModEntry();
    core::MergedInfo mergedInfo(modInfo.name, modInfo.description, modInfo.type, modInfo.author, modEntry.gameVersion,
                                this->modCD.getCurrentTitleId(), modEntry.sha256);
    utils::saveJsonToFile(this->modCD.getMergedInfoPath(), mergedInfo.toJson());
}

void ModView::setFlagsAndUpdateButtons(uint32_t flagsToSet) {
    this->flags.setFlags(flagsToSet);
    this->updateButtonsByFlags();
}

void ModView::clearFlagsAndUpdateButtons(uint32_t flagsToClear) {
    this->flags.clearFlags(flagsToClear);
    this->updateButtonsByFlags();
}

void ModView::prepareOnline() {
    const auto &currentModEntry = this->modCD.getCurrentModEntry();
    this->mcdsSize = modCD.getDownloadFileSize(currentModEntry.mcds);
    this->modSize = modCD.getDownloadFileSize(currentModEntry.mod);
    this->downloadBtnText = utils::Localization::getInterpolated("ModView.ModButton.Download",
                                                                 {{"size", utils::convertToUnit(this->modSize)}});
    this->downloadModBtn->setText(this->downloadBtnText);

    const std::string modPath = this->modCD.getDownloadModPathArchive();
    const std::string mcdsPath = this->modCD.getMcdsPath();
    std::error_code err;
    if (std::filesystem::file_size(mcdsPath, err) != this->mcdsSize) {
        utils::removeAndEmpty(modPath);
        utils::removeAndEmpty(mcdsPath);
    } else if (std::filesystem::file_size(modPath, err) != this->modSize) {
        utils::removeAndEmpty(modPath);
    }

    if (!currentModEntry.screenshots.empty()) {
        this->screenshotsSize = modCD.getDownloadFileSize(currentModEntry.screenshots);
        this->screenshotsBtnText = utils::Localization::getInterpolated(
            "ModView.ScreenshotsButton.Download", {{"size", utils::convertToUnit(screenshotsSize)}});
        this->screenshotsBtn->setText(this->screenshotsBtnText);
        this->screenshotsBtn->registerClickAction([this](brls::View *view) { return screenshotsAction(); });
        this->screenshotsBtn->setMinWidth(this->screenshotsBtn->getWidth());
        this->screenshotsBtn->setMargins(10.0f, 10.0f, 10.0f, 10.0f);

        const std::string screenshotsPath = this->modCD.getSHDownloadArchivePath();
        std::error_code err;
        if (this->modCD.isScreenshotsDownloaded() &&
            std::filesystem::file_size(screenshotsPath, err) != this->screenshotsSize) {
            utils::removeAndEmpty(screenshotsPath);
        }

        const std::string scrDir = this->modCD.getSHDownloadDirectoryPath();
        if (utils::isDirectoryEmpty(scrDir)) {
            utils::removeAndEmpty(scrDir);
        }

    } else {
        this->screenshotsBtn->setVisibility(brls::Visibility::GONE);
        this->clearScreenshotsBtn->setVisibility(brls::Visibility::GONE);
    }

    if (this->modCD.isMcdsExists() || this->modCD.isScreenshotsDownloaded()) {
        this->saveMergedInfo();
    }
    this->downloadModBtn->registerClickAction([this](brls::View *view) { return downloadModAction(); });
}

void ModView::prepareOffline() {
    this->screenshotsBtnText = this->screenshotsText;
    this->screenshotsBtn->setText(this->screenshotsBtnText);
    this->screenshotsBtn->setMinWidth(this->screenshotsBtn->getWidth());
    this->screenshotsBtn->setMargins(10.0f, 10.0f, 10.0f, 10.0f);
    if (this->modCD.isScreenshotsDownloaded()) {
        this->screenshotsBtn->setState(brls::ButtonState::ENABLED);
        this->screenshotsBtn->setText(this->viewScreenshotsText);
        this->screenshotsBtn->registerClickAction([this](brls::View *view) { return screenshotsActionOffline(); });
    } else {
        this->screenshotsBtn->setState(brls::ButtonState::DISABLED);
    }

    this->downloadModBtn->setState(brls::ButtonState::DISABLED);
    this->downloadModBtn->setText(utils::Localization::get("ModView.ModButton.Placeholder"));
}

ModView::ModView(app::ModCD &aModCD)
    : brls::Box(brls::Axis::COLUMN),
      modCD(aModCD),
      viewScreenshotsText(utils::Localization::get("ModView.ScreenshotsButton.ViewScreenshots")),
      screenshotsText(utils::Localization::get("ModView.ScreenshotsButton.Placeholder")),
      clearText(utils::Localization::get("ModView.ClearScreenshotsButton.Placeholder")),
      installText(utils::Localization::get("ModView.InstallModButton.Placeholder")),
      installingText(utils::Localization::get("ModView.InstallModButton.Progress")),
      uninstall(utils::Localization::get("ModView.UninstallModButton.Placeholder")),
      uninstallingText(utils::Localization::get("ModView.UninstallModButton.Progress")),
      loadingText(utils::Localization::get("ModView.ModButton.Loading")),
      clearingText(utils::Localization::get("ModView.CommonButton.Clearing")),
      extractingText(utils::Localization::get("ModView.ScreenshotsButton.Extracting")) {
    this->mcds = std::make_unique<app::MCDS>(this->modCD.getDownloadModPathDir());
    this->input = brls::Application::getPlatform()->getInputManager();

    this->description = new brls::ScrollingFrame();
    this->buttons = new brls::Box(brls::Axis::ROW);
    this->screenshotsButtons = new brls::Box(brls::Axis::ROW);
    this->downloadsButtons = new brls::Box(brls::Axis::ROW);
    this->modButtons = new brls::Box(brls::Axis::ROW);

    this->screenshotsBtn = new brls::Button();
    this->clearScreenshotsBtn = new brls::Button();

    this->downloadModBtn = new brls::Button();
    this->clearModBtn = new brls::Button();

    this->installBtn = new brls::Button();
    this->uninstallBtn = new brls::Button();

    dsMod.toStop = false;
    dsScreenshots.toStop = false;
    this->flags.clearAll();
    dsMod.totalSize = 0;
    dsScreenshots.totalSize = 0;
    dsMod.alreadyDownloaded = 0;
    dsScreenshots.alreadyDownloaded = 0;

    if (this->modCD.isOnlineMode()) {
        this->prepareOnline();
        if (this->modCD.isMcdsExists() || this->modCD.isScreenshotsDownloaded()) {
            this->saveMergedInfo();
        }
    } else {
        this->prepareOffline();
    }

    this->clearScreenshotsBtn->setText(this->clearText);

    brls::Label *descriptionLbl = new brls::Label();
    descriptionLbl->setIsWrapping(true);
    descriptionLbl->setMargins(10.0f, 30.0f, 10.0f, 10.0f);
    descriptionLbl->setText(this->modCD.getDescription());

    this->installBtn->setText(this->installText);
    this->clearModBtn->setText(this->clearText);
    this->uninstallBtn->setText(this->uninstall);

    this->screenshotsBtn->setStyle(&MCD_BUTTONSTYLE_PRIMARY);
    this->downloadModBtn->setStyle(&MCD_BUTTONSTYLE_PRIMARY);
    this->installBtn->setStyle(&MCD_BUTTONSTYLE_PRIMARY);
    this->clearModBtn->setStyle(&MCD_BUTTONSTYLE_PRIMARY);
    this->clearScreenshotsBtn->setStyle(&MCD_BUTTONSTYLE_PRIMARY);
    this->uninstallBtn->setStyle(&MCD_BUTTONSTYLE_PRIMARY);

    brls::Box *descriptionSecondContainer = new brls::Box();
    this->description->setHeightPercentage(85.0f);
    this->description->setWidthPercentage(96.0f);
    this->description->setContentView(descriptionSecondContainer);
    descriptionSecondContainer->addView(descriptionLbl);
    this->description->setFocusable(false);
    this->description->setBorderThickness(4.0f);
    this->description->setBorderColor(MCDBorderColor);

    this->addView(this->description);

    this->downloadModBtn->setMargins(10.0f, 10.0f, 10.0f, 10.0f);
    this->downloadModBtn->setMinWidth(this->downloadModBtn->getWidth());
    this->downloadsButtons->addView(this->downloadModBtn);
    this->clearModBtn->setMargins(10.0f, 10.0f, 10.0f, 10.0f);
    this->clearScreenshotsBtn->setMargins(10.0f, 50.0f, 10.0f, 10.0f);
    this->downloadsButtons->addView(this->clearModBtn);
    this->clearModBtn->setWidth(120);
    this->clearScreenshotsBtn->setWidth(120);
    this->screenshotsButtons->addView(this->screenshotsBtn);
    this->screenshotsButtons->addView(this->clearScreenshotsBtn);
    this->buttons->addView(this->screenshotsButtons);
    this->buttons->addView(this->downloadsButtons);

    this->installBtn->setMargins(10.0f, 10.0f, 10.0f, 10.0f);
    this->installBtn->setWidth(140);
    this->modButtons->addView(this->installBtn);
    this->uninstallBtn->setMargins(10.0f, 10.0f, 10.0f, 10.0f);
    this->uninstallBtn->setWidth(140);
    this->modButtons->addView(this->uninstallBtn);
    this->modButtons->setMargins(0.0f, 0.0f, 0.0f, 50.0f);
    this->buttons->addView(this->modButtons);

    this->setAlignItems(brls::AlignItems::CENTER);
    this->setJustifyContent(brls::JustifyContent::FLEX_END);

    this->buttons->setHeightPercentage(10.0f);
    this->buttons->setMargins(10.0f, 10.0f, 10.0f, 10.0f);
    this->buttons->setBorderThickness(4.0f);
    this->buttons->setBorderColor(MCDBorderColor);
    this->buttons->setWidthPercentage(96.0f);
    this->buttons->setJustifyContent(brls::JustifyContent::CENTER);
    this->addView(this->buttons);

    this->toast = std::make_unique<ToastView>(this, brls::Point(1280.0f / 2, round(720.0f - (720.0f / 3 / 2)) - 40.0f));

    this->clearModBtn->registerClickAction([this](brls::View *view) { return clearModAction(); });
    this->clearScreenshotsBtn->registerClickAction([this](brls::View *view) { return clearScreenshotsAction(); });
    this->installBtn->registerClickAction([this](brls::View *view) { return installAction(); });
    this->uninstallBtn->registerClickAction([this](brls::View *view) { return uninstallAction(); });
    this->registerAction("", brls::ControllerButton::BUTTON_X, [this](brls::View *) { return xAction(); });

    this->updateButtonsByFlags();
}

ModView::~ModView() {
    this->stopAll();
    if (!this->modCD.isScreenshotsDownloaded() && !this->modCD.isMcdsExists()) {
        utils::removeAndEmpty(this->modCD.getDescriptionPath());
        utils::removeAndEmpty(this->modCD.getMergedInfoPath());
        if (!this->modCD.isOnlineMode()) {
            brls::Dialog *offlineDialog =
                new brls::Dialog(utils::Localization::get("ModView.OfflineDialog.ApplicationNeedsToBeRestarted"));
            offlineDialog->addButton("OK", []() { brls::Application::quit(); });
            offlineDialog->setCancelable(false);
            offlineDialog->open();
        }
    }
}

void ModView::setAllButtonsState(brls::ButtonState state) {
    this->screenshotsBtn->setState(state);
    this->downloadModBtn->setState(state);
    this->clearModBtn->setState(state);
    this->clearScreenshotsBtn->setState(state);
    this->installBtn->setState(state);
    this->uninstallBtn->setState(state);
}

void ModView::updateButtonsByContent() {
    bool isMCDSDownloaded = this->modCD.isMcdsExists();
    bool isModDownloaded = this->modCD.isModDownloaded();
    bool isScreenshotsDownloaded = this->modCD.isScreenshotsDownloaded();

    this->clearModBtn->setState(isModDownloaded ? brls::ButtonState::ENABLED : brls::ButtonState::DISABLED);
    this->clearScreenshotsBtn->setState(isScreenshotsDownloaded ? brls::ButtonState::ENABLED
                                                                : brls::ButtonState::DISABLED);

    this->installBtn->setState((isModDownloaded && isMCDSDownloaded) ? brls::ButtonState::ENABLED
                                                                     : brls::ButtonState::DISABLED);
    this->uninstallBtn->setState((isMCDSDownloaded && this->mcds->isModInstalledByUninstallPaths())
                                     ? brls::ButtonState::ENABLED
                                     : brls::ButtonState::DISABLED);

    this->screenshotsBtn->setState(isScreenshotsDownloaded || this->modCD.isOnlineMode() ? brls::ButtonState::ENABLED
                                                                                         : brls::ButtonState::DISABLED);
    this->screenshotsBtn->setText(isScreenshotsDownloaded ? this->viewScreenshotsText : this->screenshotsBtnText);
}

void ModView::updateButtonsByFlags() {
    if (this->modCD.isOnlineMode()) {
        if (this->flags.isAnyFlagSet(Flags::MOD_DOWNLOADING_IN_PROGRESS)) {
            setAllButtonsState(brls::ButtonState::DISABLED);
            this->downloadModBtn->setState(brls::ButtonState::ENABLED);
            return;
        }

        if (this->flags.isAnyFlagSet(Flags::SCREENSHOTS_DOWNLOADING_IN_PROGRESS)) {
            setAllButtonsState(brls::ButtonState::DISABLED);
            this->screenshotsBtn->setState(brls::ButtonState::ENABLED);
            return;
        }

        if (this->flags.isAnyFlagSet(Flags::FSO_IN_PROGRESS | Flags::STOPPING_IN_PROGRESS | Flags::MOD_PROCESSING |
                                     Flags::SCREENSHOTS_PROCESSING)) {
            setAllButtonsState(brls::ButtonState::DISABLED);
            return;
        }

        if (!asyncMutex.try_lock()) {
            return;
        }
        asyncMutex.unlock();

        this->downloadModBtn->setState(brls::ButtonState::ENABLED);
        this->downloadModBtn->setText(this->downloadBtnText);

        updateButtonsByContent();
    } else {
        if (this->flags.isAnyFlagSet(Flags::FSO_IN_PROGRESS | Flags::STOPPING_IN_PROGRESS | Flags::MOD_PROCESSING |
                                     Flags::SCREENSHOTS_PROCESSING)) {
            setAllButtonsState(brls::ButtonState::DISABLED);
            return;
        }

        updateButtonsByContent();
    }
}

void ModView::draw(NVGcontext *vg, float x, float y, float width, float height, brls::Style style,
                   brls::FrameContext *ctx) {
    const std::string stopPrefix = "■ ";

    utils::DownloadState *targetState = nullptr;
    brls::Button *targetButton = nullptr;
    if (this->flags.areFlagsSet(Flags::MOD_DOWNLOADING_IN_PROGRESS)) {
        targetState = &this->dsMod;
        targetButton = this->downloadModBtn;
    } else if (this->flags.areFlagsSet(Flags::SCREENSHOTS_DOWNLOADING_IN_PROGRESS)) {
        targetState = &this->dsScreenshots;
        targetButton = this->screenshotsBtn;
    }

    if (targetButton) {
        const std::string progress =
            stopPrefix + (targetState->totalSize != -1
                              ? utils::calculateProgress(targetState->totalSize, targetState->alreadyDownloaded)
                              : utils::convertToUnit(targetState->alreadyDownloaded));
        targetButton->setText(progress);
    }

    brls::Box::draw(vg, x, y, width, height, style, ctx);
}

void ModView::stopAll() {
    dsMod.toStop = true;
    dsScreenshots.toStop = true;
    std::lock_guard<std::mutex> guard(asyncMutex);
    dsMod.toStop = false;
    dsScreenshots.toStop = false;
}
}  // namespace front
