#include "Menu.h"

#include "Bezel.h"
#include "Button.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/MenuContent.h"
#include "core/Metrics.h"
#include "core/Symbols.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"
#include "overlays/PopUpMenu.h"

#include <vector>

namespace Cupertino {
    // A toolbar item: the symbol and a chevron.down on one hover plate; the menu hangs under the plate. Returns the
    // menu's id, or 0 while measuring.
    static ImGuiID ToolbarMenuButton(const char* label, unsigned symbol) {
        const Metrics::WindowMetrics& metrics = Metrics::Window();
        const ImRect slot = Layout::Place(Px(ImVec2(metrics.menuWidth, metrics.symbolButtonSlot)));
        if (Layout::IsMeasuring())
            return 0;
        const ImGuiID id = ImGui::GetID(label);
        const bool clicked = ToolbarButton(label, slot, Environment().enabled, [&](ImDrawList* draw, Rgba color) {
            const float y = slot.Min.y + Px(metrics.symbolCenterY + metrics.menuOffsetY);
            DrawToolbarSymbol(draw, symbol, ImVec2(slot.Min.x + Px(metrics.menuSymbolX), y), color);
            const ImVec2 chevron(slot.Min.x + Px(metrics.menuChevronX), y);
            const ImVec2 half = Px(ImVec2(metrics.menuChevronFrame, metrics.menuChevronFrame)) * 0.5f;
            Typography::DrawSymbol(draw, Symbols::ChevronDown, Font::System(metrics.menuChevronSize, FontWeight::Bold), ImRect(chevron - half, chevron + half), color);
        });
        if (clicked && !PopUpMenu::IsOpen(id))
            PopUpMenu::Open(id, ToolbarPlate(slot), -1, PopUpMenu::Placement::Below);
        return id;
    }

    // A titled pull-down in a toolbar (kit's Pull-down Button): the title and a chevron.down in the toolbar's symbol
    // color on the item's plate, the menu under it. Returns the menu's id, or 0 while measuring.
    static ImGuiID ToolbarTitleMenuButton(const char* label) {
        const Metrics::WindowMetrics& metrics = Metrics::Window();
        const std::string_view title = Interaction::VisibleLabel(label);
        const Font font = Font::Style(TextStyle::Body);
        const float title_width = Typography::Width(font, title);
        const ImRect slot = Layout::Place(ImVec2(title_width + Px(metrics.toolbarMenuTitleX + metrics.toolbarMenuChevronGap + metrics.toolbarChevronFrame + metrics.toolbarMenuTrailing), Px(metrics.symbolButtonSlot)));
        if (Layout::IsMeasuring())
            return 0;
        const ImGuiID id = ImGui::GetID(label);
        const bool clicked = ToolbarButton(label, slot, Environment().enabled, [&](ImDrawList* draw, Rgba color) {
            const float x = slot.Min.x + Px(metrics.toolbarMenuTitleX);
            Typography::Draw(draw, font, ImRect(x, slot.Min.y, x + title_width, slot.Max.y), color, title);
            const float chevron = x + title_width + Px(metrics.toolbarMenuChevronGap);
            Typography::DrawSymbol(draw, Symbols::ChevronDown, Font::System(metrics.menuChevronSize, FontWeight::Bold), ImRect(chevron, slot.Min.y, chevron + Px(metrics.toolbarChevronFrame), slot.Max.y), color);
        });
        if (clicked && !PopUpMenu::IsOpen(id))
            PopUpMenu::Open(id, ToolbarPlate(slot), -1, PopUpMenu::Placement::Below);
        return id;
    }

    // A pull-down button titled by its label or by a symbol, with a chevron.down indicator on the accent plate or plain,
    // the menu under the button. Returns the menu's id, or 0 while measuring.
    static ImGuiID PullDownButton(const char* label, unsigned symbol, bool plain) {
        const Metrics::BezelPopUpMetrics metrics = Metrics::BezelPopUp();
        const Font font = Font::Style(TextStyle::Body);
        const std::string_view title = Interaction::VisibleLabel(label);
        const float title_room = plain ? metrics.plainInset + metrics.plainTitleGap + metrics.plainChevronFrame.x + metrics.plainTrailingInset : metrics.labelInset + metrics.gap + metrics.indicator + metrics.trailingInset;
        const float symbol_room = plain ? metrics.plainInset + metrics.plainChevronGap + metrics.plainChevronFrame.x + metrics.plainTrailingInset : metrics.symbolInset + metrics.symbolGap + metrics.indicator + metrics.symbolTrailingInset;
        const float width = symbol ? Px(symbol_room) + Typography::SymbolWidth(symbol, font) : Px(title_room) + Typography::Width(font, title);
        const ImRect frame = Layout::Place(Layout::Placement{.size = ImVec2(width, Px(metrics.height)), .baseline = Typography::CenteredBaseline(font, Px(metrics.height))});
        if (Layout::IsMeasuring())
            return 0;
        const ImGuiID id = ImGui::GetID(label);
        if (Interaction::Button(id, frame, ImGuiButtonFlags_PressedOnClick).pressed && !PopUpMenu::IsOpen(id))
            PopUpMenu::Open(id, frame, -1, PopUpMenu::Placement::Below);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const bool focused = Interaction::FocusVisible(id);
        if (symbol)
            Bezel::SymbolMenuButton(draw, frame, symbol, PopUpMenu::IsOpen(id), plain, focused);
        else
            Bezel::MenuButton(draw, frame, title, PopUpMenu::IsOpen(id), plain ? Bezel::MenuIndicator::PlainDown : Bezel::MenuIndicator::Down, focused);
        return id;
    }

    static ImGuiID MenuButton(const char* label, const MenuOptions& options) {
        if (Layout::ParentRole() == Layout::Role::Toolbar)
            return options.symbol ? ToolbarMenuButton(label, options.symbol) : ToolbarTitleMenuButton(label);
        return PullDownButton(label, options.symbol, options.plainIndicator);
    }

    // Shows the menu id while it is open or fading, filled by content; a choice comes back to its view in the next frame.
    static void Present(ImGuiID id, const std::function<void()>& content) {
        if (!PopUpMenu::IsOpen(id))
            return;
        const int picked = PopUpMenu::Show(id, MenuContent::Collect(id, content));
        if (picked >= 0)
            MenuContent::Choose(id, picked);
    }

    int Menu(const char* label, std::span<const char* const> actions, const MenuOptions& options) {
        const ImGuiID id = MenuButton(label, options);
        return id ? PopUpMenu::Show(id, actions) : -1;
    }

    int Menu(const char* label, std::initializer_list<const char*> actions, const MenuOptions& options) {
        return Menu(label, std::span<const char* const>(actions.begin(), actions.size()), options);
    }

    void Menu(const char* label, const std::function<void()>& content) {
        Menu(label, MenuOptions{}, content);
    }

    void Menu(const char* label, const MenuOptions& options, const std::function<void()>& content) {
        if (MenuContent::IsCollecting()) {
            MenuContent::BeginSubmenu(Interaction::VisibleLabel(label), options.symbol);
            content();
            MenuContent::EndSubmenu();
            return;
        }
        if (const MenuContent::BarMenu bar_menu = MenuContent::CurrentBarMenu()) {
            bar_menu(label, options.symbol, content);
            return;
        }
        if (const ImGuiID id = MenuButton(label, options))
            Present(id, content);
    }

    void ContextMenu(const std::function<void()>& menu, const std::function<void()>& content) {
        const ImGuiID id = Layout::NextViewId();
        const bool sidebar_row = Layout::ParentRole() == Layout::Role::Sidebar;
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Overlay;
        Layout::Container(spec, content, [&](const Layout::ContainerFrame& frame) {
            const ImVec2 pointer = ImGui::GetIO().MousePos;
            const bool hovered = Interaction::PointerOver(frame.rect);
            if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !PopUpMenu::IsOpen(id))
                PopUpMenu::Open(id, ImRect(pointer, pointer), -1, PopUpMenu::Placement::AtPoint);
            if (sidebar_row && PopUpMenu::IsOpen(id)) {
                const Metrics::SidebarMetrics& metrics = Metrics::Sidebar();
                const float inset = Px(metrics.platterInset);
                Bezel::MenuTarget(ImGui::GetWindowDrawList(), ImRect(frame.rect.Min.x + inset, frame.rect.Min.y, frame.rect.Max.x - inset, frame.rect.Max.y), CornerRadii(Px(metrics.platterRadius)));
            }
        });
        Present(id, menu);
    }

    void ControlGroup(const std::function<void()>& content) {
        const Metrics::ControlGroupMetrics& metrics = Metrics::ControlGroup();
        const ImGuiID id = Layout::NextViewId();
        const std::span<const MenuContent::Entry> entries = MenuContent::Collect(id, content);
        const Font font = Font::Style(TextStyle::Body);
        const auto segment_width = [&](const MenuContent::Entry& entry) {
            if (entry.kind == MenuContent::EntryKind::Submenu)
                return metrics.menuSegment;
            return entry.symbol ? metrics.symbolSegment : Pt(Typography::Width(font, entry.title)) + 2.0f * metrics.labelPadding;
        };
        // The segments are the items and submenus at the top of the content.
        std::vector<int> segments;
        float width = 0.0f;
        for (int i = 0; i < int(entries.size()); ++i) {
            const MenuContent::Entry& entry = entries[size_t(i)];
            if (entry.kind == MenuContent::EntryKind::Item || entry.kind == MenuContent::EntryKind::Submenu) {
                segments.push_back(i);
                width += segment_width(entry);
            }
            if (entry.kind == MenuContent::EntryKind::Submenu)
                i += entry.children;
        }
        const ImRect frame = Layout::Place(Layout::Placement{.size = Px(ImVec2(width, metrics.height)), .baseline = Typography::CenteredBaseline(font, Px(metrics.height))});
        if (Layout::IsMeasuring() || segments.empty())
            return;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const CornerRadii radii(Px(metrics.radius));
        {
            const Interaction::DisabledFade fade;
            Bezel::Pill(draw, frame, radii, false);
        }
        ImGui::PushID(int(id));
        float left = frame.Min.x;
        for (size_t k = 0; k < segments.size(); ++k) {
            const int index = segments[k];
            const MenuContent::Entry& entry = entries[size_t(index)];
            const bool menu = entry.kind == MenuContent::EntryKind::Submenu;
            const bool last = k + 1 == segments.size();
            const ImRect segment(left, frame.Min.y, last ? frame.Max.x : Draw::Snap(left + Px(segment_width(entry))), frame.Max.y);
            const ImGuiID segment_id = ImGui::GetID(index);
            Interaction::Response response;
            Disabled(!entry.enabled, [&] { response = Interaction::Button(segment_id, segment, menu ? ImGuiButtonFlags_PressedOnClick : 0); });
            const bool enabled = entry.enabled && Environment().enabled;
            const CornerRadii corners(k == 0 ? radii.topLeft : 0.0f, last ? radii.topRight : 0.0f, last ? radii.bottomRight : 0.0f, k == 0 ? radii.bottomLeft : 0.0f);
            if (enabled && ((response.held && response.hovered) || (menu && PopUpMenu::IsOpen(segment_id))))
                Draw::FillRoundedRect(draw, segment, corners, colors.controlPressed);
            if (response.focused)
                Draw::FocusRing(draw, segment, corners, colors.accent);
            if (response.pressed && enabled) {
                if (menu && !PopUpMenu::IsOpen(segment_id))
                    PopUpMenu::Open(segment_id, segment, -1, PopUpMenu::Placement::Below);
                else if (!menu)
                    MenuContent::Choose(id, index);
            }
            if (!last) {
                const ImVec2 size = Px(metrics.separator);
                const float top = segment.GetCenter().y - size.y * 0.5f;
                Draw::FillRect(draw, ImRect(segment.Max.x - size.x, top, segment.Max.x, top + size.y), colors.separator);
            }
            const Rgba color = colors.LabelColor(enabled);
            if (menu) {
                const ImVec2 symbol(segment.Min.x + Px(metrics.menuSymbolX), segment.GetCenter().y);
                const ImVec2 chevron(segment.Min.x + Px(metrics.menuChevronX), segment.GetCenter().y);
                const ImVec2 half = Px(ImVec2(10.0f, 8.0f));
                Typography::DrawSymbol(draw, entry.symbol, font, ImRect(symbol - half, symbol + half), color);
                Typography::DrawSymbol(draw, Symbols::ChevronDown, Font::System(metrics.chevronSize, FontWeight::Bold), ImRect(chevron - half, chevron + half), color);
                const int picked = PopUpMenu::Show(segment_id, entries.subspan(size_t(index) + 1, size_t(entry.children)));
                if (picked >= 0)
                    MenuContent::Choose(id, index + 1 + picked);
            } else if (entry.symbol) {
                Typography::DrawSymbol(draw, entry.symbol, font, segment, color);
            } else {
                Typography::Draw(draw, font, Bezel::TitleFrame(segment, 0.0f), color, entry.title, TextAlignment::Center);
            }
            left = segment.Max.x;
        }
        ImGui::PopID();
    }
} // namespace Cupertino
