#include "loadingView.hpp"

#include <utils/localization.hpp>

namespace front {
LoadingView::LoadingView() : brls::Box(brls::Axis::COLUMN) {
    this->setJustifyContent(brls::JustifyContent::CENTER);
    this->setAlignItems(brls::AlignItems::CENTER);
    this->setMinHeightPercentage(100.0f);
    this->setMinWidthPercentage(100.0f);

    brls::Image* logo = new brls::Image();
    logo->setImageFromRes("icon/icon.jpg");
    logo->setScalingType(brls::ImageScalingType::CENTER);
    logo->setWidth(256);
    logo->setHeight(256);
    this->addView(logo);

    brls::Label* waitText = new brls::Label();
    waitText->setText("\n" + utils::Localization::get("LoadingView.LoadingLabel"));
    waitText->setFontSize(32.0f);
    this->addView(waitText);
}
}  // namespace front
