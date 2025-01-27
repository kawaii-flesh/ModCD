#include "game.hpp"

#include <iomanip>

namespace core {
Game::Game(std::string &&aName, const uint64_t aTitleId, uint8_t aIcon[GAME_ICON_SIZE],
           const uint64_t aVersion) noexcept
    : name(std::move(aName)), titleId(aTitleId), version(aVersion) {
    if (aIcon) {
        icon.reset(new uint8_t[GAME_ICON_SIZE]);
        std::copy(aIcon, aIcon + GAME_ICON_SIZE, icon.get());
    }
}

std::string Game::titleIDToString() const noexcept {
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setw(16) << std::setfill('0') << this->titleId;
    return oss.str();
}

}  // namespace core
