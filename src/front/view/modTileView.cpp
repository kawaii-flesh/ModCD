#include "modTileView.hpp"

#include <utils/localization.hpp>

namespace front {
ModTileView::ModTileView(const core::ModInfo &modInfo) : brls::Box(brls::Axis::COLUMN) {
    brls::Label *name = new brls::Label();
    name->setText(utils::Localization::getInterpolated("ModTileView.Name", {{"name", modInfo.name}}));
    name->setMargins(5.0f, 0.0f, 5.0f, 0.0f);

    brls::Label *type = new brls::Label();
    type->setText(utils::Localization::getInterpolated("ModTileView.Type", {{"type", modInfo.type}}));
    type->setMargins(5.0f, 0.0f, 5.0f, 0.0f);

    brls::Label *author = new brls::Label();
    author->setText(utils::Localization::getInterpolated("ModTileView.Author", {{"author", modInfo.author}}));
    author->setMargins(5.0f, 0.0f, 5.0f, 0.0f);

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
}  // namespace front
