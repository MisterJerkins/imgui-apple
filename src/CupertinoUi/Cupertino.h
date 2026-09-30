#pragma once

// CupertinoUi: macOS 15 Sequoia and iOS 18 interface for Dear ImGui with SwiftUI-style layout.
#include "controls/Button.h"
#include "controls/ColorPicker.h"
#include "controls/ComboBox.h"
#include "controls/DatePicker.h"
#include "controls/Gauge.h"
#include "controls/IconPlate.h"
#include "controls/ImageWell.h"
#include "controls/KeyRecorder.h"
#include "controls/LabeledContent.h"
#include "controls/LevelIndicator.h"
#include "controls/PathControl.h"
#include "controls/Menu.h"
#include "controls/Picker.h"
#include "controls/Progress.h"
#include "controls/Slider.h"
#include "controls/Stepper.h"
#include "controls/Text.h"
#include "controls/TextField.h"
#include "controls/Toggle.h"
#include "controls/TokenField.h"
#include "core/BakedFiles.h"
#include "core/Bitmap.h"
#include "core/Color.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/KeyboardShortcut.h"
#include "core/Symbols.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Grid.h"
#include "layout/Layout.h"
#include "layout/ScrollView.h"
#include "layout/SplitView.h"
#include "layout/Stacks.h"
#include "overlays/Alert.h"
#include "overlays/Notification.h"
#include "overlays/Popover.h"
#include "overlays/Sheet.h"
#include "overlays/Tooltip.h"
#include "views/BorderedList.h"
#include "views/ContentUnavailable.h"
#include "views/Chart.h"
#include "views/Disclosure.h"
#include "views/Form.h"
#include "views/GroupBox.h"
#include "views/ScopeBar.h"
#include "views/Sidebar.h"
#include "views/Table.h"
#include "window/ColorPanel.h"
#include "window/MenuBar.h"
#include "window/Window.h"

#include <string>

namespace Cupertino {
    struct Configuration {
        // Folder with the SF Pro Text and Display and SF Mono OTF files and their derived symbol fonts; empty loads the
        // fonts baked into the library (tools/bake.py).
        std::string fontsDirectory;
        // Keyboard navigation (System Settings > Keyboard): Tab moves the focus through every control, which shows its
        // focus ring, and Space presses it. Off, as macOS ships, only fields and lists take the focus.
        bool fullKeyboardAccess = false;
    };

    // Loads the fonts and prepares ImGui; call once after ImGui::CreateContext().
    bool Initialize(const Configuration& configuration);

    // Call before ImGui::Render(): draws the tooltip and frees the state of views that disappeared.
    void EndFrame();

    struct ShowcaseOptions {
        // The pane to open first, by its slug ("buttons", "choices", "values", "text", "form", "lists", "boxes",
        // "presentations", "layout", "charts").
        const char* initialPane = nullptr;
        // The window's size and first position in points.
        ImVec2 size = ImVec2(760.0f, 540.0f);
        ImVec2 position = ImVec2(40.0f, 40.0f);
    };

    // Every view of the library in one window, a pane per kind in each of its styles and states, as ShowDemoWindow() is
    // for Dear ImGui; the close button sets open to false when it is given.
    void ShowShowcase(bool* open = nullptr, const ShowcaseOptions& options = {});
} // namespace Cupertino
