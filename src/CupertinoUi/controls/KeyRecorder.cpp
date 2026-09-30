#include "KeyRecorder.h"

#include "Bezel.h"
#include "LabeledContent.h"
#include "TextField.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/KeyboardShortcut.h"
#include "core/Metrics.h"
#include "core/State.h"
#include "core/Symbols.h"
#include "core/Theme.h"
#include "core/Typography.h"

#include <string>

namespace Cupertino {
    struct RecorderState {
        bool recording = false;
    };

    static bool IsModifierKey(ImGuiKey key) {
        return key == ImGuiKey_LeftCtrl || key == ImGuiKey_RightCtrl || key == ImGuiKey_LeftShift || key == ImGuiKey_RightShift || key == ImGuiKey_LeftAlt || key == ImGuiKey_RightAlt || key == ImGuiKey_LeftSuper || key == ImGuiKey_RightSuper;
    }

    // The keys a recorder takes: any named key or mouse button but the left one, which clicks, and the wheel; a shortcut
    // recorder leaves the modifier keys to the shortcut's key.
    static bool Recordable(ImGuiKey key, bool chords) {
        return key != ImGuiKey_MouseLeft && key != ImGuiKey_MouseWheelX && key != ImGuiKey_MouseWheelY && !(key >= ImGuiKey_ReservedForModCtrl && key <= ImGuiKey_ReservedForModSuper) && !(chords && IsModifierKey(key));
    }

    // The key pressed this frame, as the recorder owning the keys sees it.
    static ImGuiKey PressedKey(ImGuiID owner, bool chords) {
        for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; ++key) {
            if (Recordable(ImGuiKey(key), chords) && ImGui::IsKeyPressed(ImGuiKey(key), ImGuiInputFlags_None, owner))
                return ImGuiKey(key);
        }
        return ImGuiKey_None;
    }

    // Takes the keys from the next frame on: a key pressed while recording, and held after it, reaches no other view.
    static void ClaimKeys(ImGuiID owner) {
        for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; ++key) {
            if (Recordable(ImGuiKey(key), false))
                ImGui::SetKeyOwner(ImGuiKey(key), owner, ImGuiInputFlags_LockUntilRelease);
        }
    }

    // Both recorders: a single key, or with chords a key with the modifiers held as it goes down.
    static bool Recorder(const char* label, ImGuiKeyChord* key, bool chords, const KeyRecorderOptions& options) {
        const Metrics::TextFieldMetrics& metrics = Metrics::TextField();
        const Metrics::FormMetrics& form = Metrics::Form();
        const ImVec2 accessory(options.width, metrics.height);
        const std::optional<ImRect> placed = PlaceLabeledControl({.label = Interaction::VisibleLabel(label), .accessorySize = accessory, .trailingInset = form.valueTrailing, .accessoryTop = (form.rowHeight - accessory.y) * 0.5f}, metrics.labelSpacing);
        if (!placed)
            return false;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const Font font = Font::Style(TextStyle::Body);
        const bool enabled = Environment().enabled;
        const ImRect field = FieldInFrame(*placed);
        const CornerRadii radii(Px(metrics.searchRadius));
        {
            const Interaction::DisabledFade fade;
            Bezel::Field(draw, field, radii);
        }

        const ImGuiID id = ImGui::GetID(label);
        RecorderState& state = State::Get<RecorderState>(id);
        ImGui::PushID(label);
        // The clear button stands at the end of the field while a key is set; the rest of the field starts recording. As
        // in KeyboardShortcuts, a click more than 3 pt outside the field or Escape stops it, Delete clears the key and
        // goes on recording, and a key pressed is taken and ends it.
        const bool clearable = *key != ImGuiKey_None;
        const ImRect clear = FieldClearButtonFrame(field);
        bool changed = false;
        if (clearable && FieldClearButton(ImGui::GetID("clear"), clear, colors.tertiaryLabel)) {
            *key = ImGuiKey_None;
            changed = true;
        }
        const ImRect area(field.Min, ImVec2(clearable ? clear.Min.x : field.Max.x, field.Max.y));
        const Interaction::Response response = Interaction::Button(ImGui::GetID("field"), area, ImGuiButtonFlags_PressedOnClick);
        ImGui::PopID();
        ImRect reach = field;
        reach.Expand(Px(metrics.recorderClickMargin));
        if (response.pressed)
            state.recording = true;
        else if (state.recording && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !reach.Contains(ImGui::GetIO().MousePos))
            state.recording = false;
        else if (state.recording) {
            const ImGuiKey pressed = PressedKey(id, chords);
            if (pressed == ImGuiKey_Backspace || pressed == ImGuiKey_Delete) {
                changed |= *key != ImGuiKey_None;
                *key = ImGuiKey_None;
            } else if (pressed != ImGuiKey_None) {
                if (pressed != ImGuiKey_Escape) {
                    const ImGuiKeyChord taken = chords ? ImGuiKeyChord(pressed) | (ImGui::GetIO().KeyMods & ImGuiMod_Mask_) : ImGuiKeyChord(pressed);
                    changed |= *key != taken;
                    *key = taken;
                }
                state.recording = false;
            }
        }

        // The key, or the placeholder, which reads "Press Shortcut" while recording, or the modifiers held so far: centered
        // between the clear buttons' places.
        const ImGuiKeyChord held = chords && state.recording ? ImGuiKeyChord(ImGui::GetIO().KeyMods & ImGuiMod_Mask_) : 0;
        const std::string name = held ? ShortcutName(held) : chords ? ShortcutName(*key) : KeyName(ImGuiKey(*key));
        const std::string_view text = held || *key != ImGuiKey_None ? std::string_view(name) : state.recording ? std::string_view("Press Shortcut") : std::string_view(options.placeholder);
        const Rgba color = colors.LabelColor(*key != ImGuiKey_None && enabled);
        const float top = field.GetCenter().y - Px(font.lineHeight) * 0.5f;
        const float button = field.Max.x - clear.Min.x;
        Typography::Draw(draw, font, ImRect(field.Min.x + button, top, field.Max.x - button, top + Px(font.lineHeight)), color, text, TextAlignment::Center);
        if (state.recording) {
            Draw::FocusRing(draw, field, radii, colors.accent);
            ClaimKeys(id);
        }
        return changed;
    }

    bool KeyRecorder(const char* label, ImGuiKey* key, const KeyRecorderOptions& options) {
        ImGuiKeyChord chord = *key;
        const bool changed = Recorder(label, &chord, false, options);
        *key = ImGuiKey(chord);
        return changed;
    }

    bool KeyRecorder(const char* label, ImGuiKeyChord* shortcut, const KeyRecorderOptions& options) {
        return Recorder(label, shortcut, true, options);
    }
} // namespace Cupertino
