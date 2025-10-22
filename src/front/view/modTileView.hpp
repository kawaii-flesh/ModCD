#pragma once

#include <borealis.hpp>
#include <core/modInfo.hpp>

namespace front {
class ModTileView : public brls::Box {
   private:
   brls::Label *createLabel(const std::string &text);
   public:
    const core::ModInfo &modInfo;
    ModTileView(const core::ModInfo &modInfo);
};
}  // namespace front
