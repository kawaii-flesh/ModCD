#include "modActivity.hpp"

#include <utils/utils.hpp>

namespace front {

ModActivity::ModActivity(app::ModCD &aModCD) noexcept : modCD(aModCD) {
    this->modView = new ModView(this->modCD);
    MODCD_LOG_DEBUG("[{}]: created", __PRETTY_FUNCTION__);
}

ModActivity::~ModActivity() { MODCD_LOG_DEBUG("[{}]: deleted", __PRETTY_FUNCTION__); }

brls::View *ModActivity::createContentView() { return this->modView; }

void ModActivity::onContentAvailable() {
    this->registerAction(
        "Go to mods", brls::ControllerButton::BUTTON_B,
        [this](brls::View *view) {
            if (this->modView->flags.isAnyFlagSet(Flags::MOD_DOWNLOADING_IN_PROGRESS |
                                                  Flags::SCREENSHOTS_DOWNLOADING_IN_PROGRESS)) {
                this->modView->stopAll();
            } else if (this->modView->flags.isAnyFlagSet(Flags::MOD_PROCESSING | Flags::SCREENSHOTS_PROCESSING |
                                                         Flags::STOPPING_IN_PROGRESS | Flags::FSO_IN_PROGRESS)) {
                return false;
            } else {
                brls::Application::popActivity(brls::TransitionAnimation::NONE);
            }
            return true;
        },
        true);
}
}  // namespace front
