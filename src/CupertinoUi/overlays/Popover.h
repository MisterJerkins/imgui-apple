#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include <functional>

namespace Cupertino {
    // The edge of a popover that carries its arrow, as SwiftUI's arrowEdge: the panel lies on the anchor's other side.
    enum class ArrowEdge {
        // The top edge when the panel fits under the anchor, the bottom edge otherwise.
        Automatic,
        Top,
        Bottom,
        Leading,
        Trailing,
    };

    struct PopoverOptions {
        // The panel's size in points; its content lays out from the top-leading corner.
        ImVec2 size = ImVec2(240.0f, 120.0f);
        ArrowEdge arrowEdge = ArrowEdge::Automatic;
    };

    // .popover(isPresented:arrowEdge:): anchor as it is and, while *is_presented, content in a popover pointing at it. A
    // click outside the panel or Escape closes it and resets *is_presented.
    void Popover(bool* is_presented, const PopoverOptions& options, const std::function<void()>& anchor, const std::function<void()>& content);
} // namespace Cupertino

// The panel of a popover in its own overlay window above everything else, pointing at an anchor.
namespace Cupertino::PopoverPanel {
    // Opens the popover pointing at anchor (pixels).
    void Open(ImGuiID id, const ImRect& anchor, ArrowEdge edge = ArrowEdge::Automatic);
    void Close(ImGuiID id);
    bool IsOpen(ImGuiID id);

    // Draws the popover while it is open and lays content out in a panel of size points. A click outside the panel
    // or Escape closes it; it fades in and out.
    void Show(ImGuiID id, ImVec2 size, const std::function<void()>& content);
} // namespace Cupertino::PopoverPanel
