#pragma once

#include "Cupertino.h"

#include <string>

// Live examples built on CupertinoUi: windows of macOS apps as they look on macOS 15 Sequoia. Each takes open to hide
// its window with the close button, and where the window opens.
namespace Examples {
    namespace Settings {
        struct Mac;
    }

    // The window's first position and its size in points; a zero size keeps the example's own.
    struct Placement {
        ImVec2 position = ImVec2(90.0f, 80.0f);
        ImVec2 size = ImVec2(0.0f, 0.0f);

        ImVec2 SizeOr(ImVec2 own) const {
            return size.x > 0.0f ? size : own;
        }
    };

    enum class ThemeChoice {
        Light,
        Dark,
        Auto,
    };

    // What System Settings > Appearance chooses; the host applies it to Cupertino::Environment() every frame, Auto
    // following the host's own appearance.
    struct AppearancePreferences {
        ThemeChoice theme = ThemeChoice::Auto;
        Cupertino::AccentColor accent = Cupertino::AccentColor::Multicolor;
        bool wallpaperTinting = true;
    };

    struct SystemSettingsOptions {
        Placement placement;
        // The Mac whose settings the window shows; null is a MacBook Air on macOS 15.
        const Settings::Mac* mac = nullptr;
        // What the Appearance pane changes; null keeps its choices inside the example.
        AppearancePreferences* appearance = nullptr;
        // A software update is waiting: the sidebar lists it under the account.
        bool updateAvailable = false;
    };

    // System Settings: the sidebar with its search field, the toolbar's history buttons and every pane of macOS 15.
    void SystemSettings(bool* open, const SystemSettingsOptions& options = {});

    struct ActivityMonitorOptions {
        Placement placement;
        // The owner of the processes, in the User column.
        const char* user = "appleseed";
    };

    // Activity Monitor on its Energy tab: the unified toolbar with a segmented control and a search field, the process
    // table with its outline and the history charts under it.
    void ActivityMonitor(bool* open, const ActivityMonitorOptions& options = {});

    // A settings window with its tabs in the toolbar, as the TV app has: a two-column form of pop-ups with descriptions,
    // a list, checkboxes and a footer.
    void TabbedSettings(bool* open, const Placement& placement = {});

    // The pictures macOS draws from artwork (app icons, device photos, wallpapers) come baked into the build
    // (tools/bake.py); a folder with sidebar, pictures, appearance and wallpapers folders in it replaces them while
    // developing. Without either the examples draw symbols and gradients in their place.
    void SetAssetsDirectory(const std::string& directory);
    void UseBakedAssets(bool enabled);
} // namespace Examples
