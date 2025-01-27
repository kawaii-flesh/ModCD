#pragma once

#include <borealis.hpp>
#include <filesystem>

namespace front {
class ScreenshotsView : public brls::ScrollingFrame {
   private:
    brls::Box* root;
    const std::list<std::filesystem::path> screenshotsFiles;

    void addIconsRows();

   public:
    ScreenshotsView(std::list<std::filesystem::path>&& aScreenshotsFiles) noexcept;
};
}  // namespace front
