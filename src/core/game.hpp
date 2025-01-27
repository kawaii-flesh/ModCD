#pragma once

#include <cstdint>
#include <memory>
#include <string>

constexpr std::size_t GAME_ICON_SIZE = 0x20000;

namespace core {
class Game {
   public:
    const std::string name;
    const uint64_t titleId;
    std::unique_ptr<uint8_t[]> icon;
    const uint64_t version;

   public:
    Game(std::string &&aName, const uint64_t aTitleId, uint8_t aIcon[GAME_ICON_SIZE], const uint64_t aVersion) noexcept;

    std::string titleIDToString() const noexcept;
};

}  // namespace core
