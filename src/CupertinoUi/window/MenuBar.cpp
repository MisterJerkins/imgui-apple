#include "MenuBar.h"

#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/KeyboardShortcut.h"
#include "core/MenuContent.h"
#include "core/Metrics.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "overlays/PopUpMenu.h"

#include "imgui_internal.h"

namespace Cupertino {
    // The bar being filled: its frame, where the next title goes and whether the application's title is placed.
    struct BarFill {
        ImDrawList* draw = nullptr;
        ImRect bar;
        float x = 0.0f;
        bool applicationPlaced = false;
    };

    static BarFill* filling = nullptr;
    // The menu open from the bar, 0 when none.
    static ImGuiID openMenu = 0;

    // A menu of the bar: its title in order along the bar, and its menu while open.
    static void BarMenu(const char* label, unsigned symbol, const std::function<void()>& content) {
        const Metrics::MenuBarMetrics& metrics = Metrics::MenuBar();
        const Palette& colors = Theme::Colors();
        BarFill& fill = *filling;
        const ImGuiID id = ImGui::GetID(label);
        const bool application = !symbol && !fill.applicationPlaced;
        fill.applicationPlaced |= !symbol;
        const Font font = application ? Font::Style(TextStyle::Body).Weight(FontWeight::Bold) : Font::Style(TextStyle::Body);
        const Font symbol_font = Font::System(metrics.symbolSize);
        const std::string_view title = Interaction::VisibleLabel(label);
        // Each title takes its width in whole points (the titles of the 512 Pixels captures drift from fractional ones).
        const float width = symbol ? Typography::SymbolWidth(symbol, symbol_font) : Typography::Width(font, title);
        const ImRect item(fill.x, fill.bar.Min.y, fill.x + Px(ImCeil(Pt(width)) + 2.0f * metrics.titlePadding), fill.bar.Max.y);
        fill.x = item.Max.x;
        const ImRect plate(item.Min.x - Px(metrics.plateOutset), item.Min.y + Px(metrics.plateInset), item.Max.x + Px(metrics.plateOutset), item.Max.y - Px(metrics.plateInset));

        // A click opens the menu under its title; while one is open the pointer moving onto another title opens that
        // one instead.
        const ImGuiIO& io = ImGui::GetIO();
        const bool hovered = item.Contains(io.MousePos);
        const bool moved = io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f;
        const bool other_open = openMenu && openMenu != id && PopUpMenu::IsOpen(openMenu);
        if (hovered && ((ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !PopUpMenu::IsOpen(id)) || (moved && other_open))) {
            if (other_open)
                PopUpMenu::Close(openMenu);
            const ImRect anchor(plate.Min.x + Px(metrics.menuInset), plate.Min.y, plate.Max.x, fill.bar.Max.y + Px(metrics.menuGap));
            PopUpMenu::Open(id, anchor, -1, PopUpMenu::Placement::MenuBar);
            openMenu = id;
        }
        const bool open = PopUpMenu::IsOpen(id);
        if (open)
            Draw::FillRoundedRect(fill.draw, plate, CornerRadii(Px(metrics.plateRadius)), colors.menuBarSelection);
        if (symbol) {
            Typography::DrawSymbol(fill.draw, symbol, symbol_font, item, colors.menuBarTitle);
        } else {
            const float top = item.Min.y + Px(metrics.baseline) - Typography::Baseline(font);
            Typography::Draw(fill.draw, font, ImRect(item.Min.x + Px(metrics.titlePadding), top, item.Max.x, top + Px(font.lineHeight)), colors.menuBarTitle, title);
        }

        // The items' keyboard shortcuts work while the bar is shown; the menu shows while it is open.
        const std::span<const MenuContent::Entry> entries = MenuContent::Collect(id, content);
        for (size_t i = 0; i < entries.size(); ++i) {
            if (entries[i].kind == MenuContent::EntryKind::Item && entries[i].enabled && IsPressed(entries[i].shortcut))
                MenuContent::Choose(id, int(i));
        }
        if (!open) {
            if (openMenu == id)
                openMenu = 0;
            return;
        }
        const int picked = PopUpMenu::Show(id, entries);
        if (picked >= 0)
            MenuContent::Choose(id, picked);
    }

    void MenuBar(const MenuBarOptions& options, const std::function<void()>& content) {
        const Metrics::MenuBarMetrics& metrics = Metrics::MenuBar();
        const Palette& colors = Theme::Colors();
        ImGuiViewportP* viewport = static_cast<ImGuiViewportP*>(ImGui::GetMainViewport());
        const ImRect bar(viewport->Pos, ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + Px(metrics.height)));
        // The screen's work area starts under the bar, as notifications and other screen-placed panels expect.
        viewport->BuildWorkInsetMin.y += bar.GetHeight();
        // Above every window, under the menus it opens.
        Interaction::BeginOverlay("##CupertinoMenuBar", bar);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        Draw::FillRect(draw, bar, colors.menuBarBackground);

        BarFill fill{draw, bar, bar.Min.x + Px(metrics.leading)};
        BarFill* outer = filling;
        const MenuContent::BarMenu outer_menu = MenuContent::CurrentBarMenu();
        filling = &fill;
        MenuContent::SetBarMenu(BarMenu);
        content();
        MenuContent::SetBarMenu(outer_menu);
        filling = outer;

        if (options.status) {
            const Font font = Font::Style(TextStyle::Body);
            const float top = bar.Min.y + Px(metrics.baseline) - Typography::Baseline(font);
            Typography::Draw(draw, font, ImRect(bar.Min.x, top, bar.Max.x - Px(metrics.statusTrailing), top + Px(font.lineHeight)), colors.menuBarTitle, options.status, TextAlignment::Trailing);
        }
        ImGui::End();
    }
} // namespace Cupertino
