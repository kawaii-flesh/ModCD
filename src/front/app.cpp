#include "app.hpp"

#include <borealis.hpp>
#include <front/activity/gamesListActivity.hpp>
#include <front/activity/stubActivity.hpp>
#include <front/styles/colors.hpp>
#include <utils/localization.hpp>

namespace front {
App::App(int argc, char *argv[]) : modCD(argc, argv) {}

void App::start() {
    this->modCD.preInit();
    if (!brls::Application::init()) {
        MODCD_LOG_DEBUG("Unable to init Borealis application");
    }

    brls::Application::createWindow("ModCD");
    brls::Application::getPlatform()->setThemeVariant(brls::ThemeVariant::DARK);
    brls::Application::setGlobalQuit(true);

    utils::Localization::load(std::string(BRLS_RESOURCES) + "localization/en.json");
    if (utils::isApplet()) {
        brls::Application::pushActivity(new StubActivity(utils::Localization::get("Errors.RunningInAppletMode")),
                                        brls::TransitionAnimation::NONE);
    } else {
        if (!this->modCD.isOnlineMode()) {
            MCDBorderColor = MCDYellow;
        }
        brls::Application::pushActivity(new GamesListActivity(this->modCD), brls::TransitionAnimation::NONE);
    }

    while (brls::Application::mainLoop());
}
}  // namespace front
