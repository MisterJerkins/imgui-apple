#pragma once

#include <functional>

namespace Cupertino {
    struct MenuBarOptions {
        // Text at the trailing end, such as the date and time.
        const char* status = nullptr;
    };

    // The menu bar across the top of the screen. The Menu calls inside content become its titles in order: a menu with
    // a symbol shows the symbol (the Apple menu), the first titled menu is the application's, in bold. The keyboard
    // shortcuts of all their items work while the bar is shown.
    void MenuBar(const MenuBarOptions& options, const std::function<void()>& content);
} // namespace Cupertino
