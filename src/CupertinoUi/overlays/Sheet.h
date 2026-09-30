#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include <functional>

namespace Cupertino {
    struct SheetOptions {
        // Size of the sheet in points; without a height it follows the content.
        float width = 470.0f;
        float height = 0.0f;
    };

    // Presents a sheet over the current window while *is_presented is true, like SwiftUI's .sheet: the window dims and
    // a white panel sized to its content, or to the given height, sits in its middle. A Form inside it keeps 20 pt
    // margins and scrolls only in a sheet of fixed height; the content usually ends with a SheetFooter.
    void Sheet(bool* is_presented, const SheetOptions& options, const std::function<void()>& content);

    // Where a sheet of the given size in points stands on its window's frame, in pixels: centered on whole points, a
    // sheet sized by its content a little higher, and never over the toolbar.
    ImVec2 SheetOrigin(const ImRect& window, ImVec2 size, bool sized_by_content);

    // The bottom bar of a sheet: a separator across it, then the buttons on the trailing side with 20 pt margins; the
    // leading ones (a help button, secondary actions) on the other side. Push buttons there are at least 64 pt wide.
    void SheetFooter(const std::function<void()>& buttons);
    void SheetFooter(const std::function<void()>& leading, const std::function<void()>& trailing);
} // namespace Cupertino
