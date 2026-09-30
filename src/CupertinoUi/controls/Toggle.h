#pragma once

#include "IconPlate.h"
#include "core/Typography.h"

namespace Cupertino {
    enum class ToggleStyle {
        // A switch inside a Form (a trailing mini switch in its row), a checkbox elsewhere on macOS.
        Automatic,
        Switch,
        Checkbox,
        // A radio button on its own, as AppKit's radio NSButton: a click turns it on, never off.
        Radio,
        // SwiftUI's .button toggle style: a push button that stays lit (the accent bezel) while on.
        Button,
        // A push button whose title takes the accent while on (AppKit's toggle button; the kit's "As Toggle / Style 1").
        TitleButton,
        // A scope of an accessory bar, on its gray plate while on (.toggleStyle(.button) with .buttonStyle(.accessoryBar)).
        AccessoryBar,
    };

    struct ToggleOptions {
        ToggleStyle style = ToggleStyle::Automatic;
        // Secondary text under the label of a form row, like the second Text in a SwiftUI label, and its style.
        const char* description = nullptr;
        TextStyle descriptionStyle = TextStyle::Subheadline;
        // A large icon before the label turns the form row into a pane header with a regular switch.
        Icon icon;
        // Neither on nor off, as a checkbox over items that differ (AppKit's mixed state): a dash; a click turns it on.
        bool mixed = false;
    };

    // Returns true on the frame the value changes. In a menu it is an item with a checkmark.
    bool Toggle(const char* label, bool* is_on, const ToggleOptions& options = {});
} // namespace Cupertino
