#pragma once

#include <borealis.hpp>
#include <core/game.hpp>

namespace front {
class GameTileView : public brls::Button {
   public:
    const core::Game& game;
    GameTileView(const core::Game& aGame);
};
}  // namespace front
