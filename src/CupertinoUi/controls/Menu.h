#pragma once

#include <functional>
#include <initializer_list>
#include <span>

namespace Cupertino {
    struct MenuOptions {
        // A symbol: in a toolbar the button shows it with a chevron.down after it, in the menu bar it is the title (the
        // Apple menu), in a menu it stands before the submenu's title; elsewhere it is the pull-down button's title.
        unsigned symbol = 0;
        // The chevron in the label color without the accent plate, after the symbol or the title: System Settings'
        // action pull-downs (Network @2x) and Add Photo (Wallpaper @2x); the add pull-down of Displays keeps the plate.
        bool plainIndicator = false;
    };

    // A pull-down button, as SwiftUI's Menu draws on macOS: the label stays, the menu of actions opens under the button.
    // Returns the index of the action chosen this frame, or -1.
    int Menu(const char* label, std::span<const char* const> actions, const MenuOptions& options = {});
    int Menu(const char* label, std::initializer_list<const char*> actions, const MenuOptions& options = {});

    // A menu written with views, as SwiftUI's Menu: Button, Toggle, Divider, Section and Menu inside content add items,
    // checkmark items, separators, sections and submenus. On its own it is a pull-down button; inside another menu it is
    // a submenu, inside MenuBar one of the bar's menus.
    void Menu(const char* label, const std::function<void()>& content);
    void Menu(const char* label, const MenuOptions& options, const std::function<void()>& content);

    // SwiftUI's contextMenu: a right click on content opens menu, written like a Menu's content, at the pointer. A sidebar
    // row keeps the accent ring round its platter while the menu is open (Mail's sidebar @2x).
    void ContextMenu(const std::function<void()>& menu, const std::function<void()>& content);

    // SwiftUI's ControlGroup on macOS: the Buttons and Menus in content become the momentary segments of one bezel, a
    // menu's segment with a chevron after its symbol that pulls its menu down (Connect to Server @2x: +, − and an action
    // menu). As in a menu, a Button reports its click in the next frame.
    void ControlGroup(const std::function<void()>& content);
} // namespace Cupertino
