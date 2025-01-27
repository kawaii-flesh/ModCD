#pragma once

#include <app/modCD.hpp>
#include <borealis.hpp>
#include <front/view/modView.hpp>

namespace front {
class ModActivity : public brls::Activity {
   private:
    ModView *modView;
    app::ModCD &modCD;

   public:
    ModActivity(app::ModCD &aModCD) noexcept;

    ~ModActivity();

    brls::View *createContentView() override;
    void onContentAvailable() override;
};
}  // namespace front
