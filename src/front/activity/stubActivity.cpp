#include "stubActivity.hpp"

namespace front {
StubActivity::StubActivity(std::string&& message, const NVGcolor textColor) {
    this->box = new brls::Box(brls::Axis::COLUMN);
    this->box->setJustifyContent(brls::JustifyContent::CENTER);
    this->box->setAlignItems(brls::AlignItems::CENTER);

    this->messageLbl = new brls::Label();
    this->messageLbl->setText(message);
    this->messageLbl->setFontSize(32.0f);
    this->messageLbl->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    this->messageLbl->setVerticalAlign(brls::VerticalAlign::CENTER);
    this->messageLbl->setTextColor(textColor);

    this->box->addView(messageLbl);
    MODCD_LOG_DEBUG("[{}]: created", __PRETTY_FUNCTION__);
}
StubActivity::~StubActivity() { MODCD_LOG_DEBUG("[{}]: deleted", __PRETTY_FUNCTION__); }
brls::View* StubActivity::createContentView() { return this->box; }
}  // namespace front
