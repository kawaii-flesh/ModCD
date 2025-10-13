#include "gamesListView.hpp"

#include <app/repositoryProviderOnline.hpp>
#include <app/updater.hpp>
#include <front/activity/modsListActivity.hpp>
#include <front/activity/stubActivity.hpp>
#include <front/asyncLock.hpp>
#include <front/styles/colors.hpp>
#include <front/view/gameTileView.hpp>
#include <front/view/lineEdit.hpp>
#include <front/view/loadingView.hpp>
#include <front/view/toastView.hpp>
#include <utils/localization.hpp>
#include <utils/utils.hpp>

namespace front {

GamesListView::GamesListView(app::ModCD &aModCD)
    : brls::Box(brls::Axis::COLUMN),
      modCD(aModCD),
      initTextForLineEdit(utils::Localization::get("GamesListView.SearchLineEdit.Placeholder")),
      totalUpdateSize(0) {
    this->updateDS.toStop = false;
    this->updateDS.totalSize = 0;
    this->updateDS.alreadyDownloaded = 0;
    this->loadingView = new LoadingView();
    this->addView(this->loadingView);
    brls::async([this]() {
        brls::sync([this]() {
            if (prepareGames()) {
                this->lineEdit = new LineEdit(this->initTextForLineEdit, std::bind(&GamesListView::onSearchTextChanged,
                                                                                   this, std::placeholders::_1));
                this->lineEdit->setBorderThickness(4.0f);
                this->lineEdit->setBorderColor(MCDBorderColor);
                this->lineEdit->setMinWidthPercentage(90.0f);
                this->lineEdit->setMaxWidthPercentage(90.0f);
                this->lineEdit->setMargins(20.0f, 0.0f, 20.0f, 0.0f);
                this->lineEdit->setFocusable(false);
                this->addView(this->lineEdit);

                this->list = new brls::ScrollingFrame();
                this->listContent = new brls::Box(brls::Axis::COLUMN);
                this->addIconsRow();
                this->list->setContentView(this->listContent);
                this->list->setMinWidthPercentage(100.0f);
                this->list->setMaxWidthPercentage(100.0f);
                this->list->setMinHeightPercentage(86.0f);

                this->setAlignItems(brls::AlignItems::CENTER);
                this->removeView(this->loadingView);
                this->addView(this->list);

                this->registerAction("", brls::ControllerButton::BUTTON_X,
                                     std::bind(&LineEdit::onClick, this->lineEdit, std::placeholders::_1));
                this->registerAction("", brls::ControllerButton::BUTTON_B, [this](brls::View *) {
                    this->onSearchTextChanged("");
                    this->lineEdit->setCurrentText("");
                    return true;
                });
                this->toast = std::make_unique<ToastView>(
                    this, brls::Point(1280.0f / 2, round(720.0f - (720.0f / 3 / 2)) + 50.0f));

                brls::Application::giveFocus(this->list->getChildren()[0]);
                if (!this->modCD.isOnlineMode()) {
                    this->offlineDialog =
                        new brls::Dialog(utils::Localization::get("GamesListView.OfflineDialog.Message"));
                    this->offlineDialog->addButton(utils::Localization::get("ModView.OfflineDialog.OkButton"), []() {});
                    this->offlineDialog->open();
                } else {
                    try {
                        checkUpdate();
                    } catch (const utils::CURLException &curlException) {
                        MODCD_LOG_ERROR("[{}]: Check update timeout", __PRETTY_FUNCTION__);
                    }
                }
                this->toast->start(utils::Localization::getInterpolated(
                                       "GamesListView.Toast.FoundedGames",
                                       {{"count", std::to_string(this->modCD.supportedGames.size())}}),
                                   ToastColor::OK);
            }
        });
    });
}

bool GamesListView::prepareGames() {
    try {
        this->modCD.init();
        if (this->modCD.supportedGames.empty()) {
            bool isOnlineMode = this->modCD.isOnlineMode();
            brls::Application::pushActivity(
                new StubActivity(utils::Localization::get(isOnlineMode ? "Errors.NoSupportedModsOrGames"
                                                                       : "Errors.NoSupportedModsOrGamesOffline"),
                                 isOnlineMode ? MCDWhite : MCDYellow),
                brls::TransitionAnimation::NONE);
            return false;
        }
        return true;
    } catch (const app::RepositoryAccessException &exception) {
        brls::Application::pushActivity(
            new StubActivity(utils::Localization::getInterpolated("Errors.RepositoryAccessException",
                                                                  {{"url", this->modCD.getConfig().repositoryUrl}})),
            brls::TransitionAnimation::NONE);
        return false;
    }
}

void GamesListView::checkUpdate() {
    const app::ReleaseData releaseData = app::Updater::getLatestReleaseInfo(this->modCD.httpRequester);
    if (!releaseData.url.empty()) {
        this->totalUpdateSize = releaseData.fileSize;
        this->updateDialog = new brls::Dialog(utils::Localization::getInterpolated(
            "GamesListView.UpdateDialog.UpdateMessage",
            {{"currentVersion", releaseData.currentVersion}, {"newVersion", releaseData.newVersion}}));
        this->updateDialog->addButton(
            utils::Localization::get("GamesListView.UpdateDialog.UpdateButton"), [this, releaseData]() {
                brls::Application::setGlobalQuit(false);
                for (brls::Button *title : this->gameTiles) {
                    title->setState(brls::ButtonState::DISABLED);
                }
                this->toast->start(utils::Localization::get("GamesListView.Toast.DownloadIsStarting"), ToastColor::OK);
                brls::async([this, releaseData] {
                    const bool result =
                        app::Updater::update(this->modCD.httpRequester, this->modCD.getModCDNroPath(), releaseData.url,
                                             this->modCD.getModCDDirPath(), &this->updateDS);
                    if (result) {
                        this->toast->start(utils::Localization::get("GamesListView.Toast.UpdateSuccessful"),
                                           ToastColor::OK);
                        brls::Application::quit();
                    } else {
                        brls::Application::setGlobalQuit(true);
                        this->toast->start(utils::Localization::get("GamesListView.Toast.UpdateError"),
                                           ToastColor::ERROR);
                        for (brls::Button *title : this->gameTiles) {
                            title->setState(brls::ButtonState::ENABLED);
                        }
                    }
                });
            });
        this->updateDialog->addButton(utils::Localization::get("GamesListView.UpdateDialog.SkipButton"), []() {});
        this->updateDialog->open();
    }
}

void GamesListView::onSearchTextChanged(const std::string &newText) {
    for (GameTileView *gameTile : this->gameTiles) {
        bool isVisible = true;
        if (!newText.empty()) {
            std::wstring gameName = utils::to_wstring(gameTile->game.name);
            std::wstring searchString = utils::to_wstring(newText);

            std::transform(gameName.begin(), gameName.end(), gameName.begin(),
                           [](wchar_t c) { return std::towlower(c); });
            std::transform(searchString.begin(), searchString.end(), searchString.begin(),
                           [](wchar_t c) { return std::towlower(c); });
            if (gameName.find(searchString) == std::wstring::npos) {
                isVisible = false;
            }
        }

        gameTile->setVisibility(isVisible ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
        gameTile->getParent()->removeView(gameTile, false);
    }

    this->resortIconsRow();

    this->lineEdit->setCurrentText(newText);
    if (!newText.empty()) {
        this->lineEdit->setDisplayedText(utils::Localization::getInterpolated(
            "GamesListView.SearchLineEdit.PostfixLineEditText", {{"prefix", newText}}));
    }
}

void GamesListView::addIconsRow() {
    brls::Box *row = nullptr;
    int n = 3;
    int count = 0;

    for (const core::Game &game : this->modCD.supportedGames) {
        if (count % n == 0) {
            row = new brls::Box(brls::Axis::ROW);
            this->listContent->addView(row);
            this->rows.push_back(row);
        }

        GameTileView *gameTileView = new GameTileView(game);
        this->gameTiles.push_back(gameTileView);
        gameTileView->setMargins(20.0f, 25.0f, 10.0f, 25.0f);

        gameTileView->registerClickAction([this, &game](brls::View *view) {
            MODCD_LOG_DEBUG("Icon was clicked: {}", game.name);
            this->modCD.setCurrentTitleId(game.titleIDToString());
            brls::Application::pushActivity(new ModsListActivity(this->modCD, game), brls::TransitionAnimation::NONE);
            return true;
        });

        row->addView(gameTileView);
        row->setAlignItems(brls::AlignItems::FLEX_START);

        count++;
    }
}

void GamesListView::resortIconsRow() {
    int n = 3;
    int count = 1;
    int row = 0;
    bool focusWasGiven = false;

    for (GameTileView *gameTile : this->gameTiles) {
        if (gameTile->getVisibility() == brls::Visibility::VISIBLE) {
            this->rows[row]->addView(gameTile);
            if (count++ % n == 0) {
                ++row;
            }
            if (!focusWasGiven) {
                brls::Application::giveFocus(gameTile);
                focusWasGiven = true;
            }
        }
    }
    if (count == 1) {
        this->lineEdit->setFocusable(true);
        brls::Application::giveFocus(this->lineEdit);
    } else {
        this->lineEdit->setFocusable(false);
    }

    this->list->setContentOffsetY(0.0f, false);
}

void GamesListView::draw(NVGcontext *vg, float x, float y, float width, float height, brls::Style style,
                         brls::FrameContext *ctx) {
    if (this->updateDS.alreadyDownloaded != 0) {
        const std::string progress = utils::calculateProgress(this->totalUpdateSize, this->updateDS.alreadyDownloaded);
        this->toast->start(progress, ToastColor::OK, 1000);
        if (this->updateDS.alreadyDownloaded == this->totalUpdateSize) {
            this->totalUpdateSize = 0;
            this->updateDS.alreadyDownloaded = 0;
        }
    }

    brls::Box::draw(vg, x, y, width, height, style, ctx);
}

}  // namespace front
