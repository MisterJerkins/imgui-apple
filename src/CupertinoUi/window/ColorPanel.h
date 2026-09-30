#pragma once

#include "core/Color.h"

#include "imgui.h"

namespace Cupertino {
    // NSColorPanel, the Colors window: the color wheel with its brightness slider, or RGB or HSB sliders, then the opacity,
    // the color and two rows of saved colors (a click on an empty one saves the color, on a filled one picks it). It floats
    // in front of the app's windows while *is_presented; the close button hides it. Returns true when the color changes.
    bool ColorPanel(bool* is_presented, Rgba* color);

    // The Colors window shared by bordered color wells, as in AppKit: a click on a well makes it the active one and opens
    // the window with its color, which then edits that well's color until the well is clicked again or the window closes.
    namespace SharedColorPanel {
        // Called by a well every frame: clicked toggles it, and the window's edits come into color. Returns true when the
        // window changed the color.
        bool Bind(ImGuiID well, Rgba* color, bool clicked);
        bool IsActive(ImGuiID well);
        // Draws the window over everything once the frame's views are laid out; Cupertino::EndFrame calls it.
        void Present();
    } // namespace SharedColorPanel
} // namespace Cupertino
