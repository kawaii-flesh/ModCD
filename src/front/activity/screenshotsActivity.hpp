#pragma once

#include <app/modCD.hpp>
#include <borealis.hpp>
#include <front/view/screenshotsView.hpp>

namespace front {
class ScreenshotsActivity : public brls::Activity {
   private:
    ScreenshotsView *screenshotsView;

   public:
    ScreenshotsActivity(std::list<std::filesystem::path> &&aScreenshotsFiles);
    ~ScreenshotsActivity();

    brls::View *createContentView() override;
    void onContentAvailable() override;
};
}  // namespace front
