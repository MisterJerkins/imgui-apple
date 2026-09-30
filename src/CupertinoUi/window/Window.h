#pragma once

#include "core/Environment.h"
#include "core/Shape.h"

#include <functional>
#include <initializer_list>
#include <optional>
#include <span>
#include <vector>

namespace Cupertino {
    // SwiftUI's windowStyle.
    enum class WindowStyle {
        // The content fills the window under the traffic lights; its toolbar or TitleBar gives the title
        // (.hiddenTitleBar, or a window with a toolbar).
        Automatic,
        // .titleBar: a 28 pt title bar with the window's title over the content, the traffic lights in it.
        TitleBar,
        // .plain: no chrome, neither title bar nor traffic lights nor the frame's rim; the window moves by any empty spot.
        Plain,
    };

    struct WindowOptions {
        // Size and initial position in points.
        ImVec2 size = ImVec2(715.0f, 625.0f);
        ImVec2 position = ImVec2(40.0f, 40.0f);
        // The traffic lights that work; the others stand gray (Finder's copy dialog @2x: only minimize).
        bool closable = true;
        bool minimizable = true;
        bool zoomable = true;
        // The traffic lights stand in a 28 pt title bar, 8 pt from the edges, as over a settings window's tab bar,
        // instead of in a 52 pt toolbar.
        bool compactTitleBar = false;
        // Stays in front of the other windows, as a utility panel (the Colors window).
        bool floating = false;
        // The toolbar's style, as SwiftUI's windowToolbarStyle: its height, where the traffic lights and the toolbar's
        // titles and items center.
        WindowToolbarStyle toolbarStyle = WindowToolbarStyle::Unified;
        // Limits in points for resizing by the bottom and right edges: the window resizes along an axis where maxSize is
        // larger than minSize (System Settings only in height), and keeps its size along the others.
        ImVec2 minSize = ImVec2(0.0f, 0.0f);
        ImVec2 maxSize = ImVec2(0.0f, 0.0f);
        WindowStyle style = WindowStyle::Automatic;
    };

    // The height in points of the unified toolbar of the window being drawn: 52, or 38 in the compact style.
    float ToolbarHeight();

    // A macOS window: rounded corners, window shadow, edge highlight and traffic lights. The content fills the
    // window; it is dragged by its chrome. When open is given, the close button sets it to false.
    void Window(const char* title, bool* open, const WindowOptions& options, const std::function<void()>& content);

    // The light edge inside a window's frame, drawn over its content: the rim all around and the highlight along the
    // top, its first half point full and the next at 40% (512 Pixels window captures @2x, light 0.68/0.26, dark
    // 0.36/0.28 over the rim's 0.2). App-modal alerts are windows and draw it too.
    void DrawWindowEdge(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, CornerStyle style = CornerStyle::Continuous);

    // The window a sheet is attached to, as the sheet sees it: its frame, and the environment and opacity its content
    // has when no sheet blocks it. Outside a window nothing is attached.
    struct SheetHost {
        ImRect frame;
        EnvironmentValues environment;
        float alpha = 1.0f;
        bool attached = false;
    };

    // Attaches a sheet presented to the given degree (0 ... 1) to the window being drawn, for this frame: the window
    // dims under the sheet and disables its close button, and from the next frame on its content stays main but not
    // key, fades and takes no input.
    SheetHost AttachSheet(float presentation);
    // Draws content as the host's sheet: in the host's environment (modify changes it further) at opacity, taking input
    // while the host takes none.
    void InSheet(const SheetHost& host, float opacity, const std::function<void(EnvironmentValues&)>& modify, const std::function<void()>& content);

    // The title bar of a titled window without a toolbar, as its content's first view (Force Quit @2x): 28 pt of the
    // title bar's material with the title in 13 pt Bold across the middle and a hairline under it.
    void TitleBar(const char* title);

    // The same bar drawn into bar (pixels): the title across the middle of its top 28 pt and the hairline at its bottom,
    // for bars that hold more under the title (a settings window's tabs, the Colors window's modes).
    void DrawTitleBar(ImDrawList* draw, const ImRect& bar, const char* title);

    struct TabBarItem {
        const char* title = "";
        unsigned symbol = 0;
    };

    // The toolbar of a settings window, as SwiftUI draws a TabView in a Settings scene (Bars/Tab Bar): the selected tab's
    // title in bold over the tabs, each a symbol over its label in cells as wide as the widest label, the selected one on
    // a gray backing in the accent color. Returns true when the selection changes.
    bool TabBar(int* selection, std::span<const TabBarItem> items);
    bool TabBar(int* selection, std::initializer_list<TabBarItem> items);

    struct ToolbarOptions {
        const char* title = nullptr;
        const char* subtitle = nullptr;
        // Filled behind the toolbar; the palette's toolbar background unless set.
        std::optional<Rgba> background;
    };

    // The unified toolbar of a window without a sidebar: 52 pt with the title (and subtitle) after the traffic lights,
    // the items after the title, and a hairline under it.
    void Toolbar(const ToolbarOptions& options, const std::function<void()>& items);

    enum class NavigationStep {
        None,
        Back,
        Forward,
    };

    // A part of a toolbar subtitle: its text with an optional symbol before it.
    struct ToolbarSubtitle {
        unsigned symbol = 0;
        const char* text = nullptr;
    };

    struct NavigationSplitViewOptions {
        // The divider is the first point column of the detail side, so the detail content starts at width + 1.
        float sidebarWidth = 215.0f;
        const char* title = nullptr;
        // A second line under a smaller title, in parts (Battery: "Battery Level: 100%"; AirPods: the charge of the buds
        // and of the case).
        std::vector<ToolbarSubtitle> subtitle;
        bool showsHistoryButtons = true;
        bool canGoBack = false;
        bool canGoForward = false;
        // Toolbar items after the title, laid out from the bar's trailing end (Finder's view, group, share, tag, action
        // and search @2x).
        std::function<void()> toolbarItems;
        // The toolbar's material under the bar at all times, as Finder's; System Settings' shows it only while content
        // scrolls under.
        bool toolbarBackground = false;
        // A sidebar the pointer resizes by its divider between these widths, as navigationSplitViewColumnWidth(min:ideal:
        // max:); without them it keeps sidebarWidth, as System Settings' does.
        float minSidebarWidth = 0.0f;
        float maxSidebarWidth = 0.0f;
        // NavigationSplitView(columnVisibility:): the sidebar shows while *sidebarVisible, and the sidebar.left toolbar
        // button hides and shows it, the sidebar sliding out and in; the button stands at the sidebar's trailing end and,
        // with the sidebar hidden, after the traffic lights. Without it the sidebar always shows, as in System Settings.
        bool* sidebarVisible = nullptr;
        // .inspector(isPresented:): a trailing column under the toolbar with this content, split from the detail by a
        // divider the pointer drags between the limits.
        std::function<void()> inspector;
        float minInspectorWidth = 200.0f;
        float maxInspectorWidth = 400.0f;
    };

    // The System Settings layout: a full-height sidebar under the traffic lights, and a detail column with a
    // 52 pt toolbar (back and forward buttons, title). Returns the history button pressed this frame. In a sheet both
    // columns run from the top without a toolbar (Wi-Fi details).
    NavigationStep NavigationSplitView(const NavigationSplitViewOptions& options, const std::function<void()>& sidebar, const std::function<void()>& detail);
} // namespace Cupertino
