#pragma once

#include "imgui.h"

#include <string>

namespace Cupertino {
    struct KeyRecorderOptions {
        float width = 130.0f;
        // Shown while no key is set.
        const char* placeholder = "Record Shortcut";
    };

    // A shortcut recorder as KeyboardShortcuts.Recorder draws one: the key in a rounded field with a clear button. A click
    // records the next key or mouse button (not the left one), owning the keys meanwhile; Escape or a click outside stops,
    // Delete clears. Returns true when the key changes.
    bool KeyRecorder(const char* label, ImGuiKey* key, const KeyRecorderOptions& options = {});

    // The same recorder for a shortcut with modifiers (KeyboardShortcuts.Recorder): a key goes in with the modifiers held
    // as it goes down, which show in the field while they are held alone; a modifier by itself is not taken.
    bool KeyRecorder(const char* label, ImGuiKeyChord* shortcut, const KeyRecorderOptions& options = {});
} // namespace Cupertino
