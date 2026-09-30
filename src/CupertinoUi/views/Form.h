#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include <functional>

namespace Cupertino {
    struct SectionOptions {
        const char* header = nullptr;
        // Secondary text under the header, as under Energy Mode (Battery @2x).
        const char* description = nullptr;
        // Trailing content on the header line, such as the spinner beside Nearby Devices (Bluetooth @2x).
        std::function<void()> headerAccessory;
        const char* footer = nullptr;
        // Content that brings its own box, a bordered list or table, stands under the header without the section's box
        // and row insets (Open at Login in Login Items @2x).
        bool boxed = true;
    };

    // Grouped form as in System Settings: scrolls, 20 pt margins, sections 10 pt apart. Controls inside take their
    // form look (switch toggles, borderless pop-ups).
    void Form(const std::function<void()>& content);

    // A rounded group of rows with separators; in a sidebar list it is a group of rows without a background, in a menu a
    // group of items after a separator under its header.
    void Section(const std::function<void()>& content);
    void Section(const SectionOptions& options, const std::function<void()>& content);

    // The box of a section: the fill and a two-tone inside border; a bordered list draws its rows between the two.
    void DrawSectionBox(ImDrawList* draw, const ImRect& rect);
    void DrawSectionBorder(ImDrawList* draw, const ImRect& rect);
    // A box set into a pane or a row: the quaternary fill inside a section's border (the sleep warning of Screen Saver,
    // the selected disk of Startup Disk @2x), or a tile's tertiary fill in a line of the separator color (Saved to
    // iCloud @2x).
    void DrawInsetBox(ImDrawList* draw, const ImRect& rect);
    void DrawTileBox(ImDrawList* draw, const ImRect& rect);
} // namespace Cupertino
