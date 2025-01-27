#include "screenshotsView.hpp"

#include <front/activity/bigPictureActivity.hpp>
#include <front/styles/colors.hpp>
#include <utils/constants.hpp>
#include <utils/utils.hpp>

namespace {
constexpr float DELIM = 2.5f;
constexpr float W_SIZE = modcd_constants::SCREEN_WIDTH / DELIM;
constexpr float H_SIZE = modcd_constants::SCREEN_HEIGHT / DELIM;

void setFullSize(brls::View *view) {
    view->setMinWidth(W_SIZE);
    view->setMaxWidth(W_SIZE);
    view->setMinHeight(H_SIZE);
    view->setMaxHeight(H_SIZE);
}
}  // namespace

namespace front {
ScreenshotsView::ScreenshotsView(std::list<std::filesystem::path> &&aScreenshotsFiles) noexcept
    : brls::ScrollingFrame(), screenshotsFiles(std::move(aScreenshotsFiles)) {
    this->setFocusable(false);
    this->root = new brls::Box(brls::Axis::COLUMN);

    this->addIconsRows();

    this->setContentView(root);
}

void ScreenshotsView::addIconsRows() {
    const int iconsPerRow = 2;
    brls::Box *row = nullptr;
    int iconCount = 0;
    for (const std::string &scr : this->screenshotsFiles) {
        if (iconCount % iconsPerRow == 0) {
            row = new brls::Box(brls::Axis::ROW);
            this->root->addView(row);
        }

        brls::Button *button = new brls::Button();
        if (iconCount == 0) {
            brls::Application::giveFocus(button);
        }
        setFullSize(button);
        button->setMargins(40.0f, 40.0f, 40.0f, 40.0f);
        button->setPadding(0.0f);
        if (scr.find(modcd_constants::DELIMITER) != std::string::npos) {
            std::pair<std::string, std::string> ba = utils::splitOnTwo(scr);

            auto view = new BAView(ba.first, ba.second, W_SIZE, H_SIZE, false);
            button->addView(view);
            button->setBorderThickness(8.0f);
            button->setBorderColor(MCDRed);
        } else {
            brls::Image *image = new brls::Image();
            image->setImageFromFile(scr);
            setFullSize(image);
            button->addView(image);
            button->setBorderThickness(8.0f);
            button->setBorderColor({0.5f, 0.5f, 0.5f, 1.0f});
        }

        button->registerClickAction([this, scr, iconCount](brls::View *view) {
            view->onFocusLost();
            MODCD_LOG_DEBUG("Icon was clicked: {}", scr);
            brls::Application::pushActivity(new BigPictureActivity(this->screenshotsFiles, iconCount),
                                            brls::TransitionAnimation::NONE);
            return true;
        });

        row->addView(button);
        row->setAlignItems(brls::AlignItems::FLEX_START);

        iconCount++;
    }
}

}  // namespace front
