#pragma once

#include <app/modCD.hpp>
#include <borealis.hpp>
#include <core/mergedInfo.hpp>

namespace front {
class ModsListView : public brls::Box {
   private:
    app::ModCD &modCD;
    const core::Game &game;
    brls::Box *scrollingContent;

   public:
    ModsListView(app::ModCD &aModCD, const core::Game &aGame);

    void addRows(std::list<core::ModInfo> &modInfos);
};
}  // namespace front
