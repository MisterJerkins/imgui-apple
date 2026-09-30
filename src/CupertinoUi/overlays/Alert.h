#pragma once

#include "controls/Button.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <initializer_list>
#include <span>
#include <string>

namespace Cupertino {
    struct AlertButton {
        const char* title = "";
        // Default draws the accent and answers Return, Cancel answers Escape, Destructive is red: filled and answering
        // Return as the alert's first button (NSAlert's destructive action), red text further down.
        ButtonRole role = ButtonRole::Normal;
    };

    // A text field in an alert, as a TextField or SecureField in the actions of SwiftUI's .alert.
    struct AlertTextField {
        const char* placeholder = "";
        std::string* text = nullptr;
        bool secure = false;
    };

    struct AlertOptions {
        const char* message = nullptr;
        // Paints the 64 pt icon above the title, usually the app icon.
        void (*icon)(ImDrawList* draw, const ImRect& frame) = nullptr;
        // dialogSeverity(.critical): the caution triangle takes the icon's place with the icon as a badge at its corner
        // (VS Code's alert @2x).
        bool critical = false;
        // The round help button in the top-right corner.
        bool showsHelp = false;
        // The suppression checkbox centered under the buttons ("Don't ask again"), bound to suppressed.
        const char* suppression = nullptr;
        bool* suppressed = nullptr;
        // The button that has the keyboard focus when the alert opens, as with keyboard navigation on (Calendar @2x:
        // Cancel); Tab moves it through the buttons and Space presses it. -1 leaves the buttons to the mouse and Return.
        int focus = -1;
        // Fields under the message, the first one focused as the alert opens; Tab goes to the next one and Return answers
        // the default button.
        std::span<const AlertTextField> textFields;
    };

    // Presents an alert while *is_presented is true, like SwiftUI's .alert: the icon, the bold title, the message and
    // the buttons, two side by side (the first on the leading side) or more stacked with a cancel button last, apart
    // from the others. Returns the index of the button chosen this frame, which also dismisses the alert, or -1.
    int Alert(const char* title, bool* is_presented, std::span<const AlertButton> buttons, const AlertOptions& options = {});
    int Alert(const char* title, bool* is_presented, std::initializer_list<AlertButton> buttons, const AlertOptions& options = {});

    // .confirmationDialog on macOS: an alert of the actions with Cancel added last. Returns the action chosen this frame,
    // the actions' count for Cancel, or -1.
    int ConfirmationDialog(const char* title, bool* is_presented, std::span<const AlertButton> actions, const AlertOptions& options = {});
    int ConfirmationDialog(const char* title, bool* is_presented, std::initializer_list<AlertButton> actions, const AlertOptions& options = {});
} // namespace Cupertino
