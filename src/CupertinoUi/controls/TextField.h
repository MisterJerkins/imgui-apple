#pragma once

#include "core/Typography.h"
#include "layout/ScrollView.h"

#include <functional>
#include <span>
#include <string>

namespace Cupertino {
    // SwiftUI's textFieldStyle outside forms.
    enum class TextFieldStyle {
        // AppKit's bezeled field.
        Automatic,
        // .plain: the text alone, without bezel or focus ring, lined up with the text around it.
        Plain,
    };

    struct TextFieldOptions {
        TextFieldStyle style = TextFieldStyle::Automatic;
        // Shown while empty. Outside a form the label is the placeholder, as in SwiftUI.
        const char* placeholder = nullptr;
        // Width in points; 0 takes the offered width, in a form row the trailing half of the row.
        float width = 0.0f;
        // Text alignment outside a form; a form row's field always aligns its text to the trailing edge.
        TextAlignment alignment = TextAlignment::Leading;
        // An x.circle.fill at the end of a field with text that clears it (the kit's text field; AppKit draws it on search
        // fields only).
        bool clearButton = false;
        // The accent ring around the focused field; without it the field shows focus by its insertion point alone, like
        // .focusEffectDisabled().
        bool focusRing = true;
        // .onSubmit: runs when Return is pressed in the field.
        std::function<void()> onSubmit;
    };

    // The field of a text, search or key field inside its 22 pt layout frame (pixels), and where its clear button stands.
    ImRect FieldInFrame(const ImRect& frame);
    ImRect FieldClearButtonFrame(const ImRect& field);
    // x.circle.fill in frame (pixels) at the end of a field with text, which a click clears: returns true when clicked.
    bool FieldClearButton(ImGuiID id, const ImRect& frame, Rgba color);

    // A single-line field bound to text, like SwiftUI's TextField: the bezeled field of AppKit, or in a form row the
    // label on the leading side and an outlined value box. Returns true when the text changes.
    bool TextField(const char* label, std::string* text, const TextFieldOptions& options = {});

    // The same field showing a bullet per character (SecureField).
    bool SecureField(const char* label, std::string* text, const TextFieldOptions& options = {});

    struct SearchFieldOptions {
        // Shown while empty; without one the visible label, or "Search".
        const char* placeholder = nullptr;
        // Width in points; 0 takes the offered width.
        float width = 0.0f;
        // searchSuggestions: shown in a menu under the field while text is typed; choosing one puts it in the field, as
        // searchCompletion. The app filters them by the text.
        std::span<const char* const> suggestions;
    };

    // A rounded search field with a magnifier and, while not empty, a clear button.
    bool SearchField(const char* label, std::string* text, const SearchFieldOptions& options = {});

    struct TextEditorOptions {
        Font font = Font::Style(TextStyle::Body);
        // NSScrollView's border around the editor; SwiftUI's TextEditor draws none.
        ScrollViewBorder border = ScrollViewBorder::None;
    };

    // A multi-line editor bound to text, like SwiftUI's TextEditor (an NSTextView in a scroll view): the text wraps
    // 5 pt in from the sides on the text background, Return inserts a line, and it scrolls with the overlay scroller.
    // It takes all the space it is offered. Returns true when the text changes.
    bool TextEditor(const char* label, std::string* text, const TextEditorOptions& options = {});
} // namespace Cupertino
