#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include <functional>
#include <string_view>

namespace Cupertino {
    // Shows text in a tooltip while the pointer rests on content, like SwiftUI's .help: after a second without moving,
    // under the pointer, until the pointer leaves, a button is pressed or a key is typed. Moving to another view with help
    // while one shows switches at once.
    void Help(std::string_view text, const std::function<void()>& content);
} // namespace Cupertino

namespace Cupertino::Tooltip {
    // The tooltip's size in pixels for text, wrapped at the tooltip's width.
    ImVec2 Measure(std::string_view text);

    // Draws a tooltip panel with its top-left corner at origin (pixels).
    void Draw(ImDrawList* draw, ImVec2 origin, std::string_view text);

    // Draws the tooltip over everything once the frame's views are laid out; Cupertino::EndFrame calls it.
    void Present();
} // namespace Cupertino::Tooltip
