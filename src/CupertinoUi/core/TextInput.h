#pragma once

#include "Color.h"
#include "Typography.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <span>
#include <string>
#include <string_view>

// Text editing drawn with Cupertino typography, shared by text, secure and search fields and the text editor: focus,
// caret, selection, word and line moves, clipboard, undo, horizontal scrolling of a line or wrapped lines.
namespace Cupertino::TextInput {
    struct Style {
        Font font = Font::Style(TextStyle::Body);
        Rgba color;
        TextAlignment alignment = TextAlignment::Leading;
        // Shown while the text is empty, also while editing.
        std::string_view placeholder;
        // One bullet per character, as in SecureField.
        bool secure = false;
        // Lines wrapped at the rect's width from its top, as NSTextView: Return inserts a line, the arrows move between
        // lines, and the scrolling window keeps the insertion point in view.
        bool multiline = false;
        // Typing at the end of a line completes it to the first of these that begins with the text, case aside: the rest
        // follows the caret, selected, so typing on replaces it (NSComboBox with completes).
        std::span<const char* const> completions;
    };

    struct Result {
        bool changed = false;
        bool focused = false;
        // Return was pressed in a single line; the field keeps focus with its text selected, as NSTextField does.
        bool submitted = false;
    };

    // Edits text inside rect (pixels): one line centered vertically, or wrapped lines from the top. A click focuses the
    // field and places the caret; a click elsewhere, Escape or Tab ends editing.
    Result Edit(ImGuiID id, const ImRect& rect, std::string* text, const Style& style);

    // The height in pixels of text wrapped at width pixels, as a multiline edit lays it out: a line at least.
    float TextHeight(const Font& font, std::string_view text, float width);
} // namespace Cupertino::TextInput
