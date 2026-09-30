#include "KeyboardShortcut.h"

#include "imgui.h"

#include <cctype>
#include <cstring>
#include <utility>

namespace Cupertino {
    std::string KeyLabel(const KeyboardShortcut& shortcut) {
        if (shortcut.IsEmpty())
            return {};
        std::string label = shortcut.key;
        if (label.size() == 1)
            label[0] = char(std::toupper(static_cast<unsigned char>(label[0])));
        return label;
    }

    std::string ModifierGlyphs(EventModifiers modifiers) {
        std::string glyphs;
        if (Contains(modifiers, EventModifiers::Control))
            glyphs += "\xE2\x8C\x83";
        if (Contains(modifiers, EventModifiers::Option))
            glyphs += "\xE2\x8C\xA5";
        if (Contains(modifiers, EventModifiers::Shift))
            glyphs += "\xE2\x87\xA7";
        if (Contains(modifiers, EventModifiers::Command))
            glyphs += "\xE2\x8C\x98";
        return glyphs;
    }

    // The ImGui key of a shortcut's key: letters, digits, punctuation and the keys with glyphs.
    static ImGuiKey KeyOf(const char* key) {
        if (key[1] == '\0') {
            const char character = char(std::tolower(static_cast<unsigned char>(key[0])));
            if (character >= 'a' && character <= 'z')
                return ImGuiKey(ImGuiKey_A + (character - 'a'));
            if (character >= '0' && character <= '9')
                return ImGuiKey(ImGuiKey_0 + (character - '0'));
            static const std::pair<char, ImGuiKey> punctuation[] = {{'[', ImGuiKey_LeftBracket}, {']', ImGuiKey_RightBracket}, {',', ImGuiKey_Comma}, {'.', ImGuiKey_Period}, {'/', ImGuiKey_Slash}, {';', ImGuiKey_Semicolon}, {'\'', ImGuiKey_Apostrophe}, {'-', ImGuiKey_Minus}, {'=', ImGuiKey_Equal}, {'`', ImGuiKey_GraveAccent}, {'\\', ImGuiKey_Backslash}, {' ', ImGuiKey_Space}};
            for (const auto& [glyph, imgui_key] : punctuation) {
                if (glyph == character)
                    return imgui_key;
            }
            return ImGuiKey_None;
        }
        static const std::pair<const char*, ImGuiKey> glyphs[] = {{KeyEquivalent::UpArrow, ImGuiKey_UpArrow}, {KeyEquivalent::DownArrow, ImGuiKey_DownArrow}, {KeyEquivalent::LeftArrow, ImGuiKey_LeftArrow}, {KeyEquivalent::RightArrow, ImGuiKey_RightArrow}, {KeyEquivalent::Escape, ImGuiKey_Escape}, {KeyEquivalent::Tab, ImGuiKey_Tab}, {KeyEquivalent::Delete, ImGuiKey_Backspace}, {KeyEquivalent::Return, ImGuiKey_Enter}};
        for (const auto& [glyph, imgui_key] : glyphs) {
            if (std::strcmp(glyph, key) == 0)
                return imgui_key;
        }
        return ImGuiKey_None;
    }

    bool IsPressed(const KeyboardShortcut& shortcut) {
        if (shortcut.IsEmpty() || Contains(shortcut.modifiers, EventModifiers::Control) || Contains(shortcut.modifiers, EventModifiers::Function))
            return false;
        const ImGuiKey key = KeyOf(shortcut.key);
        if (key == ImGuiKey_None || !ImGui::IsKeyPressed(key, false))
            return false;
        const ImGuiIO& io = ImGui::GetIO();
        return io.KeyCtrl == Contains(shortcut.modifiers, EventModifiers::Command) && io.KeyShift == Contains(shortcut.modifiers, EventModifiers::Shift) && io.KeyAlt == Contains(shortcut.modifiers, EventModifiers::Option);
    }

    std::string KeyName(ImGuiKey key) {
        switch (key) {
            case ImGuiKey_MouseLeft:
                return "Mouse 1";
            case ImGuiKey_MouseRight:
                return "Mouse 2";
            case ImGuiKey_MouseMiddle:
                return "Mouse 3";
            case ImGuiKey_MouseX1:
                return "Mouse 4";
            case ImGuiKey_MouseX2:
                return "Mouse 5";
            case ImGuiKey_LeftShift:
            case ImGuiKey_RightShift:
                return "Shift";
            case ImGuiKey_LeftCtrl:
            case ImGuiKey_RightCtrl:
                return "Ctrl";
            case ImGuiKey_LeftAlt:
            case ImGuiKey_RightAlt:
                return "Alt";
            case ImGuiKey_UpArrow:
                return "\xE2\x86\x91";
            case ImGuiKey_DownArrow:
                return "\xE2\x86\x93";
            case ImGuiKey_LeftArrow:
                return "\xE2\x86\x90";
            case ImGuiKey_RightArrow:
                return "\xE2\x86\x92";
            default:
                return ImGui::GetKeyName(key);
        }
    }

    std::string ShortcutName(ImGuiKeyChord shortcut) {
        EventModifiers modifiers = EventModifiers::None;
        if (shortcut & ImGuiMod_Super)
            modifiers = modifiers | EventModifiers::Control;
        if (shortcut & ImGuiMod_Alt)
            modifiers = modifiers | EventModifiers::Option;
        if (shortcut & ImGuiMod_Shift)
            modifiers = modifiers | EventModifiers::Shift;
        if (shortcut & ImGuiMod_Ctrl)
            modifiers = modifiers | EventModifiers::Command;
        const ImGuiKey key = ImGuiKey(shortcut & ~ImGuiMod_Mask_);
        return ModifierGlyphs(modifiers) + (key != ImGuiKey_None ? KeyName(key) : std::string());
    }
} // namespace Cupertino
