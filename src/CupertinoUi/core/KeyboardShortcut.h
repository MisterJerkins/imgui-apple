#pragma once

#include "imgui.h"

#include <string>

namespace Cupertino {
    // Modifier keys of a keyboard shortcut, as SwiftUI's EventModifiers; menus show them in the order ⌃ ⌥ ⇧ fn ⌘.
    enum class EventModifiers : unsigned {
        None = 0,
        Command = 1 << 0,
        Shift = 1 << 1,
        Option = 1 << 2,
        Control = 1 << 3,
        Function = 1 << 4,
    };

    constexpr EventModifiers operator|(EventModifiers a, EventModifiers b) {
        return EventModifiers(unsigned(a) | unsigned(b));
    }

    constexpr bool Contains(EventModifiers modifiers, EventModifiers modifier) {
        return (unsigned(modifiers) & unsigned(modifier)) != 0;
    }

    // Keys that menus show as a glyph, as SwiftUI's KeyEquivalent (UTF-8).
    namespace KeyEquivalent {
        inline constexpr const char* UpArrow = "\xE2\x96\xB2";
        inline constexpr const char* DownArrow = "\xE2\x96\xBC";
        inline constexpr const char* LeftArrow = "\xE2\x97\x80";
        inline constexpr const char* RightArrow = "\xE2\x96\xB6";
        inline constexpr const char* Escape = "\xE2\x8E\x8B";
        inline constexpr const char* Tab = "\xE2\x87\xA5";
        inline constexpr const char* Delete = "\xE2\x8C\xAB";
        inline constexpr const char* Return = "\xE2\x86\xA9";
    } // namespace KeyEquivalent

    // SwiftUI's keyboardShortcut(_:modifiers:): a menu item shows it at its trailing edge, the menu bar performs it.
    struct KeyboardShortcut {
        // A character ("n", "[") or a KeyEquivalent; null for none.
        const char* key = nullptr;
        EventModifiers modifiers = EventModifiers::Command;

        bool IsEmpty() const {
            return !key || !*key;
        }
    };

    // The key as a menu shows it: letters in capitals.
    std::string KeyLabel(const KeyboardShortcut& shortcut);

    // The modifiers as menus show them, fn excepted (it is the globe symbol): "⌃⌥⇧⌘".
    std::string ModifierGlyphs(EventModifiers modifiers);

    // How a recorder names a key: mouse buttons counted from 1, modifiers by their names on Windows keyboards, arrows as
    // arrows.
    std::string KeyName(ImGuiKey key);

    // A shortcut as recorders and menus show it: its modifiers as glyphs in the order ⌃ ⌥ ⇧ ⌘, then the key. Ctrl is ⌘ and
    // Alt is ⌥ as in menus, the Windows key is ⌃.
    std::string ShortcutName(ImGuiKeyChord shortcut);

    // Whether the shortcut's key went down this frame with exactly its modifiers held. On Windows keyboards ⌘ is Ctrl and
    // ⌥ is Alt; ⌃ and fn have no key.
    bool IsPressed(const KeyboardShortcut& shortcut);
} // namespace Cupertino
