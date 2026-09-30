#include "State.h"

#include <unordered_map>

namespace Cupertino::State {
    struct Key {
        ImGuiID id;
        const void* type;

        bool operator==(const Key&) const = default;
    };

    struct KeyHash {
        size_t operator()(const Key& key) const {
            return size_t(key.id) ^ (reinterpret_cast<size_t>(key.type) * 0x9E3779B97F4A7C15ull);
        }
    };

    static std::unordered_map<Key, Slot, KeyHash>& Slots() {
        static std::unordered_map<Key, Slot, KeyHash> slots;
        return slots;
    }

    Slot& Find(ImGuiID id, const void* type, void* (*create)(), void (*destroy)(void*), bool* created) {
        auto [it, inserted] = Slots().try_emplace(Key{id, type});
        Slot& slot = it->second;
        if (inserted) {
            slot.data = create();
            slot.destroy = destroy;
        }
        slot.lastUsedFrame = ImGui::GetFrameCount();
        if (created)
            *created = inserted;
        return slot;
    }

    void Collect(int idle_frames) {
        const int frame = ImGui::GetFrameCount();
        auto& slots = Slots();
        for (auto it = slots.begin(); it != slots.end();) {
            if (frame - it->second.lastUsedFrame > idle_frames) {
                it->second.destroy(it->second.data);
                it = slots.erase(it);
            } else {
                ++it;
            }
        }
    }

    void Clear() {
        for (auto& [key, slot] : Slots())
            slot.destroy(slot.data);
        Slots().clear();
    }
} // namespace Cupertino::State
