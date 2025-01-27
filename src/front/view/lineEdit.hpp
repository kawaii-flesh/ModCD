#pragma once

#include <borealis.hpp>
#include <functional>

namespace front {
class LineEdit : public brls::Button {
   private:
    brls::Label *lbl;
    std::string currentText;
    const std::string emptyTextPlaceholder;
    const std::size_t maxLength;
    std::function<void(const std::string &)> onTextChanged;

   private:
    std::string openForText();

   public:
    LineEdit(const std::string &aInitText, std::function<void(const std::string &)> aOnTextChanged,
             const float fontSize = 24.0f, const std::size_t aMaxLength = 50);

    bool onClick(brls::View *) noexcept;
    void setCurrentText(const std::string &text) noexcept;
    void setDisplayedText(const std::string &text) noexcept;
};

}  // namespace front
