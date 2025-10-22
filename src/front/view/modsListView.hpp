#pragma once

#include <app/modCD.hpp>
#include <borealis.hpp>
#include <core/mergedInfo.hpp>
#include <front/interfaces/iUpdatable.hpp>

namespace front {
class ModsListView : public brls::Box, public IUpdatable {
   private:
    app::ModCD &modCD;
    const core::Game &game;
    brls::Box *scrollingContent;
    void updateModTiles();

   public:
    ModsListView(app::ModCD &aModCD, const core::Game &aGame);

    void addRows(std::list<core::ModInfo> &modInfos);
    void updateUI() override;
};
}  // namespace front
