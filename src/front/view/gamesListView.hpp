#pragma once

#include <app/modCD.hpp>
#include <borealis.hpp>
#include <core/game.hpp>
#include <front/view/gameTileView.hpp>
#include <front/view/lineEdit.hpp>
#include <front/view/loadingView.hpp>
#include <front/view/modTileView.hpp>
#include <front/view/modsListView.hpp>
#include <front/view/toastView.hpp>
#include <string>

namespace front {
class GamesListView : public brls::Box {
   private:
    app::ModCD &modCD;
    brls::ScrollingFrame *list;
    brls::Box *listContent;
    LineEdit *lineEdit;
    LoadingView *loadingView;
    std::vector<brls::Box *> rows;
    std::list<GameTileView *> gameTiles;
    std::unique_ptr<ToastView> toast;
    brls::Dialog *offlineDialog;
    brls::Dialog *updateDialog;
    utils::DownloadState updateDS;
    long totalUpdateSize;
    const std::string initTextForLineEdit;

   private:
    void onSearchTextChanged(const std::string &newText);
    void addIconsRow();
    void resortIconsRow();
    void draw(NVGcontext *vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext *ctx) override;
    bool prepareGames();
    void checkUpdate();

   public:
    GamesListView(app::ModCD &aModCD);
};
}  // namespace front
