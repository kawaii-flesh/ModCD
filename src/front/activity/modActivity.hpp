#pragma once

#include <app/modCD.hpp>
#include <borealis.hpp>
#include <front/view/modView.hpp>
#include <front/interfaces/iUpdatable.hpp>

namespace front {
class ModActivity : public brls::Activity {
   private:
    ModView *modView;
    IUpdatable *parentUpdatable;
    app::ModCD &modCD;

   public:
    ModActivity(app::ModCD &aModCD, IUpdatable *parentUpdatable) noexcept;

    ~ModActivity();

    brls::View *createContentView() override;
    void onContentAvailable() override;
};
}  // namespace front
