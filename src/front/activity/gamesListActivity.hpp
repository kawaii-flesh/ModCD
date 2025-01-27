#pragma once

#include <app/modCD.hpp>
#include <borealis.hpp>
#include <front/view/gamesListView.hpp>

namespace front {
class GamesListActivity : public brls::Activity {
   private:
    app::ModCD &modCD;
    GamesListView *gamesListView;

   public:
    GamesListActivity(app::ModCD &aModCD) noexcept;

    ~GamesListActivity();

    brls::View *createContentView() override;
};
}  // namespace front
