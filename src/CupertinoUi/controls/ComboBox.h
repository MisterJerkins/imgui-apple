#pragma once

#include <initializer_list>
#include <span>
#include <string>

namespace Cupertino {
    struct ComboBoxOptions {
        // Width in points; 0 fits the longest item.
        float width = 0.0f;
        // Shown while the text is empty.
        const char* placeholder = nullptr;
    };

    // An editable pop-up like NSComboBox: a text field in a pop-up button's bezel whose indicator opens the items under
    // it; choosing one puts it in the field, and typing completes to the first item it begins. With a visible label it stands after the label, or is a form row in a form
    // section. Returns true when the text changes.
    bool ComboBox(const char* label, std::string* text, std::span<const char* const> items, const ComboBoxOptions& options = {});
    bool ComboBox(const char* label, std::string* text, std::initializer_list<const char*> items, const ComboBoxOptions& options = {});
} // namespace Cupertino
