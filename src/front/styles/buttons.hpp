#pragma once

#include <borealis.hpp>

namespace front {
inline const brls::ButtonStyle MCD_BUTTONSTYLE_PRIMARY = {
    .shadowType = brls::ShadowType::GENERIC,
    .hideHighlightBackground = true,

    .highlightPadding = "brls/button/primary_highlight_padding",
    .borderThickness = "",

    .enabledBackgroundColor = "brls/button/primary_enabled_background",
    .enabledLabelColor = "brls/button/primary_enabled_text",
    .enabledBorderColor = "",

    .disabledBackgroundColor = "brls/button/primary_disabled_background",
    .disabledLabelColor = "brls/button/default_enabled_text",
    .disabledBorderColor = "",
};

}  // namespace front
