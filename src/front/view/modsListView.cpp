#include "modsListView.hpp"

#include <front/activity/modActivity.hpp>
#include <front/styles/colors.hpp>
#include <front/view/modTileView.hpp>
#include <utils/localization.hpp>
#include <utils/utils.hpp>

namespace front {
ModsListView::ModsListView(app::ModCD& aModCD, const core::Game& aGame)
    : brls::Box(brls::Axis::COLUMN), modCD(aModCD), game(aGame), scrollingContent(new brls::Box(brls::Axis::COLUMN)) {
    this->setAlignItems(brls::AlignItems::CENTER);

    const float fontSize = 24.0f;

    brls::Label* gameNameHeader = new brls::Label();
    std::string version = "[ver. " + std::to_string(this->game.version) + "] - ";
    gameNameHeader->setText(version + this->game.name);
    gameNameHeader->setFontSize(fontSize);

    brls::Box* labelBox = new brls::Button();
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
    this->addRows(this->modCD.unsupportedMods[game.titleId]);

    brls::ScrollingFrame* list = new brls::ScrollingFrame();
    list->setContentView(this->scrollingContent);
    list->setFocusable(false);
    list->setMinWidthPercentage(100.0f);
    list->setMaxWidthPercentage(100.0f);
    list->setMinHeightPercentage(86.0f);
    this->addView(list);
}

void ModsListView::addRows(std::list<core::ModInfo>& modInfos) {
    brls::Box* row = nullptr;

    for (const core::ModInfo& modInfo : modInfos) {
        row = new brls::Box(brls::Axis::ROW);
        this->scrollingContent->addView(row);

        brls::Box* borderWrapper = new brls::Box();
        borderWrapper->setMargins(20.0f, 10.0f, 0.0f, 25.0f);
        borderWrapper->setMaxWidthPercentage(95.0f);
        borderWrapper->setMinWidthPercentage(95.0f);

        brls::Button* button = new brls::Button();
        ModTileView* modTileView = new ModTileView(modInfo);
        button->addView(modTileView);
        button->setJustifyContent(brls::JustifyContent::FLEX_START);

        modTileView->setMinWidthPercentage(95.0);

        button->registerClickAction([this, &modInfo](brls::View* view) {
            MODCD_LOG_DEBUG("Icon was clicked: {}", modInfo.name);
            if (this->modCD.isOnlineMode()) {
                core::Mod mod = this->modCD.getModByModInfo(modInfo);
                auto modEntryIt = std::find_if(mod.files.begin(), mod.files.end(),
                                               [version = this->game.version](const core::ModEntry& modEntry) {
                                                   return modEntry.gameVersion == version;
                                               });
                this->modCD.setCurrentModInfo(modInfo);
                if (modEntryIt != mod.files.end()) {
                    this->modCD.setCurrentModEntry(*modEntryIt);
                    brls::Application::pushActivity(new ModActivity(this->modCD, true, this),
                                                    brls::TransitionAnimation::NONE);
                } else {
                    const core::ModEntry unsupportedEntry = mod.files.back();

                    auto* dialog = new brls::Dialog(utils::Localization::getInterpolated(
                        "ModsListView.UnsupportedVersionDialog.WarningMessage",
                        {{"modVersion", std::to_string(unsupportedEntry.gameVersion)},
                         {"gameVersion", std::to_string(this->game.version)}}));

                    dialog->addButton(utils::Localization::get("ModsListView.UnsupportedVersionDialog.ConfirmButton"),
                                      [this, unsupportedEntry]() {
                                          this->modCD.setCurrentModEntry(unsupportedEntry);
                                          brls::Application::pushActivity(new ModActivity(this->modCD, false, this),
                                                                          brls::TransitionAnimation::NONE);
                                      });

                    dialog->addButton(utils::Localization::get("ModsListView.UnsupportedVersionDialog.CancelButton"),
                                      []() {});

                    dialog->open();

                    MODCD_LOG_DEBUG("[{}]: no files for current version - version: {}", __PRETTY_FUNCTION__,
                                    this->game.version);
                }
            } else {
                try {
                    core::MergedInfo mergedInfo = core::MergedInfo::fromJson(modInfo.url);
                    core::ModEntry modEntry;
                    modEntry.gameVersion = std::move(mergedInfo.supportedVersion);
                    modEntry.sha256 = std::move(mergedInfo.hash);
                    this->modCD.setCurrentModInfo(modInfo);
                    this->modCD.setCurrentModEntry(modEntry);
                    brls::Application::pushActivity(new ModActivity(this->modCD, true, this),
                                                    brls::TransitionAnimation::NONE);
                } catch (const std::exception& e) {
                    MODCD_LOG_DEBUG("[{}]: Exception occured: {}", __PRETTY_FUNCTION__, e.what());
                }
            }

            return true;
        });

        borderWrapper->addView(button);
        row->addView(borderWrapper);
        row->setAlignItems(brls::AlignItems::FLEX_START);
    }
    this->updateModTiles();
}

void ModsListView::updateModTiles() {
    const std::list<core::MergedInfo>& mergedInfoObjects = this->modCD.getMergedInfoObjects();

    for (brls::View* rowView : this->scrollingContent->getChildren()) {
        brls::Box* row = dynamic_cast<brls::Box*>(rowView);
        if (!row) {
            continue;
        }

        for (brls::View* wrapperView : row->getChildren()) {
            brls::Box* wrapper = dynamic_cast<brls::Box*>(wrapperView);
            if (!wrapper) {
                continue;
            }

            for (brls::View* buttonView : wrapper->getChildren()) {
                brls::Button* button = dynamic_cast<brls::Button*>(buttonView);
                if (!button) {
                    continue;
                }

                for (brls::View* childView : button->getChildren()) {
                    ModTileView* modTileView = dynamic_cast<ModTileView*>(childView);
                    if (!modTileView) {
                        continue;
                    }

                    auto it = std::find_if(mergedInfoObjects.begin(), mergedInfoObjects.end(),
                                           [&modTileView](const core::MergedInfo& info) {
                                               return info.name == modTileView->modInfo.name &&
                                                      info.description == modTileView->modInfo.description &&
                                                      info.type == modTileView->modInfo.type &&
                                                      info.author == modTileView->modInfo.author;
                                           });

                    bool handled = false;
                    if (it != mergedInfoObjects.end()) {
                        switch (it->status) {
                            case core::EnvironmentStatus::MOD_DOWNLOADED:
                                button->setBorderColor(MCDBlue);
                                handled = true;
                                break;
                            case core::EnvironmentStatus::INSTALLED:
                                button->setBorderColor(MCDGreen);
                                handled = true;
                                break;
                            case core::EnvironmentStatus::SCREENSHOTS_DOWNLOADED:
                                button->setBorderColor(MCDYellow);
                                handled = true;
                                break;
                            default:
                                break;
                        }
                    }

                    if (handled) {
                        button->setBorderThickness(1.0f);
                    } else {
                        const auto& supportedVersions = modTileView->modInfo.supportedVersions;
                        const bool supportsCurrentVersion =
                            std::find(supportedVersions.begin(), supportedVersions.end(), this->game.version) !=
                            supportedVersions.end();

                        if (!supportsCurrentVersion) {
                            button->setBorderColor(MCDRed);
                            button->setBorderThickness(1.0f);
                        } else {
                            button->setBorderColor(brls::TRANSPARENT);
                            button->setBorderThickness(0.0f);
                        }
                    }
                }
            }
        }
    }
}

void ModsListView::updateUI() { this->updateModTiles(); }
}  // namespace front
