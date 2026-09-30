#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include <cfloat>
#include <functional>

namespace Cupertino {
    struct SplitViewOptions {
        // The first pane's width (height in a VSplitView) in points, where the divider stands; dragging it changes it.
        float* position = nullptr;
        // The first pane's limits, and the least room the second pane keeps.
        float minimum = 60.0f;
        float maximum = FLT_MAX;
        float secondMinimum = 60.0f;
    };

    // HSplitView and VSplitView: two panes filling the offered space side by side or stacked, split by a 1 pt divider
    // (Xcode @2x: #000@0.25 in light, black in dark) that the pointer drags with the resize cursor.
    void HSplitView(const SplitViewOptions& options, const std::function<void()>& first, const std::function<void()>& second);
    void VSplitView(const SplitViewOptions& options, const std::function<void()>& first, const std::function<void()>& second);

    // A divider's line (pixels) that the pointer drags within 3 pt of it: position (points from start, the container's
    // edge in pixels) follows the pointer between minimum and maximum. Returns true while it is dragged.
    bool SplitDivider(ImGuiID id, const ImRect& line, ImGuiAxis axis, float start, float* position, float minimum, float maximum);
} // namespace Cupertino
