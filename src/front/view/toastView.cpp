#include "toastView.hpp"

namespace front {

namespace {
const NVGcolor &getBorderColor(ToastColor color) {
    switch (color) {
        case ToastColor::INFO:
            return MCDGrey;
        case ToastColor::ERROR:
            return MCDRed;
        case ToastColor::WARNING:
            return MCDYellow;
        case ToastColor::OK:
            break;
    }
    return MCDGreen;
}
}  // namespace

class CallbackTimer : public brls::Timer {
   private:
    std::function<void()> onStopCallback;

   private:
    bool onUpdate(brls::Time delta) override {
        this->duration -= delta;
        if ((this->duration <= 0) && onStopCallback) {
            onStopCallback();
        }
        return true;
    }

   public:
    CallbackTimer(const std::function<void()> &aOnStopCallback) noexcept : onStopCallback(aOnStopCallback) {}
};

ToastView::ToastView(brls::Box *aRoot, const brls::Point &aCenterPoint, float fontSize)
    : root(aRoot),
      label(new brls::Label()),
      timer(std::make_unique<CallbackTimer>(std::bind(&ToastView::onTimeout, this))),
      centerPoint(aCenterPoint) {
    this->setFocusable(false);
    this->addView(this->label);
    this->label->setFontSize(fontSize);

    this->detach();
    this->root->addView(this);
    this->setVisibility(brls::Visibility::GONE);
    this->setBorderThickness(3.0f);

    this->registerClickAction([this](View *) {
        this->onTimeout();
        return true;
    });
}

ToastView::~ToastView() { this->root->removeView(this); }

void ToastView::start(const std::string &text, const ToastColor color, brls::Time duration) {
    this->label->setText(text);
    this->setDetachedPosition(this->centerPoint.x - round(this->getWidth() / 2.0f),
                              this->centerPoint.y - round(this->getHeight() / 2.0f));

    this->setBorderColor(getBorderColor(color));

    this->setVisibility(brls::Visibility::VISIBLE);
    this->timer->start(duration);
}

void ToastView::onTimeout() { this->setVisibility(brls::Visibility::GONE); }
}  // namespace front
