#pragma once

#include "Bitmap.h"
#include "Color.h"
#include "KeyboardShortcut.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <functional>
#include <span>
#include <string>
#include <string_view>

// The rows of a menu, and menu content written the SwiftUI way: while a menu's content runs, Button, Toggle, Divider,
// Section and Menu add its items, separators, headers and submenus instead of drawing.
namespace Cupertino::MenuContent {
    enum class EntryKind {
        Item,
        Separator,
        // A section title in small gray type (Safari's Move & Resize: Halves, Quarters, Arrange).
        Header,
        // An item that opens the entries after it, children of them.
        Submenu,
    };

    // One row of a menu. A submenu's entries follow it; children counts them together with their own descendants.
    struct Entry {
        EntryKind kind = EntryKind::Item;
        std::string title;
        // A second line under the title in small secondary type (NSMenuItem.subtitle).
        std::string subtitle;
        // An SF Symbol before the title, centered in a column as wide as the menu's widest symbol (Finder's Go menu), or a
        // 16 pt picture in that column (the app icons of the Share menu @2x).
        unsigned symbol = 0;
        Bitmap image;
        // Or a picture painted there as Icon::paint paints one (the folders of a path control's menu).
        void (*paint)(ImDrawList* draw, const ImRect& frame, Rgba color) = nullptr;
        KeyboardShortcut shortcut;
        bool enabled = true;
        // A checkable item (a toggle, a picker's option) keeps the checkmark column open for its whole menu.
        bool checkable = false;
        bool checked = false;
        // A capsule at the item's end with a count or a short text (NSMenuItemBadge).
        std::string badge;
        // Stands in for the item before it while Option is held (NSMenuItem.isAlternate).
        bool alternate = false;
        int children = 0;
    };

    // Whether a menu's content is running, so views add entries to it.
    bool IsCollecting();

    // Runs content to collect the entries of the menu id. The entry chosen from the menu in the frame before reports
    // true from its view, once.
    std::span<const Entry> Collect(ImGuiID id, const std::function<void()>& content);

    // Hands a choice made from the menu (or by its keyboard shortcut) to its view in the next collect.
    void Choose(ImGuiID id, int entry);

    // Adds an item (its kind, children and enablement are the collector's) and returns whether it was chosen; disabled in a
    // disabled environment.
    bool AddItem(Entry item);
    void AddSeparator();
    void AddHeader(std::string_view title);
    // A section after other entries starts with a separator, and the entries after it follow another, as SwiftUI
    // separates a menu's sections.
    void BeginSection(const char* header);
    void EndSection();
    void BeginSubmenu(std::string_view title, unsigned symbol);
    void EndSubmenu();

    // A menu bar takes the menus declared in its content: it installs this while the content runs, and Menu hands its
    // title, symbol and content to it.
    using BarMenu = void (*)(const char* label, unsigned symbol, const std::function<void()>& content);
    BarMenu CurrentBarMenu();
    void SetBarMenu(BarMenu bar_menu);
} // namespace Cupertino::MenuContent
