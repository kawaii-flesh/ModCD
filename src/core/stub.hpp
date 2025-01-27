#pragma once
#include <core/game.hpp>
#include <cstdint>
#include <list>
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace core {

class GameStub {
   public:
    const std::string titleId;
    const uint64_t version;

   public:
    GameStub(std::string &&aTitleId, uint64_t aVersion) noexcept;
    Game toGame() const;
    static GameStub fromJson(const nlohmann::json &j);
};

class Stub {
   public:
    std::list<GameStub> games;

   public:
    static Stub fromJson(const nlohmann::json &j);
};

}  // namespace core
