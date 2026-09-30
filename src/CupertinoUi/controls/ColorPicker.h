#pragma once

#include "core/Color.h"

namespace Cupertino {
    enum class ColorWellStyle {
        // SwiftUI's ColorPicker: the color set into a white push-button bezel (System Settings > Accessibility > Display).
        Automatic,
        // AppKit's classic bordered well (kit, 28 x 20): the color inside a gray gradient frame. A click makes it the active
        // well, pressed in, and opens the Colors window on its color.
        Bordered,
    };

    struct ColorPickerOptions {
        ColorWellStyle style = ColorWellStyle::Automatic;
    };

    // A color well, like SwiftUI's ColorPicker. A click opens a popover of preset colors, or the Colors window for a
    // bordered well; in a form row the label sits on the leading side. Returns true when the color changes.
    bool ColorPicker(const char* label, Rgba* color, const ColorPickerOptions& options = {});
} // namespace Cupertino
