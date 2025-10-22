#include "modTileView.hpp"

#include <utils/localization.hpp>

namespace front {
ModTileView::ModTileView(const core::ModInfo &modInfo) : brls::Box(brls::Axis::COLUMN), modInfo(modInfo) {
    brls::Label *name =
        this->createLabel(utils::Localization::getInterpolated("ModTileView.Name", {{"name", modInfo.name}}));

    brls::Label *type =
        this->createLabel(utils::Localization::getInterpolated("ModTileView.Type", {{"type", modInfo.type}}));

    brls::Label *author =
        this->createLabel(utils::Localization::getInterpolated("ModTileView.Author", {{"author", modInfo.author}}));

    this->addView(name);
    this->addView(type);
    this->addView(author);

    if (!modInfo.description.empty()) {
        brls::Label *description = new brls::Label();
        description->setText(
            utils::Localization::getInterpolated("ModTileView.Description", {{"description", modInfo.description}}));
        description->setMargins(5.0f, 0.0f, 5.0f, 0.0f);
        this->addView(description);
    }
}

brls::Label *ModTileView::createLabel(const std::string &text) {
    auto label = new brls::Label();
    label->setText(text);
    label->setMargins(5.0f, 0.0f, 5.0f, 0.0f);
    return label;
}

}  // namespace front
