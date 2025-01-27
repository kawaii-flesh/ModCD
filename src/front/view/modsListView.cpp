#include "modsListView.hpp"

#include <front/activity/modActivity.hpp>
#include <front/styles/colors.hpp>
#include <front/view/modTileView.hpp>
#include <utils/utils.hpp>

namespace front {
ModsListView::ModsListView(app::ModCD &aModCD, const core::Game &aGame)
    : brls::Box(brls::Axis::COLUMN), modCD(aModCD), game(aGame), scrollingContent(new brls::Box(brls::Axis::COLUMN)) {
    this->setAlignItems(brls::AlignItems::CENTER);

    const float fontSize = 24.0f;

    brls::Label *gameNameHeader = new brls::Label();
    gameNameHeader->setText(this->game.name);
    gameNameHeader->setFontSize(fontSize);

    brls::Box *labelBox = new brls::Button();
    labelBox->addView(gameNameHeader);
    labelBox->setFocusable(false);
    labelBox->setPadding(fontSize / 2, fontSize, fontSize / 2, fontSize);
    labelBox->setBorderThickness(4.0f);
    labelBox->setBorderColor(MCDBorderColor);
    labelBox->setMinWidthPercentage(90.0f);
    labelBox->setMaxWidthPercentage(90.0f);
    labelBox->setMargins(20.0f, 0.0f, 20.0f, 0.0f);
    labelBox->setJustifyContent(brls::JustifyContent::CENTER);
    this->addView(labelBox);

    this->addRows(this->modCD.supportedMods[game.titleId]);

    brls::ScrollingFrame *list = new brls::ScrollingFrame();
    list->setContentView(this->scrollingContent);
    list->setFocusable(false);
    list->setMinWidthPercentage(100.0f);
    list->setMaxWidthPercentage(100.0f);
    list->setMinHeightPercentage(86.0f);
    this->addView(list);
}

void ModsListView::addRows(std::list<core::ModInfo> &modInfos) {
    brls::Box *row = nullptr;

    for (const core::ModInfo &modInfo : modInfos) {
        row = new brls::Box(brls::Axis::ROW);
        this->scrollingContent->addView(row);

        brls::Button *button = new brls::Button();
        ModTileView *modTileView = new ModTileView(modInfo);
        button->addView(modTileView);
        button->setMargins(20.0f, 10.0f, 0.0f, 25.0f);
        button->setMaxWidthPercentage(95.0f);
        button->setMinWidthPercentage(95.0f);
        button->setJustifyContent(brls::JustifyContent::FLEX_START);

        modTileView->setMinWidthPercentage(95.0);

        button->registerClickAction([this, &modInfo](brls::View *view) {
            MODCD_LOG_DEBUG("Icon was clicked: {}", modInfo.name);
            if (this->modCD.isOnlineMode()) {
                core::Mod mod = this->modCD.getModByModInfo(modInfo);
                auto modEntryIt = std::find_if(mod.files.begin(), mod.files.end(),
                                               [version = this->game.version](const core::ModEntry &modEntry) {
                                                   return modEntry.gameVersion == version;
                                               });
                if (modEntryIt != mod.files.end()) {
                    this->modCD.setCurrentModInfo(modInfo);
                    this->modCD.setCurrentModEntry(*modEntryIt);
                    brls::Application::pushActivity(new ModActivity(this->modCD), brls::TransitionAnimation::NONE);
                } else {
                    MODCD_LOG_DEBUG("[{}]: there are no files for the current version - version: {}",
                                    __PRETTY_FUNCTION__, this->game.version);
                }
            } else {
                try {
                    core::MergedInfo mergedInfo = core::MergedInfo::fromJson(modInfo.url);
                    core::ModEntry modEntry;
                    modEntry.gameVersion = std::move(mergedInfo.supportedVersion);
                    modEntry.sha256 = std::move(mergedInfo.hash);
                    this->modCD.setCurrentModInfo(modInfo);
                    this->modCD.setCurrentModEntry(modEntry);
                    brls::Application::pushActivity(new ModActivity(this->modCD), brls::TransitionAnimation::NONE);
                } catch (const std::exception &e) {
                    MODCD_LOG_DEBUG("[{}]: Exception occured: {}", __PRETTY_FUNCTION__, e.what());
                }
            }

            return true;
        });

        row->addView(button);
        row->setAlignItems(brls::AlignItems::FLEX_START);
    }
}
}  // namespace front
