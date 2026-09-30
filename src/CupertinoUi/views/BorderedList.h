#pragma once

#include "ListRows.h"
#include "Table.h"

#include <functional>
#include <initializer_list>
#include <span>
#include <vector>

namespace Cupertino {
    enum class ListStyle {
        // .bordered: rows in the box of a form section, + and − in a tinted bar inside it (File Sharing @2x).
        Bordered,
        // .plain: rows edge to edge on the table background with a hairline under the list, + and − in a white bar under a
        // line (kit Lists/Small List, Example 1).
        Plain,
        // .inset: rows in a box with 10 pt corners, the selection rounded 4 pt in, + and − in a bezel under the box (kit
        // Lists/Small List, Example 2).
        Inset,
    };

    struct BorderedListOptions {
        ListStyle style = ListStyle::Bordered;
        int rows = 0;
        // Rows 24 pt apart, 29 when they hold controls (the Users list of File Sharing).
        float rowHeight = 24.0f;
        // Height of the box in points; 0 takes the offered height. The list takes the offered width.
        float height = 0.0f;
        bool alternatesRows = true;
        // Lines between rows, left out next to the selection (kit Lists/Small List).
        bool showsSeparators = false;
        // + and − buttons in a bar under the rows.
        bool showsAddRemove = false;
        // Items of a gear pull-down after + and − in the bar (kit Lists/Small List); ListEvent::actionItem is the one chosen.
        std::span<const char* const> actions;
        // Column titles in a tinted header over the rows (Open at Login in Login Items @2x); the rows lay their cells out
        // themselves.
        std::initializer_list<TableColumn> columns;
        // A title over the rows in the section header's weight, on a row of its own with a line under it (Preferred
        // Languages in Language & Region @2x).
        const char* title = nullptr;
        // Items of the menu a right click on a row opens; ListEvent::row is the row clicked.
        std::span<const char* const> contextMenu;
    };

    // .listStyle(.bordered(alternatesRowBackgrounds: true)) as System Settings lists shared folders: rows in a form
    // section's box with the selection across the row and + and − under them, or the kit's plain and inset lists.
    // content(row) lays a row out from its leading edge; the list clicked last has the accent selection, others gray.
    ListEvent BorderedList(const char* id, const BorderedListOptions& options, std::vector<bool>* selection, const std::function<void(int row)>& content);
} // namespace Cupertino
