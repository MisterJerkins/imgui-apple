#pragma once

#include "IconPlate.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <initializer_list>
#include <span>

namespace Cupertino {
    enum class PickerStyle {
        // A pop-up menu: borderless with an indicator in a Form row, a bezel pop-up button elsewhere.
        Automatic,
        Menu,
        RadioGroup,
        Segmented,
        // A 22 pt square button showing only the up and down chevrons, opening the same menu (the kit's Arrow Button).
        Arrows,
        // In a menu the options stand in it as their own section under the label; elsewhere a radio group. Any other
        // style in a menu is a submenu of the options titled with the label.
        Inline,
    };

    struct PickerOptions {
        PickerStyle style = PickerStyle::Automatic;
        // Fixed width in points; 0 sizes the control to its content.
        float width = 0.0f;
        // Secondary text under the label of a form row, like the second Text in a SwiftUI label, and a symbol before it in
        // its own color (a warning: exclamationmark.triangle.fill in yellow).
        const char* description = nullptr;
        unsigned descriptionSymbol = 0;
        Rgba descriptionSymbolColor;
        // A small icon before the label of a form row (Control Center's modules).
        Icon icon;
        // Radio buttons side by side on the trailing side of a form row, as .horizontalRadioGroupLayout().
        bool horizontal = false;
        // Paints the image in front of an item's title into frame (e.g. a color swatch), in the menu and in the
        // pop-up's value; itemImageSize is its size in points.
        void (*itemImage)(ImDrawList* draw, const ImRect& frame, int item) = nullptr;
        ImVec2 itemImageSize = ImVec2(24.0f, 12.0f);
        // The menu of a form pop-up keeps its selected title on the value even over the indicator, as the AppKit pop-up of
        // the Wi-Fi details sheet does (@2x); otherwise it stays left of the indicator, as in Battery (@2x).
        bool menuCoversIndicator = false;
        // Disables the pop-up of a form row but not its label and description, like a disabled control inside
        // LabeledContent (the password delay of Lock Screen while iPhone Mirroring signs in, dark @2x).
        bool controlDisabled = false;
    };

    // Returns true on the frame the selection changes.
    bool Picker(const char* label, int* selection, std::span<const char* const> items, const PickerOptions& options = {});
    bool Picker(const char* label, int* selection, std::initializer_list<const char*> items, const PickerOptions& options = {});

    // A segmented control whose segments each switch on and off (AppKit's select-any): a push button's bezel split by
    // separators, a segment that is on in the accent. Items are titles or Typography::SymbolText; width is the whole
    // control's in points, 0 fits the items. Returns true on the frame a segment changes.
    bool SegmentedToggles(const char* label, std::span<bool> on, std::span<const char* const> items, float width = 0.0f);
    bool SegmentedToggles(const char* label, std::span<bool> on, std::initializer_list<const char*> items, float width = 0.0f);
} // namespace Cupertino
