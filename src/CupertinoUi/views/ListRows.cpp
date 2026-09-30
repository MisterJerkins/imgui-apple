#include "ListRows.h"

#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/State.h"
#include "core/Symbols.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "overlays/PopUpMenu.h"

namespace Cupertino::ListRows {
    struct RowMenuState {
        int row = -1;
    };

    static ImGuiID MenuId(ImGuiID list) {
        return ImHashStr("##RowMenu", 0, list);
    }

    void Row(ImGuiID list, int row, const ImRect& rect, std::vector<bool>* selection, int& anchor, bool has_menu, ListEvent& event) {
        const Interaction::Response response = Interaction::Button(ImGui::GetID(row), rect, ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_AllowOverlap, ImGuiItemFlags_NoNav);
        if (response.pressed && selection) {
            Interaction::ClickSelection(*selection, anchor, row);
            Interaction::FocusList(list);
        }
        if (response.pressed && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            event.doubleClickedRow = row;
        if (!has_menu || !response.hovered || !ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            return;
        Interaction::FocusedList() = list;
        State::Get<RowMenuState>(MenuId(list)).row = row;
        const ImVec2 pointer = ImGui::GetIO().MousePos;
        PopUpMenu::Open(MenuId(list), ImRect(pointer, pointer), -1, PopUpMenu::Placement::AtPoint);
    }

    void Menu(ImGuiID list, std::span<const char* const> items, ListEvent& event) {
        if (items.empty())
            return;
        event.menuItem = PopUpMenu::Show(MenuId(list), items);
        event.row = event.menuItem >= 0 ? State::Get<RowMenuState>(MenuId(list)).row : -1;
    }

    int MenuRow(ImGuiID list) {
        return PopUpMenu::IsOpen(MenuId(list)) ? State::Get<RowMenuState>(MenuId(list)).row : -1;
    }

    bool BarButton(ImGuiID id, const ImRect& rect, unsigned symbol, bool enabled, const BarStyle& style) {
        const Palette& colors = Theme::Colors();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        bool pressed = false;
        Disabled(!enabled, [&] {
            const Interaction::Response response = Interaction::Button(id, rect);
            if (response.held && response.hovered)
                Draw::FillRect(draw, rect, colors.tertiaryFill);
            pressed = response.pressed;
        });
        const bool active = enabled && Environment().enabled;
        Typography::DrawSymbol(draw, symbol, style.font, rect, active ? style.symbol : colors.tertiaryLabel);
        const float half = style.dividerHeight > 0.0f ? Px(style.dividerHeight) * 0.5f : rect.GetHeight() * 0.5f;
        Draw::FillRect(draw, ImRect(rect.Max.x, rect.GetCenter().y - half, rect.Max.x + Px(1.0f), rect.GetCenter().y + half), style.divider);
        return pressed && active;
    }

    int ActionsButton(ImGuiID id, const ImRect& rect, std::span<const char* const> actions, Rgba color) {
        const Metrics::BorderedListMetrics& metrics = Metrics::BorderedList();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Interaction::Response response = Interaction::Button(id, rect, ImGuiButtonFlags_PressedOnClick);
        if (response.pressed && !PopUpMenu::IsOpen(id))
            PopUpMenu::Open(id, rect, -1, PopUpMenu::Placement::Below);
        if (PopUpMenu::IsOpen(id))
            Draw::FillRect(draw, rect, Theme::Colors().tertiaryFill);
        const float gear = rect.Min.x + Px(metrics.actionSymbolX);
        const float chevron = rect.Min.x + Px(metrics.actionChevronX);
        Typography::DrawSymbol(draw, Symbols::Gear, Font::System(metrics.actionSymbolSize, FontWeight::Medium), ImRect(gear, rect.Min.y, chevron, rect.Max.y), color);
        Typography::DrawSymbol(draw, Symbols::ChevronDown, Font::System(metrics.actionChevronSize, FontWeight::Bold), ImRect(chevron, rect.Min.y + Px(1.0f), chevron + Px(metrics.actionChevronWidth), rect.Max.y + Px(1.0f)), color);
        return PopUpMenu::Show(id, actions);
    }
} // namespace Cupertino::ListRows
