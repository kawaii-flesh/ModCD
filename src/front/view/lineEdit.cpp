#include "lineEdit.hpp"

namespace front {
LineEdit::LineEdit(const std::string &aEmptyTextPlaceholder, std::function<void(const std::string &)> aOnTextChanged,
                   const float fontSize, const std::size_t aMaxLength)
    : emptyTextPlaceholder(aEmptyTextPlaceholder), onTextChanged(aOnTextChanged), maxLength(aMaxLength) {
    this->lbl = new brls::Label();
    this->lbl->setFontSize(fontSize);
    this->lbl->setText(aEmptyTextPlaceholder);
    this->addView(this->lbl);
    this->setPadding(fontSize / 2, fontSize, fontSize / 2, fontSize);
    this->registerClickAction(std::bind(&LineEdit::onClick, this, std::placeholders::_1));
}

std::string LineEdit::openForText() {
    brls::Application::getImeManager()->openForText(
        [&currentText = this->currentText](const std::string &text) { currentText = text; }, "", "", this->maxLength,
        this->currentText, brls::KeyboardKeyDisableBitmask::KEYBOARD_DISABLE_NONE);
    this->lbl->setText(this->currentText);
    return this->currentText;
}

bool LineEdit::onClick(brls::View *) noexcept {
    this->onTextChanged(this->openForText());
    return true;
}

void LineEdit::setCurrentText(const std::string &text) noexcept {
    this->currentText = text;
    if (text.empty()) {
        lbl->setText(this->emptyTextPlaceholder);
    } else {
        lbl->setText(text);
    }
}

void LineEdit::setDisplayedText(const std::string &text) noexcept { lbl->setText(text); }
}  // namespace front
