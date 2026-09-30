#pragma once

#include "imgui.h"
#include "imgui_internal.h"

namespace Cupertino::State {
    struct Slot {
        void* data = nullptr;
        void (*destroy)(void*) = nullptr;
        int lastUsedFrame = 0;
    };

    template <typename T>
    inline const char TypeTag = 0;

    // Finds or creates the slot of one state type for a widget; created reports a fresh slot.
    Slot& Find(ImGuiID id, const void* type, void* (*create)(), void (*destroy)(void*), bool* created);

    // Per-widget state of any default-constructible type, shared by all modules instead of per-widget maps.
    template <typename T>
    T& Get(ImGuiID id, bool* created = nullptr) {
        Slot& slot = Find(id, &TypeTag<T>, []() -> void* { return new T(); }, [](void* data) { delete static_cast<T*>(data); }, created);
        return *static_cast<T*>(slot.data);
    }

    // Frees state that no widget asked for during the last idle_frames frames.
    void Collect(int idle_frames = 180);
    void Clear();
} // namespace Cupertino::State
