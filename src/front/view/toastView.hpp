#pragma once

#include <borealis.hpp>
#include <front/styles/colors.hpp>
#include <functional>
#include <memory>

namespace front {

class CallbackTimer;

enum class ToastColor { OK, ERROR, INFO, WARNING };

class ToastView : public brls::Button {
   private:
    brls::Box *root;
    brls::Label *label;
    std::unique_ptr<CallbackTimer> timer;
    brls::Point centerPoint;

   public:
    ToastView(brls::Box *aRoot, const brls::Point &aCenterPoint, float fontSize = 24);
    ~ToastView();

    void start(const std::string &text, const ToastColor color, brls::Time duration = 2000);

    void onTimeout();
};

}  // namespace front
