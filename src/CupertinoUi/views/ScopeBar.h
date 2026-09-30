#pragma once

#include <functional>
#include <initializer_list>
#include <span>

namespace Cupertino {
    // The scope bar of a search, as macOS draws SwiftUI's searchScopes (Finder's search @2x): a 30 pt bar across its
    // container with the title in Bold 12 secondary text, the scopes as accessory bar toggles of which one is on, the
    // actions (accessory bar action buttons) at the far end, and a separator under it. Returns true when the scope changes.
    bool ScopeBar(const char* title, int* selection, std::span<const char* const> scopes, const std::function<void()>& actions = {});
    bool ScopeBar(const char* title, int* selection, std::initializer_list<const char*> scopes, const std::function<void()>& actions = {});
} // namespace Cupertino
