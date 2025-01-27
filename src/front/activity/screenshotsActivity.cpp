#include "screenshotsActivity.hpp"

#include <utils/utils.hpp>

namespace front {
ScreenshotsActivity::ScreenshotsActivity(std::list<std::filesystem::path> &&aScreenshotsFiles) {
    screenshotsView = new ScreenshotsView(std::move(aScreenshotsFiles));
    MODCD_LOG_DEBUG("[{}]: created", __PRETTY_FUNCTION__);
}
ScreenshotsActivity::~ScreenshotsActivity() { MODCD_LOG_DEBUG("[{}]: deleted", __PRETTY_FUNCTION__); }

brls::View *ScreenshotsActivity::createContentView() { return this->screenshotsView; }

void ScreenshotsActivity::onContentAvailable() {
    this->registerAction(
        "Go to content", brls::ControllerButton::BUTTON_B,
        [](brls::View *view) {
            brls::Application::popActivity(brls::TransitionAnimation::NONE);
            return true;
        },
        true);
}
}  // namespace front
