#pragma once

#include "core/Bitmap.h"
#include "core/Color.h"
#include "core/KeyboardShortcut.h"
#include "core/Typography.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <functional>

namespace Cupertino {
    enum class ButtonRole {
        Normal,
        // The default button: accent bezel with white text, triggered by Return.
        Default,
        Cancel,
        Destructive,
    };

    enum class ButtonStyle {
        // A bezel: white, or the accent for the default role.
        Bordered,
        // The label alone, like the info buttons in System Settings rows.
        Borderless,
        // The title in the link color with a pointing hand, like SwiftUI's Link ("About HDMI Passthrough…" in TV Settings).
        Link,
        // SwiftUI's .accessoryBar: a scope of an accessory bar, its bold title on a gray plate while pressed (Finder's
        // search scopes); as a toggle's style the plate marks it on.
        AccessoryBar,
        // SwiftUI's .accessoryBarAction: an action of an accessory bar, its small title or symbol in a thin outline
        // (Finder's Save and + beside the search scopes).
        AccessoryBarAction,
    };

    struct ButtonOptions {
        ButtonRole role = ButtonRole::Normal;
        ButtonStyle style = ButtonStyle::Bordered;
        // An SF Symbol: before the title in a menu and on a push button, where a label that is only an id ("##add") leaves
        // the symbol alone; in the other styles it replaces the title.
        unsigned symbol = 0;
        // A menu item's 16 pt picture before its title, in the symbols' column (the app icons of the Share menu @2x).
        Bitmap image;
        float minWidth = 0.0f;
        // Shown by a menu item, and performed from the keyboard by the items of a MenuBar.
        KeyboardShortcut shortcut;
        // The title's font in the borderless and link styles; push buttons keep the body font, as AppKit's do.
        Font font = Font::Style(TextStyle::Body);
        // A borderless symbol's scale: large as an info button (Battery Health @2x), medium as the stop button of
        // Finder's copy window (@2x).
        SymbolScale symbolScale = SymbolScale::Large;
        // A menu item's badge, as SwiftUI's badge(_:) on a Button in a menu (NSMenuItemBadge): a count or a short text in a
        // capsule at the item's end.
        const char* badge = nullptr;
        // A menu item's second line in small secondary type, as NSMenuItem.subtitle ("Can include bookmarks, passwords, and
        // more" under Safari's Import Browsing Data).
        const char* subtitle = nullptr;
        // A menu item shown in place of the item before it while Option is held, as NSMenuItem's alternates (the Apple
        // menu's About This Mac and System Information…); usually its shortcut adds Option.
        bool alternate = false;
    };

    // A push button; returns true when clicked. In a toolbar a symbol button becomes a toolbar item (ToolbarButton),
    // in a menu's content a menu item.
    bool Button(const char* label, const ButtonOptions& options = {});

    // A toolbar item over slot (38 pt tall, centered in the 52 pt bar): a rounded hover plate behind content drawn in the
    // toolbar's symbol color, gray when disabled. Returns true when clicked.
    bool ToolbarButton(const char* id, const ImRect& slot, bool enabled, const std::function<void(ImDrawList*, Rgba)>& content);
    // The plate a toolbar item in slot lights under the pointer, and where its menu opens from.
    ImRect ToolbarPlate(const ImRect& slot);
    // The toolbar item showing a symbol on its middle.
    bool ToolbarButton(const char* id, const ImRect& slot, unsigned symbol, bool enabled = true);
    // A toolbar symbol (13 pt Medium, large scale) centered on a point.
    void DrawToolbarSymbol(ImDrawList* draw, unsigned symbol, ImVec2 center, Rgba color);

    // The round help button with a question mark.
    bool HelpButton(const char* id = "##help");

    // The push button Toggle draws for its button styles: while on, lit shows the accent bezel with a white title,
    // otherwise the title takes the accent. Returns true on the frame the value changes.
    bool ButtonToggle(const char* label, bool* is_on, bool lit);

    // The accessory bar scope Toggle draws for its accessory bar style: on the gray plate while on. Returns true on the
    // frame the value changes.
    bool AccessoryBarToggle(const char* label, bool* is_on);
} // namespace Cupertino
