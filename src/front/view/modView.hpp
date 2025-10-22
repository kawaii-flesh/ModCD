#pragma once

#include <app/mcds.hpp>
#include <app/modCD.hpp>
#include <borealis.hpp>
#include <core/game.hpp>
#include <cstdint>
#include <front/view/toastView.hpp>
#include <string>
#include <utils/flagsManager.hpp>

namespace front {

enum Flags : uint32_t {
    STOPPING_IN_PROGRESS = 1 << 0,
    MOD_DOWNLOADING_IN_PROGRESS = 1 << 1,
    MOD_PROCESSING = 1 << 2,
    SCREENSHOTS_PROCESSING = 1 << 3,
    SCREENSHOTS_DOWNLOADING_IN_PROGRESS = 1 << 4,
    FSO_IN_PROGRESS = 1 << 5,
    SCROLL_MODE = 1 << 6,
};

class ModView : public brls::Box {
   private:
    app::ModCD &modCD;
    utils::DownloadState dsMod;
    utils::DownloadState dsScreenshots;

    brls::InputManager *input;

    brls::ScrollingFrame *description;
    brls::Box *buttons;
    brls::Box *screenshotsButtons;
    brls::Box *downloadsButtons;
    brls::Box *modButtons;

    brls::Button *screenshotsBtn;
    brls::Button *clearScreenshotsBtn;

    brls::Button *downloadModBtn;
    brls::Button *clearModBtn;

    brls::Button *installBtn;
    brls::Button *uninstallBtn;

    std::unique_ptr<ToastView> toast;

    long mcdsSize;
    long modSize;
    long screenshotsSize;
    std::string downloadBtnText;
    std::string screenshotsBtnText;

    const std::string viewScreenshotsText;
    const std::string screenshotsText;
    const std::string clearText;
    const std::string installText;
    const std::string installingText;
    const std::string uninstall;
    const std::string uninstallingText;
    const std::string loadingText;
    const std::string clearingText;
    const std::string extractingText;

   private:
    void prepareOnline();
    void prepareOffline();
    void draw(NVGcontext *vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext *ctx) override;
    void setAllButtonsState(brls::ButtonState state);
    void updateButtonsByContent();
    void updateButtonsByFlags();

    bool xAction();
    bool downloadModAction();
    bool clearModAction();
    bool screenshotsAction();
    bool screenshotsActionOffline();
    bool clearScreenshotsAction();
    bool installAction();
    bool uninstallAction();

    bool canStopModDownloading() const noexcept;
    void stopModDownloading();
    bool isModProcessingState() const noexcept;
    void unsetModDownloadingState();
    utils::DownloadingResult downloadModAndMCDS();
    void showModDownloadingResult(utils::DownloadingResult result);
    void setModProcessingState();
    void setModDownloadingState();

    bool canStopScreenshotsDownloading() const noexcept;
    void setScreenshotsStoppingState();
    bool isScreenshotsProcessingInProgress() const noexcept;
    std::list<std::filesystem::path> extractScreenshots();
    void showScreenshots(std::list<std::filesystem::path> screenshots);
    void handleScreenshotsDownloadingError(utils::DownloadingResult result);
    void setScreenshotsProcessingState();
    utils::DownloadingResult checkScreenshotsArchive();
    void setScreenshotsDownloadingState();

    void setFSOState(brls::Button *button, const std::string &buttonText);
    void unsetFSOState(brls::Button *button, const std::string &buttonText);

    void setFlagsAndUpdateButtons(uint32_t flagsToSet);
    void clearFlagsAndUpdateButtons(uint32_t flagsToClear);

   public:
    ModView(app::ModCD &aModCD);
    ~ModView();
    void stopAll();
    utils::FlagManager flags;
};
}  // namespace front
