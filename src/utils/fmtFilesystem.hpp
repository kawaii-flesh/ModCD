#pragma once

#include <fmt/core.h>

#include <filesystem>

template <>
struct fmt::formatter<std::filesystem::path> : fmt::formatter<std::string> {
    template <typename FormatContext>
    auto format(const std::filesystem::path& path, FormatContext& ctx) {
        return fmt::formatter<std::string>::format(path.string(), ctx);
    }
};
