#pragma once

#include <app/modCD.hpp>
#include <borealis.hpp>
#include <front/view/modsListView.hpp>

namespace front {
class ModsListActivity : public brls::Activity {
   private:
    app::ModCD &modCD;
    const core::Game &game;
    ModsListView *modsRep;

   public:
    ModsListActivity(app::ModCD &aModCD, const core::Game &aGame) noexcept;

    ~ModsListActivity();

    brls::View *createContentView() override;
    void onContentAvailable() override;
};
}  // namespace front
