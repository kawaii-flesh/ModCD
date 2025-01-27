#pragma once

#include <borealis.hpp>
#include <core/modInfo.hpp>

namespace front {
class ModTileView : public brls::Box {
   public:
    ModTileView(const core::ModInfo &modInfo);
};
}  // namespace front
