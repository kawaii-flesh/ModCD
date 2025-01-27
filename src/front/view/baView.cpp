#include "baView.hpp"

#include <algorithm>
#include <front/styles/colors.hpp>
#include <utils/utils.hpp>

namespace {
constexpr int axesMult = 20;
}

namespace front {
void BAView::draw(NVGcontext *vg, float x, float y, float width, float height, brls::Style style,
                  brls::FrameContext *ctx) {
    if (this->scrollable) {
        brls::ControllerState state;
        brls::Application::getPlatform()->getInputManager()->updateUnifiedControllerState(&state);

        float delta = state.axes[brls::LEFT_X];
        if (delta == 0.0f) {
            delta = state.axes[brls::RIGHT_X];
        }
        delta *= axesMult;

        resizeImages(-round(delta));
    }

    brls::Box::draw(vg, x, y, width, height, style, ctx);
}

brls::View *BAView::getDefaultFocus() { return this->scrollable ? this : nullptr; }

void BAView::onFocusGained() {}

BAView::BAView(const std::filesystem::path &beforeImagePath, const std::filesystem::path &afterImagePath,
               float aMaxWidth, float aMaxHeight, bool aScrollable)
    : brls::Box(brls::Axis::ROW), maxWidth(aMaxWidth), maxHeight(aMaxHeight), scrollable(aScrollable) {
    constexpr float lineThick = 2.0f;

    this->beforeImage = new brls::Image();
    this->beforeImage->setScalingType(brls::ImageScalingType::CROP_RIGHT);
    this->beforeImage->setLineRight(lineThick);
    this->beforeImage->setLineColor(MCDGrey);
    this->beforeImage->setImageFromFile(beforeImagePath);
    this->addView(this->beforeImage);

    this->afterImage = new brls::Image();
    this->afterImage->setScalingType(brls::ImageScalingType::CROP_LEFT);
    this->afterImage->setLineLeft(lineThick);
    this->afterImage->setLineColor(MCDGrey);
    this->afterImage->setImageFromFile(afterImagePath);
    this->addView(this->afterImage);

    resetSize();

    if (scrollable) {
        this->addGestureRecognizer(new brls::ScrollGestureRecognizer(
            [this](brls::PanGestureStatus status, brls::Sound *sound) {
                if (status.state == brls::GestureState::START || status.state == brls::GestureState::STAY ||
                    status.state == brls::GestureState::END) {
                    this->resizeImages(status.delta.x);
                }
            },
            brls::PanAxis::HORIZONTAL));
    }
}

void BAView::resizeImages(float delta) {
    float newWidth = std::clamp(this->beforeImage->getWidth() - delta, 0.0f, this->maxWidth);

    this->beforeImage->setWidth(newWidth);
    this->afterImage->setWidth(this->maxWidth - newWidth);
}

void BAView::resetSize() {
    this->beforeImage->setWidth(this->maxWidth / 2.0f);
    this->afterImage->setWidth(this->maxWidth / 2.0f);
    this->afterImage->setHeight(this->maxHeight);
}
}  // namespace front
