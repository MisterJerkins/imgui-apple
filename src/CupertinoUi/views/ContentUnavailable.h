#pragma once

#include <functional>
#include <string_view>

namespace Cupertino {
    struct ContentUnavailableOptions {
        // An SF Symbol over the title.
        unsigned symbol = 0;
        const char* description = nullptr;
    };

    // ContentUnavailableView (Passwords' "No Item Selected", macOS 15 @2x): in the middle of the offered space, the symbol
    // large in the tertiary label color, the title in bold and the description under it, both secondary and centered,
    // and the actions under them.
    void ContentUnavailableView(const char* title, const ContentUnavailableOptions& options = {}, const std::function<void()>& actions = nullptr);

    // ContentUnavailableView.search(text:): "No Results for “text”" under a magnifying glass.
    void ContentUnavailableSearch(std::string_view text);
} // namespace Cupertino
