#pragma once
#include <borealis.hpp>
#include <front/styles/colors.hpp>
#include <string>
#include <utils/utils.hpp>

namespace front {
class StubActivity : public brls::Activity {
   private:
    brls::Box *box;
    brls::Label *messageLbl;

   public:
    StubActivity(std::string &&message, const NVGcolor textColor = MCDWhite);
    ~StubActivity();
    brls::View *createContentView() override;
};
}  // namespace front
