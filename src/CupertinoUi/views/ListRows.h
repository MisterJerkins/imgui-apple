#pragma once

#include "core/Color.h"
#include "core/Typography.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <span>
#include <vector>

// What lists and tables share: their events, how their rows take clicks, and the menu a right click opens on a row.
namespace Cupertino {
    enum class ListAction {
        None,
        Add,
        Remove,
    };

    struct ListEvent {
        ListAction action = ListAction::None;
        // The context menu item chosen this frame and the row it was opened on, or -1.
        int menuItem = -1;
        int row = -1;
        // The row double-clicked this frame, SwiftUI's primaryAction, or -1.
        int doubleClickedRow = -1;
        // The item of the actions pull-down chosen this frame, or -1.
        int actionItem = -1;
        // A click on a column's header changed the sort order this frame.
        bool sortChanged = false;
    };

    // Called in the list's ID scope (ImGui::PushID of the list's id).
    namespace ListRows {
        // A row: a click selects it (Command toggles, Shift extends from anchor) and gives the list the keyboard focus, a
        // double click goes into event, and with a menu a right click opens it at the pointer for the row, the selection
        // staying as it is. Controls in the row keep their own clicks.
        void Row(ImGuiID list, int row, const ImRect& rect, std::vector<bool>* selection, int& anchor, bool has_menu, ListEvent& event);
        // Shows the menu a row opened; the item chosen and its row go into event.
        void Menu(ImGuiID list, std::span<const char* const> items, ListEvent& event);
        // The row whose menu is open, or -1; it is ringed meanwhile unless it is selected (Finder, Mail @2x).
        int MenuRow(ImGuiID list);

        // How the bar under a list's rows draws its buttons: the symbols' font and color while enabled, and the divider
        // after each, dividerHeight points tall (0 spans the bar).
        struct BarStyle {
            Font font;
            Rgba symbol;
            Rgba divider;
            float dividerHeight = 0.0f;
        };
        // A + or − button of the bar and the divider after it; disabled, its symbol turns tertiary.
        bool BarButton(ImGuiID id, const ImRect& rect, unsigned symbol, bool enabled, const BarStyle& style);
        // The bar's actions pull-down: gear and chevron.down, its menu under it. Returns the item chosen, or -1.
        int ActionsButton(ImGuiID id, const ImRect& rect, std::span<const char* const> actions, Rgba color);
    } // namespace ListRows
} // namespace Cupertino
