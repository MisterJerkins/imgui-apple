#pragma once

#include "Stacks.h"

#include <functional>
#include <span>

namespace Cupertino {
    struct GridOptions {
        // Where cells sit in their columns and rows.
        Alignment alignment;
        float horizontalSpacing = 8.0f;
        float verticalSpacing = 8.0f;
    };

    // SwiftUI's Grid: rows of cells whose columns are as wide as their widest cell. Views between the rows span the
    // grid's width, as a Divider does.
    void Grid(const GridOptions& options, const std::function<void()>& rows);
    void Grid(const std::function<void()>& rows);
    // A row of a Grid: each view in it is a cell of the next column.
    void GridRow(const std::function<void()>& cells);

    enum class GridItemSize {
        Fixed,
        Flexible,
        // As many columns as fit, each at least the minimum wide.
        Adaptive,
    };

    // GridItem: .fixed(width), .flexible(minimum:maximum:) or .adaptive(minimum:maximum:), and the space after it.
    struct GridItem {
        GridItemSize size = GridItemSize::Flexible;
        float minimum = 10.0f;
        float maximum = Infinity;
        float spacing = 8.0f;
        HorizontalAlignment alignment = HorizontalAlignment::Center;
    };

    struct LazyVGridOptions {
        // Space between rows in points.
        float spacing = 8.0f;
    };

    // LazyVGrid: count cells laid out row by row in the columns, their widths resolved from the offered width. Rows out
    // of the window's sight keep their height without building their cells.
    void LazyVGrid(std::span<const GridItem> columns, int count, const std::function<void(int index)>& cell, const LazyVGridOptions& options = {});
} // namespace Cupertino
