#pragma once

#include "ListRows.h"
#include "core/Typography.h"

#include <functional>
#include <initializer_list>
#include <span>
#include <string_view>
#include <vector>

namespace Cupertino {
    struct TableColumn {
        const char* title = nullptr;
        // Width in points up to the column's divider (1 pt, before the next column); the last column takes what is left.
        float width = 0.0f;
        TextAlignment alignment = TextAlignment::Leading;
        // How narrow and wide a drag on the divider after the column makes it, in points; 0 leaves it unbounded (the
        // narrowest is the table's minimum column).
        float minWidth = 0.0f;
        float maxWidth = 0.0f;
    };

    enum class TableStyle {
        // Activity Monitor @2x: on white, rows 10 pt in from the sides with a rounded selection, dividers between the
        // header's titles.
        Inset,
        // NSTableView's full-width style in a form section (Sound @2x): on the section's background, rows and selection
        // edge to edge under a tinted header without dividers, the section's separator over it edge to edge too; the
        // table is as tall as its rows.
        FullWidth,
        // A table without a header in a scroll view with a line border (Force Quit @2x): white inside a 1 pt border in
        // the separator color, 22 pt rows from the top edge, a square selection across them.
        Bordered,
    };

    // SwiftUI's sort order of a Table: the column its rows are sorted by and the direction.
    struct TableSort {
        int column = -1;
        bool ascending = false;
    };

    struct TableOptions {
        TableStyle style = TableStyle::Inset;
        std::initializer_list<TableColumn> columns;
        int rows = 0;
        // + and − in a bar under a bordered table's rows, inside its border, as AppKit's square buttons (TV Settings @2x);
        // the border turns the darker line of the bar then. ListEvent::action is the one clicked.
        bool showsAddRemove = false;
        // The sort order: the column's title turns semibold with a chevron at the header's end, chevron.up ascending and
        // chevron.down descending (Activity Monitor @2x). A click on another title sorts by it, on the same one reverses
        // it, and the caller sorts its rows.
        TableSort* sort = nullptr;
        // The header over the columns; SwiftUI's tableColumnHeaders(.hidden) turns it off.
        bool columnHeaders = true;
        // Columns with a width take a drag on the divider after them.
        bool resizableColumns = true;
        // Where the first column starts in the inset style, in points; the rows' own inset stays 10 pt.
        float leading = 16.5f;
        // Rows in alternating colors, as NSTableView's usesAlternatingRowBackgroundColors; in the bordered style they
        // run on past the last row to the bottom (Connect to Server @2x).
        bool alternatesRows = true;
        // Rows this many points apart instead of the style's (Connect to Server @2x: AppKit's classic 19).
        float rowHeight = 0.0f;
        // The cells' font; Activity Monitor sets its table in 11 pt. Header titles stay 11 pt, as NSTableHeaderView's.
        Font font = Font::Style(TextStyle::Body);
        // The table is its window's first responder until another list or table takes a click (Activity Monitor); a
        // table otherwise draws its selection in the accent only after a click in it.
        bool initialFocus = false;
        // The focus ring around a bordered table while it has the keyboard focus, as NSTableView's default focus ring
        // type (Connect to Server @2x); SwiftUI's lists and Force Quit's table go without.
        bool focusRing = false;
        // An outline in the first column, as a Table with children (kit's Lists/Tables): each row's depth and whether it
        // can expand; a click on a row's chevron flips it in expanded, and the caller lists the rows under expanded ones.
        std::span<const int> levels;
        std::span<const bool> expandable;
        std::vector<bool>* expanded = nullptr;
        // Items of the menu a right click on a row opens; ListEvent::row is the row clicked.
        std::span<const char* const> contextMenu;
    };

    // One cell being drawn: its content rectangle in pixels (the column less its padding), the whole column span of the
    // row (an outline column puts its disclosure triangle there), the column's alignment and the state of its row.
    struct TableCell {
        int row = 0;
        int column = 0;
        ImRect rect;
        ImRect bounds;
        TextAlignment alignment = TextAlignment::Leading;
        Font font;
        bool selected = false;
        // Selected in the focused table: the content turns white on the accent.
        bool emphasized = false;
    };

    // A table in macOS 15's inset style (Activity Monitor @2x): a 28 pt header with dividers over rows that scroll, 24 pt
    // rows 10 pt in from the sides, stripes and a rounded selection neighbouring rows share, or the full-width style.
    // selection holds a flag per row (a click selects one, Command toggles, Shift extends); cell draws the visible cells.
    // A label beside the table lines up with its top, as with a line of the table's font there.
    ListEvent Table(const char* id, const TableOptions& options, std::vector<bool>* selection, const std::function<void(const TableCell&)>& cell);

    // The text of a cell in its column's alignment: white on the selection, tertiary when dimmed.
    void TableText(const TableCell& cell, std::string_view text, bool dimmed = false);
} // namespace Cupertino
