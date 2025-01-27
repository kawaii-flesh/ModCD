#pragma once

#include <borealis.hpp>
#include <filesystem>

namespace front {

class BAView : public brls::Box {
   private:
    void draw(NVGcontext *vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext *ctx) override;
    brls::View *getDefaultFocus() override;
    void onFocusGained() override;

   private:
    brls::Image *beforeImage;
    brls::Image *afterImage;
    float maxWidth;
    float maxHeight;
    bool scrollable;

   public:
    BAView(const std::filesystem::path &beforeImagePath, const std::filesystem::path &afterImagePath, float aMaxWidth,
           float aMaxHeight, bool scrollable);

    void resizeImages(float delta);
    void resetSize();
};
}  // namespace front
