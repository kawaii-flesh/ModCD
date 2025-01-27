#include "stub.hpp"

#include <nlohmann/json.hpp>

namespace core {

GameStub::GameStub(std::string &&aTitleId, uint64_t aVersion) noexcept
    : titleId(std::move(aTitleId)), version(aVersion) {}

Game GameStub::toGame() const {
    return Game("Stub name", std::stoull(this->titleId, nullptr, 16), nullptr, this->version);
}
GameStub GameStub::fromJson(const nlohmann::json &j) { return GameStub(j.at("titleId"), j.at("version")); }

Stub Stub::fromJson(const nlohmann::json &j) {
    Stub stub;
    for (const auto &gameStubJson : j.at("games")) {
        stub.games.push_back(GameStub::fromJson(gameStubJson));
    }
    return stub;
}
}  // namespace core
