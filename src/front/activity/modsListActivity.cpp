#include "modsListActivity.hpp"

#include <utils/utils.hpp>

namespace front {

ModsListActivity::ModsListActivity(app::ModCD &aModCD, const core::Game &aGame) noexcept
    : modCD(aModCD), game(aGame), modsRep(new ModsListView(modCD, game)) {
    MODCD_LOG_DEBUG("[{}]: created - titleId: {}", __PRETTY_FUNCTION__, aGame.titleIDToString());
}

ModsListActivity::~ModsListActivity() {
    MODCD_LOG_DEBUG("[{}]: deleted - titleId: {}", __PRETTY_FUNCTION__, this->game.titleIDToString());
}

brls::View *ModsListActivity::createContentView() { return this->modsRep; }

void ModsListActivity::onContentAvailable() {
    this->registerAction(
        "Go to games", brls::ControllerButton::BUTTON_B,
        [](brls::View *view) {
            brls::Application::popActivity(brls::TransitionAnimation::NONE);
            return true;
        },
        true);
}
}  // namespace front
