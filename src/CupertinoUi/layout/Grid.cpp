#include "Grid.h"

#include "core/Environment.h"
#include "core/State.h"

#include <vector>

namespace Cupertino {
    // Column widths of a grid: the ones in use, and the ones its rows measure now for the next pass.
    struct GridColumns {
        ImVector<float> applied;
        ImVector<float> measured;
    };

    struct ActiveGrid {
        GridColumns* columns = nullptr;
        GridOptions options;
    };

    static std::vector<ActiveGrid>& ActiveGrids() {
        static std::vector<ActiveGrid> grids;
        return grids;
    }

    void Grid(const GridOptions& options, const std::function<void()>& rows) {
        GridColumns& columns = State::Get<GridColumns>(Layout::NextViewId());
        if (!columns.measured.empty())
            columns.applied = columns.measured;
        columns.measured.resize(0);
        ActiveGrids().push_back({&columns, options});
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Vertical;
        spec.spacing = options.verticalSpacing;
        spec.alignment.horizontal = HorizontalAlignment::Leading;
        Layout::Container(spec, rows);
        ActiveGrids().pop_back();
    }

    void Grid(const std::function<void()>& rows) {
        Grid(GridOptions{}, rows);
    }

    void GridRow(const std::function<void()>& cells) {
        IM_ASSERT(!ActiveGrids().empty() && "GridRow belongs in a Grid");
        const ActiveGrid& grid = ActiveGrids().back();
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Horizontal;
        spec.spacing = grid.options.horizontalSpacing;
        spec.alignment.vertical = grid.options.alignment.vertical;
        spec.columns = &grid.columns->applied;
        spec.measuredColumns = &grid.columns->measured;
        spec.columnAlignment = grid.options.alignment.horizontal;
        Layout::Container(spec, cells);
    }

    struct ResolvedColumn {
        float width = 0.0f;
        float spacing = 0.0f;
        HorizontalAlignment alignment = HorizontalAlignment::Center;
    };

    // The columns across the width in points: fixed items keep their size, an adaptive item repeats as often as its
    // minimum fits in what the others leave, flexible items share the rest within their bounds.
    static std::vector<ResolvedColumn> ResolveColumns(std::span<const GridItem> items, float width) {
        float taken = 0.0f;
        int flexible = 0;
        for (size_t i = 0; i < items.size(); ++i) {
            if (items[i].size == GridItemSize::Fixed)
                taken += items[i].minimum;
            else if (items[i].size == GridItemSize::Flexible)
                ++flexible;
            if (i + 1 < items.size())
                taken += items[i].spacing;
        }
        std::vector<ResolvedColumn> columns;
        float adaptive_total = 0.0f;
        std::vector<ResolvedColumn> adaptive;
        for (const GridItem& item : items) {
            if (item.size != GridItemSize::Adaptive || !adaptive.empty())
                continue;
            const float space = ImMax(width - taken, item.minimum);
            const int count = ImMax(1, int((space + item.spacing) / (item.minimum + item.spacing)));
            const float each = ImClamp((space - item.spacing * float(count - 1)) / float(count), item.minimum, item.maximum);
            adaptive.assign(size_t(count), ResolvedColumn{each, item.spacing, item.alignment});
            adaptive_total = each * float(count) + item.spacing * float(count - 1);
        }
        const float share = flexible > 0 ? ImMax(0.0f, width - taken - adaptive_total) / float(flexible) : 0.0f;
        for (const GridItem& item : items) {
            if (item.size == GridItemSize::Adaptive) {
                columns.insert(columns.end(), adaptive.begin(), adaptive.end());
                adaptive.clear();
            } else {
                const float column_width = item.size == GridItemSize::Fixed ? item.minimum : ImClamp(share, item.minimum, item.maximum);
                columns.push_back({column_width, item.spacing, item.alignment});
            }
        }
        return columns;
    }

    // Row heights of a lazy grid and where its top was, in the window's content coordinates.
    struct LazyGridState {
        ImVector<float> rowHeights;
        float top = 0.0f;
        bool placed = false;
    };

    void LazyVGrid(std::span<const GridItem> items, int count, const std::function<void(int index)>& cell, const LazyVGridOptions& options) {
        const ImGuiID id = Layout::NextViewId();
        LazyGridState& state = State::Get<LazyGridState>(id);
        const float width = Layout::Proposal().x / Environment().Scale();
        const std::vector<ResolvedColumn> columns = ResolveColumns(items, width);
        if (columns.empty())
            return;
        const int per_row = int(columns.size());
        const int rows = (count + per_row - 1) / per_row;
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        const float row_spacing = Px(options.spacing);
        float row_top = window->Pos.y - window->Scroll.y + state.top;

        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Vertical;
        spec.spacing = options.spacing;
        spec.alignment.horizontal = HorizontalAlignment::Leading;
        Layout::Container(spec, [&] {
            if (state.rowHeights.Size < rows)
                state.rowHeights.resize(rows, -1.0f);
            for (int row = 0; row < rows; ++row) {
                const float height = state.rowHeights[row];
                const bool hidden = state.placed && height >= 0.0f && !Layout::IsMeasuring() && (row_top > window->ClipRect.Max.y || row_top + height < window->ClipRect.Min.y);
                if (hidden) {
                    Layout::Place(ImVec2(Px(width), height));
                } else {
                    ImGui::PushID(row);
                    Layout::ContainerSpec row_spec;
                    row_spec.arrangement = Layout::Arrangement::Horizontal;
                    Layout::Container(row_spec, [&] {
                        for (int column = 0; column < per_row; ++column) {
                            const int index = row * per_row + column;
                            if (index >= count)
                                break;
                            const ResolvedColumn& resolved = columns[size_t(column)];
                            Frame({.width = resolved.width, .alignment = {resolved.alignment, VerticalAlignment::Center}}, [&] { cell(index); });
                            if (column + 1 < per_row)
                                Frame({.width = resolved.spacing}, [] {});
                        }
                    }, [&](const Layout::ContainerFrame& frame) { state.rowHeights[row] = frame.rect.GetHeight(); });
                    ImGui::PopID();
                }
                row_top += (height >= 0.0f ? height : 0.0f) + row_spacing;
            }
        }, [&](const Layout::ContainerFrame& frame) {
            state.top = frame.rect.Min.y - window->Pos.y + window->Scroll.y;
            state.placed = true;
        });
    }
} // namespace Cupertino
