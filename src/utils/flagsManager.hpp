#pragma once

#include <atomic>

namespace utils {

class FlagManager {
   public:
   private:
    std::atomic<uint32_t> statusFlags{0};

   public:
    void setFlags(uint32_t flags) noexcept;

    void clearFlags(uint32_t flags) noexcept;
    void clearAll() noexcept;

    bool areFlagsSet(uint32_t flags) const noexcept;
    bool isAnyFlagSet(uint32_t flags) const noexcept;
};

}  // namespace utils
