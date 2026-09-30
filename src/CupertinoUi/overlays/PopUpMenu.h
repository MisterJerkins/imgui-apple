#pragma once

#include "core/MenuContent.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <span>

// Menus of pop-up and pull-down buttons, context menus and the menu bar, drawn in overlay windows above everything else.
namespace Cupertino::PopUpMenu {
    enum class Placement {
        // The selected item's label covers the anchor (the button's label), like NSPopUpButton.
        OverLabel,
        // The menu hangs under the anchor (the button), left edges aligned, like a pull-down.
        Below,
        // A context menu: its top-left corner at the anchor's (the pointer).
        AtPoint,
        // A menu of the menu bar: its top-left corner at the anchor's bottom-left.
        MenuBar,
        // A combo box's list (AppKit docs @2x): 18 pt rows under the anchor (the control's bezel) with their titles on the
        // anchor's text, which starts at trailing_limit's place (see Open).
        ComboBoxList,
        // A search field's suggestions: under the anchor (the field) without taking the pointer or the focus from it, so
        // typing goes on; Return picks the highlighted one, Space types.
        Suggestions,
    };

    // selected is the checked item a pop-up puts over its label, or -1. The menu ends at trailing_limit when given (a form
    // pop-up's at its indicator, which stays in view, Battery @2x; a combo box's list at its text) and the root menu is
    // at least minimum_width points wide, as NSMenu's minimumWidth.
    void Open(ImGuiID id, const ImRect& anchor, int selected, Placement placement = Placement::OverLabel, float trailing_limit = FLT_MAX, float minimum_width = 0.0f);
    bool IsOpen(ImGuiID id);
    // Closes at once, without fading, as the menu bar does when the pointer moves on to another title.
    void Close(ImGuiID id);

    // Paints an item's image into frame; images sit in a column before the titles.
    using ItemImage = void (*)(ImDrawList* draw, const ImRect& frame, int item);

    // Draws the menu while it is open or fading out. Returns the index of the entry chosen this frame, or -1.
    int Show(ImGuiID id, std::span<const MenuContent::Entry> entries, ItemImage image = nullptr, ImVec2 image_size = ImVec2());
    // A menu of plain titles: a pop-up's (opened with a selected item) keeps the checkmark column and checks that item.
    int Show(ImGuiID id, std::span<const char* const> items, ItemImage image = nullptr, ImVec2 image_size = ImVec2());
} // namespace Cupertino::PopUpMenu
