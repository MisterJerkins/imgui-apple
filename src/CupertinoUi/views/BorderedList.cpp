#include "BorderedList.h"

#include "Form.h"
#include "controls/Bezel.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/State.h"
#include "core/Symbols.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"
#include "overlays/PopUpMenu.h"

#include <algorithm>

namespace Cupertino {
    struct BorderedListState {
        // The row a Shift-click extends the selection from.
        int anchor = 0;
    };

    // An inset list's + and − in a push-button bezel split by a divider (kit Footer/Standalone).
    static ListAction BezelButtons(const ImRect& bezel, bool can_remove) {
        const Metrics::BorderedListMetrics& metrics = Metrics::BorderedList();
        const Palette& colors = Theme::Colors();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const float radius = Px(Metrics::PushButton().radius);
        Bezel::Pill(draw, bezel, CornerRadii(radius), false);
        const float half = (bezel.GetWidth() - Px(1.0f)) * 0.5f;
        ListAction action = ListAction::None;
        for (int i = 0; i < 2; ++i) {
            const bool add = i == 0;
            const ImRect segment(add ? bezel.Min.x : bezel.Max.x - half, bezel.Min.y, add ? bezel.Min.x + half : bezel.Max.x, bezel.Max.y);
            const bool enabled = add || can_remove;
            Disabled(!enabled, [&] {
                const Interaction::Response response = Interaction::Button(ImGui::GetID(add ? "add" : "remove"), segment);
                if (response.held && response.hovered)
                    Draw::FillRoundedRect(draw, segment, add ? CornerRadii(radius, 0.0f, 0.0f, radius) : CornerRadii(0.0f, radius, radius, 0.0f), colors.controlPressed);
                if (response.pressed && enabled)
                    action = add ? ListAction::Add : ListAction::Remove;
            });
            Typography::DrawSymbol(draw, add ? Symbols::Plus : Symbols::Minus, Font::System(metrics.buttonSymbolSize, FontWeight::Semibold), segment, colors.LabelColor(enabled));
        }
        const float divider = Px(metrics.insetBezelDivider) * 0.5f;
        Draw::FillRect(draw, ImRect(bezel.Min.x + half, bezel.GetCenter().y - divider, bezel.Min.x + half + Px(1.0f), bezel.GetCenter().y + divider), colors.quaternaryFill);
        return action;
    }

    ListEvent BorderedList(const char* id, const BorderedListOptions& options, std::vector<bool>* selection, const std::function<void(int row)>& content) {
        const Metrics::BorderedListMetrics& metrics = Metrics::BorderedList();
        const bool bordered = options.style == ListStyle::Bordered;
        const bool inset = options.style == ListStyle::Inset;
        // Without a height the list takes the offered height, or where nothing limits it (a scroll view) its rows'.
        const float header_height = options.columns.size() > 0 ? metrics.headerHeight : options.title ? metrics.titleHeight : 0.0f;
        const float rows_top = inset ? metrics.insetTop : 0.0f;
        const float bar_height = options.showsAddRemove && !inset ? metrics.footerHeight : 0.0f;
        const float bezel_height = options.showsAddRemove && inset ? metrics.insetBezelSpacing + metrics.insetBezel.y : 0.0f;
        const float natural = header_height + 2.0f * rows_top + options.rowHeight * float(options.rows) + bar_height + bezel_height;
        Layout::Placement placement;
        placement.size = ImVec2(Layout::Proposal().x, options.height > 0.0f ? Px(options.height) : Layout::Proposal().y > 0.0f ? Layout::Proposal().y : Px(natural));
        placement.flexibleWidth = true;
        placement.flexibleHeight = options.height <= 0.0f;
        const ImRect placed = Layout::Place(placement);
        ListEvent event;
        if (Layout::IsMeasuring())
            return event;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const ImGuiID list_id = ImGui::GetID(id);
        BorderedListState& state = State::Get<BorderedListState>(list_id);
        if (selection && int(selection->size()) != options.rows)
            selection->resize(size_t(options.rows), false);
        const auto selected = [&](int row) { return selection && row < options.rows && (*selection)[size_t(row)]; };

        // The box: a form section's, or the table background with a hairline under it.
        const ImRect box(placed.Min, ImVec2(placed.Max.x, placed.Max.y - Px(bezel_height)));
        const float radius = bordered ? Px(Metrics::Form().sectionRadius) : inset ? Px(metrics.insetRadius) : 0.0f;
        const CornerStyle corners = bordered ? CornerStyle::Circular : CornerStyle::Continuous;
        if (bordered) {
            Draw::FillRoundedRect(draw, box, CornerRadii(radius), colors.sectionBackground, corners);
        } else {
            Draw::DropShadows(draw, box, CornerRadii(radius), Theme::ListShadows(), corners);
            Draw::FillRoundedRect(draw, box, CornerRadii(radius), colors.tableBackground, corners);
        }
        const float header = Px(header_height);
        const ImRect rows(box.Min.x, box.Min.y + header + Px(rows_top), box.Max.x, box.Max.y - Px(bar_height));
        if (options.title) {
            const ImRect line(box.Min.x + Px(metrics.titleLeading), box.Min.y, box.Max.x, rows.Min.y - Px(1.0f));
            Typography::Draw(draw, Font::Style(TextStyle::Body).Weight(FontWeight::Semibold), line, colors.label, options.title);
            Draw::FillRect(draw, ImRect(box.Min.x, rows.Min.y - Px(1.0f), box.Max.x, rows.Min.y), colors.rowSeparator);
        } else if (header > 0.0f) {
            // The header tinted like the bar with a line under it, the titles in the label color.
            const ImRect band(box.Min.x, box.Min.y, box.Max.x, rows.Min.y - Px(1.0f));
            Draw::FillRoundedRect(draw, band, CornerRadii(radius, radius, 0.0f, 0.0f), colors.quaternaryFill, corners);
            Draw::FillRect(draw, ImRect(box.Min.x, band.Max.y, box.Max.x, rows.Min.y), colors.secondaryFill);
            float x = box.Min.x + Px(metrics.rowLeading);
            for (const TableColumn& column : options.columns) {
                const float right = column.width > 0.0f ? x + Px(column.width) : box.Max.x - Px(metrics.rowTrailing);
                Typography::Draw(draw, Font::System(metrics.headerFontSize), ImRect(x, band.Min.y, right, band.Max.y), colors.label, column.title ? column.title : "", column.alignment);
                x = right;
            }
        }
        const float row_height = Px(options.rowHeight);
        // A focused list's selection is accented and turns gray with the window; its rows' content turns white halfway.
        const float focus = Interaction::ListFocus(list_id);
        const bool focused = focus > 0.5f;

        // Row backgrounds: alternate rows darker, the selection across the row (rounded and inset in an inset list), rows
        // in the box's corners following them; separators under rows that do not touch the selection.
        draw->PushClipRect(rows.Min, rows.Max, true);
        for (int row = 0; row < options.rows; ++row) {
            const ImRect rect(rows.Min.x, rows.Min.y + row_height * float(row), rows.Max.x, rows.Min.y + row_height * float(row + 1));
            if (rect.Min.y >= rows.Max.y)
                break;
            const float top = bordered && row == 0 && header == 0.0f ? radius : 0.0f;
            const float bottom = bordered && !options.showsAddRemove && rect.Max.y >= box.Max.y ? radius : 0.0f;
            const Rgba selection_color = Theme::ListSelection(focus);
            if (selected(row) && inset) {
                const float indent = Px(metrics.insetSelectionInset);
                Draw::FillRoundedRect(draw, ImRect(rect.Min.x + indent, rect.Min.y, rect.Max.x - indent, rect.Max.y), CornerRadii(Px(metrics.insetSelectionRadius)), selection_color);
            } else if (selected(row)) {
                Draw::FillRoundedRect(draw, rect, CornerRadii(top, top, bottom, bottom), selection_color, corners);
            } else if (options.alternatesRows && row % 2 == 1) {
                Draw::FillRoundedRect(draw, rect, CornerRadii(top, top, bottom, bottom), colors.quaternaryFill, corners);
            }
            if (options.showsSeparators && !selected(row) && !selected(row + 1))
                Draw::FillRect(draw, ImRect(rect.Min.x, rect.Max.y - Px(1.0f), rect.Max.x, rect.Max.y), colors.quaternaryFill);
        }
        draw->PopClipRect();

        const bool focus_ring = Interaction::ListKeys(list_id, rows, selection, state.anchor);
        ImGui::PushID(list_id);
        for (int row = 0; row < options.rows; ++row) {
            const float top = rows.Min.y + row_height * float(row);
            if (top >= rows.Max.y)
                break;
            const ImRect rect(rows.Min.x, top, rows.Max.x, ImMin(top + row_height, rows.Max.y));
            ListRows::Row(list_id, row, rect, selection, state.anchor, !options.contextMenu.empty(), event);
            if (ListRows::MenuRow(list_id) == row && !selected(row))
                Bezel::MenuTarget(draw, rect, inset ? CornerRadii(Px(metrics.insetSelectionRadius)) : CornerRadii(0.0f));
            Layout::ContainerSpec spec;
            spec.arrangement = Layout::Arrangement::Horizontal;
            spec.spacing = 8.0f;
            spec.padding = EdgeInsets{0.0f, metrics.rowLeading, 0.0f, bordered ? metrics.rowTrailing : metrics.rowLeading};
            spec.role = Layout::Role::ListRow;
            WithEnvironment([&](EnvironmentValues& environment) {
                if (selected(row) && focused)
                    environment.backgroundProminence = BackgroundProminence::Increased;
            }, [&] { Layout::Region(ImGui::GetID(row + options.rows), rect, spec, [&] { content(row); }); });
        }

        ListRows::Menu(list_id, options.contextMenu, event);

        const bool has_selection = selection && std::find(selection->begin(), selection->end(), true) != selection->end();
        if (bar_height > 0.0f) {
            // A line over the bar, the bar tinted inside a bordered box, and the buttons from the leading edge.
            const ImRect bar(box.Min.x, rows.Max.y + Px(1.0f), box.Max.x, box.Max.y);
            if (bordered)
                Draw::FillRoundedRect(draw, bar, CornerRadii(0.0f, 0.0f, radius, radius), colors.quaternaryFill, corners);
            Draw::FillRect(draw, ImRect(box.Min.x, rows.Max.y, box.Max.x, rows.Max.y + Px(1.0f)), bordered ? colors.secondaryFill : colors.listBarLine);
            const float width = Px(bordered ? metrics.buttonWidth : metrics.plainButtonWidth);
            const float bottom = bordered ? bar.Max.y - Px(1.0f) : bar.Max.y;
            const ImRect add(bar.Min.x, bar.Min.y, bar.Min.x + width, bottom);
            const ImRect remove(add.Max.x + Px(1.0f), add.Min.y, add.Max.x + Px(1.0f) + width, bottom);
            // Symbols in the label color on a plain list's white bar, in the secondary label color on a tinted one.
            const ListRows::BarStyle style{.font = Font::System(metrics.buttonSymbolSize, FontWeight::Semibold), .symbol = bordered ? colors.secondaryLabel : colors.label, .divider = bordered ? colors.tertiaryFill : colors.quaternaryFill, .dividerHeight = metrics.buttonDivider};
            if (ListRows::BarButton(ImGui::GetID("add"), add, Symbols::Plus, true, style))
                event.action = ListAction::Add;
            if (ListRows::BarButton(ImGui::GetID("remove"), remove, Symbols::Minus, has_selection, style))
                event.action = ListAction::Remove;
            if (!options.actions.empty()) {
                const ImRect actions(remove.Max.x + Px(1.0f), add.Min.y, remove.Max.x + Px(1.0f + metrics.actionWidth), bottom);
                event.actionItem = ListRows::ActionsButton(ImGui::GetID("actions"), actions, options.actions, bordered ? colors.secondaryLabel : colors.label);
            }
        }
        if (bezel_height > 0.0f) {
            const ImVec2 bezel_min(box.Min.x, box.Max.y + Px(metrics.insetBezelSpacing));
            const ListAction action = BezelButtons(ImRect(bezel_min, bezel_min + Px(metrics.insetBezel)), has_selection);
            if (action != ListAction::None)
                event.action = action;
        }
        ImGui::PopID();
        // The border lies over the rows and the bar.
        if (bordered)
            DrawSectionBorder(draw, box);
        // The ring shows round the list's box while Tab has taken the keyboard to it.
        if (focus_ring)
            Draw::FocusRing(draw, box, CornerRadii(radius), colors.accent, corners);
        return event;
    }
} // namespace Cupertino
