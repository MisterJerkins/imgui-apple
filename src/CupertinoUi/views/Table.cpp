#include "Table.h"

#include "controls/Bezel.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/State.h"
#include "core/Symbols.h"
#include "core/Theme.h"
#include "layout/Layout.h"
#include "layout/ScrollView.h"
#include "views/Disclosure.h"

#include <algorithm>

namespace Cupertino {
    struct TableState {
        // The row a Shift-click extends the selection from.
        int anchor = 0;
        // The columns' widths in points as dragged, the width a drag started from, and how far the rows are scrolled.
        std::vector<float> widths;
        float dragStart = 0.0f;
        float offset = 0.0f;
    };

    struct ColumnSpan {
        float left = 0.0f;
        float right = 0.0f;
    };

    // Column bounds in pixels: each column ends at its divider and the next starts after it; the last one runs to the
    // table's end.
    static std::vector<ColumnSpan> ColumnSpans(const TableOptions& options, const std::vector<float>& widths, const ImRect& rect) {
        std::vector<ColumnSpan> spans;
        float x = rect.Min.x + (options.style == TableStyle::Inset ? Px(options.leading) : 0.0f);
        const int count = int(widths.size());
        for (int index = 0; index < count; ++index) {
            const float right = index + 1 < count && widths[size_t(index)] > 0.0f ? x + Px(widths[size_t(index)]) : rect.Max.x;
            spans.push_back({x, right});
            x = right + Px(Metrics::Table().divider);
        }
        return spans;
    }

    // The header: the titles, the sort chevron and the dividers. A click on a title sorts by its column, a drag on a
    // divider resizes the column before it; returns whether the sort order changed.
    static bool Header(ImDrawList* draw, const TableOptions& options, TableState& state, const std::vector<ColumnSpan>& spans, const ImRect& header) {
        const Metrics::TableMetrics& metrics = Metrics::Table();
        const Palette& colors = Theme::Colors();
        const bool full_width = options.style == TableStyle::FullWidth;
        if (full_width)
            Draw::FillRect(draw, header, colors.quaternaryFill);
        // Titles are 11 pt with proportional digits, even when the cells line theirs up.
        const Font font = Font::Style(TextStyle::Subheadline);
        const int count = int(spans.size());
        const float grab = Px(metrics.dividerGrab);
        bool sorted_now = false;
        int index = 0;
        for (const TableColumn& column : options.columns) {
            const ColumnSpan& span = spans[size_t(index)];
            ImGui::PushID(index);
            const bool resizable = options.resizableColumns && index + 1 < count && column.width > 0.0f;
            if (resizable) {
                const Interaction::Response divider = Interaction::Button(ImGui::GetID("##divider"), ImRect(span.right - grab, header.Min.y, span.right + Px(metrics.divider) + grab, header.Max.y), ImGuiButtonFlags_PressedOnClick, ImGuiItemFlags_NoNav);
                if (divider.hovered || divider.held)
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                if (divider.pressed)
                    state.dragStart = state.widths[size_t(index)];
                if (divider.held) {
                    const float minimum = column.minWidth > 0.0f ? column.minWidth : metrics.minColumnWidth;
                    const float maximum = column.maxWidth > 0.0f ? column.maxWidth : FLT_MAX;
                    state.widths[size_t(index)] = ImClamp(state.dragStart + Pt(ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 0.0f).x), minimum, maximum);
                }
            }
            if (options.sort && column.title) {
                const ImRect cell(span.left + (index > 0 ? grab : 0.0f), header.Min.y, span.right - (resizable ? grab : 0.0f), header.Max.y);
                if (Interaction::Button(ImGui::GetID("##sort"), cell, 0, ImGuiItemFlags_NoNav).pressed) {
                    *options.sort = options.sort->column == index ? TableSort{index, !options.sort->ascending} : TableSort{index, true};
                    sorted_now = true;
                }
            }
            ImGui::PopID();
            // Dividers stand between columns, 16 pt tall on the header's middle.
            if (index + 1 < count && !full_width) {
                const float half = Px(metrics.dividerHeight) * 0.5f;
                Draw::FillRect(draw, ImRect(span.right, header.GetCenter().y - half, span.right + Px(metrics.divider), header.GetCenter().y + half), colors.tableDivider);
            }
            if (column.title) {
                const bool sorted = options.sort && index == options.sort->column;
                float title_right = span.right - Px(metrics.titleInset);
                if (sorted) {
                    // The sort chevron sits at the end of the header cell; the title keeps to its side.
                    const Font chevron_font = Font::System(metrics.sortChevronSize, FontWeight::Semibold);
                    const unsigned chevron = options.sort->ascending ? Symbols::ChevronUp : Symbols::ChevronDown;
                    const float chevron_width = Typography::SymbolWidth(chevron, chevron_font);
                    const float chevron_right = span.right - Px(metrics.sortChevronInset);
                    Typography::DrawSymbol(draw, chevron, chevron_font, ImRect(chevron_right - chevron_width, header.Min.y, chevron_right, header.Max.y), colors.tableSortIndicator);
                    title_right = chevron_right - chevron_width - Px(metrics.sortChevronSpacing);
                }
                const ImRect title(span.left + Px(full_width ? metrics.fullWidthTitleInset : metrics.titleInset), header.Min.y, title_right, header.Max.y);
                Typography::Draw(draw, sorted ? font.Weight(FontWeight::Semibold) : font, title, colors.label, column.title, column.alignment);
            }
            ++index;
        }
        Draw::FillRect(draw, ImRect(header.Min.x, header.Max.y - Px(metrics.headerLine), header.Max.x, header.Max.y), full_width ? colors.rowSeparator : colors.tableDivider);
        return sorted_now;
    }

    // The rows over area, whose top is the first row's less the style's top padding, drawing those in visible: stripes,
    // the selection a run of rows shares, the ring of a row whose menu is open, outline chevrons and the cells.
    static void Rows(ImGuiID table_id, const TableOptions& options, TableState& state, std::vector<bool>* selection, const std::vector<ColumnSpan>& spans, const ImRect& area, const ImRect& visible, float focus, ListEvent& event, const std::function<void(const TableCell&)>& cell) {
        const Metrics::TableMetrics& metrics = Metrics::Table();
        const Palette& colors = Theme::Colors();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const bool full_width = options.style == TableStyle::FullWidth;
        const bool bordered = options.style == TableStyle::Bordered;
        const bool square = full_width || bordered;
        const float inset = square ? 0.0f : Px(metrics.rowInset);
        const float top_padding = square ? 0.0f : Px(metrics.contentTop);
        const float row_height = Px(options.rowHeight > 0.0f ? options.rowHeight : bordered ? metrics.borderedRowHeight : metrics.rowHeight);
        const CornerRadii radii(square ? 0.0f : Px(metrics.rowRadius));
        const auto row_rect = [&](int row) {
            const float top = area.Min.y + top_padding + row_height * float(row);
            return ImRect(area.Min.x + inset, top, area.Max.x - inset, top + row_height);
        };
        const auto selected = [&](int row) { return selection && row >= 0 && row < options.rows && (*selection)[size_t(row)]; };
        const int first = ImMax(0, int((visible.Min.y - area.Min.y - top_padding) / row_height));
        const auto shown = [&](int row) { return row_rect(row).Min.y < visible.Max.y; };

        // Stripes behind unselected odd rows; a run of selected rows is one rounded shape split by light lines.
        for (int row = first; (row < options.rows || (bordered && options.alternatesRows)) && shown(row); ++row) {
            if (options.alternatesRows && row % 2 == 1 && !selected(row))
                Draw::FillRoundedRect(draw, row_rect(row), radii, full_width ? colors.quaternaryFill : colors.tableStripe);
        }
        const Rgba selection_color = Theme::ListSelection(focus);
        int run_start = first;
        while (run_start > 0 && selected(run_start - 1))
            --run_start;
        for (int row = run_start; row < options.rows && shown(row); ++row) {
            if (!selected(row))
                continue;
            int end = row;
            while (selected(end + 1))
                ++end;
            const ImRect run(row_rect(row).Min, row_rect(end).Max);
            Draw::FillRoundedRect(draw, run, radii, selection_color);
            for (int split = row + 1; split <= end; ++split) {
                const float y = row_rect(split).Min.y;
                Draw::FillRect(draw, ImRect(run.Min.x, y - Px(metrics.selectionSplit), run.Max.x, y), colors.tableSelectionSplit);
            }
            row = end;
        }

        ImGui::PushID(table_id);
        const bool outline = size_t(options.rows) <= options.levels.size();
        const float leading = Px(full_width ? metrics.fullWidthCellLeading : bordered ? metrics.borderedCellLeading : metrics.cellLeading);
        for (int row = first; row < options.rows && shown(row); ++row) {
            const ImRect bounds = row_rect(row);
            // An outline row's chevron takes its clicks before the row.
            const int level = outline ? options.levels[size_t(row)] : 0;
            const bool expandable = outline && size_t(row) < options.expandable.size() && options.expandable[size_t(row)];
            const ImVec2 chevron(bounds.Min.x + Px(metrics.outlineChevronX + metrics.outlineIndent * float(level)), bounds.GetCenter().y);
            if (expandable && options.expanded) {
                options.expanded->resize(size_t(options.rows), false);
                const float half = Px(metrics.outlineIndent) * 0.5f;
                if (Interaction::Button(ImGui::GetID(-1 - row), ImRect(chevron.x - half, bounds.Min.y, chevron.x + half, bounds.Max.y)).pressed)
                    (*options.expanded)[size_t(row)] = !(*options.expanded)[size_t(row)];
            }
            if (selection)
                ListRows::Row(table_id, row, bounds, selection, state.anchor, !options.contextMenu.empty(), event);
            if (ListRows::MenuRow(table_id) == row && !selected(row))
                Bezel::MenuTarget(draw, bounds, radii);
            if (expandable) {
                const bool open = options.expanded && (*options.expanded)[size_t(row)];
                const Rgba color = selected(row) && focus > 0.5f ? colors.selectedContent : colors.secondaryLabel;
                DrawOutlineChevron(draw, chevron, Font::System(metrics.outlineChevronSize, FontWeight::Bold), DisclosureTurn(ImGui::GetID(-1 - row), open), color);
            }
            int column = 0;
            for (const TableColumn& spec : options.columns) {
                const ColumnSpan& span = spans[size_t(column)];
                const float bottom = bordered ? bounds.Min.y + Px(metrics.borderedCellHeight) : bounds.Max.y;
                // The first column's content steps in with the outline level.
                const float indent = column == 0 ? Px(metrics.outlineIndent * float(level)) : 0.0f;
                const ImRect content(span.left + leading + indent, bounds.Min.y, span.right - Px(metrics.cellTrailing), bottom);
                cell(TableCell{.row = row, .column = column, .rect = content, .bounds = ImRect(span.left, bounds.Min.y, span.right, bounds.Max.y), .alignment = spec.alignment, .font = options.font, .selected = selected(row), .emphasized = selected(row) && focus > 0.5f});
                ++column;
            }
        }
        ListRows::Menu(table_id, options.contextMenu, event);
        ImGui::PopID();
    }

    ListEvent Table(const char* id, const TableOptions& options, std::vector<bool>* selection, const std::function<void(const TableCell&)>& cell) {
        const Metrics::TableMetrics& metrics = Metrics::Table();
        const bool full_width = options.style == TableStyle::FullWidth;
        const bool bordered = options.style == TableStyle::Bordered;
        Layout::Placement placement;
        placement.size = Layout::Proposal();
        placement.flexibleWidth = true;
        placement.flexibleHeight = true;
        if (full_width) {
            placement.size = ImVec2(Layout::FullProposal().x, Px(metrics.headerHeight + metrics.rowHeight * float(options.rows)));
            placement.flexibleHeight = false;
            placement.ignoresChildInsets = true;
            placement.fullWidthSeparator = true;
        }
        placement.baseline = Typography::Baseline(options.font);
        const ImRect rect = Layout::Place(placement);
        ListEvent event;
        if (Layout::IsMeasuring())
            return event;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const ImGuiID table_id = ImGui::GetID(id);
        TableState& state = State::Get<TableState>(table_id);
        if (selection && int(selection->size()) != options.rows)
            selection->resize(size_t(options.rows), false);
        if (state.widths.size() != options.columns.size()) {
            state.widths.clear();
            for (const TableColumn& column : options.columns)
                state.widths.push_back(column.width);
        }

        if (!full_width)
            Draw::FillRect(draw, rect, colors.tableBackground);
        // A bordered table has its rows inside the border and no header.
        const ImRect inside = bordered ? ImRect(rect.Min + Px(ImVec2(metrics.border, metrics.border)), rect.Max - Px(ImVec2(metrics.border, metrics.border))) : rect;
        const std::vector<ColumnSpan> spans = ColumnSpans(options, state.widths, inside);
        const bool headed = !bordered && options.columnHeaders;
        const ImRect header(inside.Min.x, inside.Min.y, inside.Max.x, inside.Min.y + (headed ? Px(metrics.headerHeight) : 0.0f));
        ImGui::PushID(table_id);
        if (headed)
            event.sortChanged = Header(draw, options, state, spans, header);
        ImGui::PopID();

        const bool has_bar = bordered && options.showsAddRemove;
        const ImRect body(inside.Min.x, header.Max.y, inside.Max.x, inside.Max.y - (has_bar ? Px(metrics.barHeight) : 0.0f));
        const bool focus_ring = Interaction::ListKeys(table_id, body, selection, state.anchor);
        // The focused table's selection is accented and turns gray with the window; its rows' text turns white halfway.
        const float focus = Interaction::ListFocus(table_id, options.initialFocus);
        if (full_width) {
            draw->PushClipRect(body.Min, body.Max, true);
            Rows(table_id, options, state, selection, spans, body, body, focus, event, cell);
            draw->PopClipRect();
        } else {
            // The rows scroll under the header; a row the keys move to comes into view.
            const float row_height = options.rowHeight > 0.0f ? options.rowHeight : bordered ? metrics.borderedRowHeight : metrics.rowHeight;
            const float top_padding = bordered ? 0.0f : metrics.contentTop;
            const int key_row = Interaction::ListKeyRow(table_id);
            if (key_row >= 0) {
                const float top = top_padding + row_height * float(key_row);
                state.offset = ImClamp(state.offset, top + row_height - Pt(body.GetHeight()), top);
            }
            const float content_height = 2.0f * top_padding + row_height * float(options.rows);
            Layout::ContainerSpec region;
            region.alignment = Alignment{HorizontalAlignment::Leading, VerticalAlignment::Top};
            WithEnvironment([](EnvironmentValues& environment) { environment.scrollEdge = 0; }, [&] {
                Layout::Region(ImHashStr("##rows", 0, table_id), body, region, [&] {
                    ScrollView({.offset = &state.offset}, [&] {
                        const ImRect area = Layout::Place(ImVec2(Layout::Proposal().x, Px(content_height)));
                        if (!Layout::IsMeasuring())
                            Rows(table_id, options, state, selection, spans, ImRect(body.Min.x, area.Min.y, body.Max.x, area.Max.y), ImGui::GetCurrentWindow()->InnerRect, focus, event, cell);
                    });
                });
            });
        }
        if (has_bar) {
            // The line over the bar, then + from the table's edge and − after it.
            Draw::FillRect(draw, ImRect(body.Min.x, body.Max.y, body.Max.x, body.Max.y + Px(1.0f)), colors.listBarLine);
            const ListRows::BarStyle style{.font = Font::System(metrics.barSymbolSize), .symbol = colors.label, .divider = colors.listBarLine};
            const ImRect add(rect.Min.x, body.Max.y + Px(1.0f), rect.Min.x + Px(metrics.barButtons.x), inside.Max.y);
            const ImRect remove(add.Max.x + Px(1.0f), add.Min.y, add.Max.x + Px(1.0f + metrics.barButtons.y), add.Max.y);
            const bool has_selection = selection && std::find(selection->begin(), selection->end(), true) != selection->end();
            ImGui::PushID(table_id);
            if (ListRows::BarButton(ImGui::GetID("add"), add, Symbols::Plus, true, style))
                event.action = ListAction::Add;
            if (ListRows::BarButton(ImGui::GetID("remove"), remove, Symbols::Minus, has_selection, style))
                event.action = ListAction::Remove;
            ImGui::PopID();
        }
        if (bordered) {
            Draw::StrokeRoundedRect(draw, rect, CornerRadii(0.0f), has_bar ? colors.listBarLine : colors.separator, Px(metrics.border), StrokeAlignment::Inside);
            if ((options.focusRing || focus_ring) && focus > 0.0f) {
                const Draw::Opacity ring(focus);
                Draw::FocusRing(draw, rect, CornerRadii(0.0f), colors.accent);
            }
        } else if (focus_ring) {
            Draw::FocusRing(draw, rect, CornerRadii(0.0f), colors.accent);
        }
        return event;
    }

    void TableText(const TableCell& cell, std::string_view text, bool dimmed) {
        const Palette& colors = Theme::Colors();
        const Rgba color = cell.emphasized ? colors.selectedContent : dimmed || !Environment().enabled ? colors.tertiaryLabel : colors.label;
        Typography::Draw(ImGui::GetWindowDrawList(), cell.font, cell.rect, color, text, cell.alignment);
    }
} // namespace Cupertino
