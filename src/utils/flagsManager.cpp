#include "flagsManager.hpp"

namespace utils {
void FlagManager::setFlags(uint32_t flags) noexcept { statusFlags.fetch_or(flags, std::memory_order_relaxed); }

void FlagManager::clearFlags(uint32_t flags) noexcept { statusFlags.fetch_and(~flags, std::memory_order_relaxed); }

void FlagManager::clearAll() noexcept { statusFlags.store(0, std::memory_order_relaxed); }

bool FlagManager::areFlagsSet(uint32_t flags) const noexcept {
    return (statusFlags.load(std::memory_order_relaxed) & flags) == flags;
}

bool FlagManager::isAnyFlagSet(uint32_t flags) const noexcept {
    return (statusFlags.load(std::memory_order_relaxed) & flags) != 0;
}
}  // namespace utils
