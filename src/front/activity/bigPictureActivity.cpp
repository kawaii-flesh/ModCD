#include "bigPictureActivity.hpp"

#include <utils/constants.hpp>
#include <utils/utils.hpp>

namespace {
class FocusableImage : public brls::Image {
   public:
    void onFocusGained() override {}
};
}  // namespace

namespace front {

void BigPictureActivity::prepare(const std::filesystem::path &path) {
    this->root->clearViews();
    bool isBA = (path.string().find(modcd_constants::DELIMITER) != std::string::npos);
    if (isBA) {
        std::pair<std::filesystem::path, std::filesystem::path> ba = utils::splitOnTwo(path);
        BAView *baView =
            new BAView(ba.first, ba.second, modcd_constants::SCREEN_WIDTH, modcd_constants::SCREEN_HEIGHT, true);
        baView->resetSize();
        this->root->addView(baView);
        brls::Application::giveFocus(baView);
    } else {
        FocusableImage *image = new FocusableImage();
        image->setImageFromFile(path);
        image->setFocusable(true);
        this->root->addView(image);
        brls::Application::giveFocus(this->root);
    }
}

BigPictureActivity::BigPictureActivity(const std::list<std::filesystem::path> &aImagesPath, int index)
    : imagesPath(aImagesPath) {
    this->baseIndex = std::next(imagesPath.cbegin(), index);
    MODCD_LOG_DEBUG("[{}]: created - scrs: {}", __PRETTY_FUNCTION__, *baseIndex);

    this->root = new brls::Box(brls::Axis::COLUMN);
    this->root->setAlignItems(brls::AlignItems::CENTER);
    this->root->setJustifyContent(brls::JustifyContent::CENTER);

    this->prepare(*this->baseIndex);
}

BigPictureActivity::~BigPictureActivity() {
    MODCD_LOG_DEBUG("[{}]: deleted - scrs: {}", __PRETTY_FUNCTION__, *this->baseIndex);
}

brls::View *BigPictureActivity::createContentView() { return this->root; }

void BigPictureActivity::onContentAvailable() {
    this->registerAction(
        "Go to screenshots", brls::ControllerButton::BUTTON_B,
        [](brls::View *view) {
            brls::Application::popActivity(brls::TransitionAnimation::NONE);
            return true;
        },
        true);

    auto leftAction = [this](brls::View *) {
        this->baseIndex =
            std::prev(this->baseIndex == this->imagesPath.begin() ? this->imagesPath.end() : this->baseIndex);
        prepare(*this->baseIndex);
        return true;
    };

    auto rightAction = [this](brls::View *) {
        this->baseIndex = std::next(this->baseIndex);
        if (this->baseIndex == this->imagesPath.end()) {
            this->baseIndex = this->imagesPath.begin();
        }
        prepare(*this->baseIndex);
        return true;
    };

    this->registerAction("", brls::ControllerButton::BUTTON_LT, leftAction);
    this->registerAction("", brls::ControllerButton::BUTTON_LB, leftAction);
    this->registerAction("", brls::ControllerButton::BUTTON_LEFT, leftAction);

    this->registerAction("", brls::ControllerButton::BUTTON_RT, rightAction);
    this->registerAction("", brls::ControllerButton::BUTTON_RB, rightAction);
    this->registerAction("", brls::ControllerButton::BUTTON_RIGHT, rightAction);
}
}  // namespace front
