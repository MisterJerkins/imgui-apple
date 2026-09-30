#pragma once

#include "Color.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <functional>

namespace Cupertino {
    enum class Platform {
        MacOS,
        IOS,
    };

    enum class Appearance {
        Light,
        Dark,
    };

    enum class ControlSize {
        Mini,
        Small,
        Regular,
        Large,
        // macOS has no extra large controls: SwiftUI draws them large.
        ExtraLarge,
    };

    // The window state controls draw for, as SwiftUI's controlActiveState: the key window; a main window that is not
    // key, like one behind its sheet (gray controls and sidebar, active chrome); or an inactive window.
    enum class ControlActiveState {
        Key,
        Active,
        Inactive,
    };

    // Draws controls as if the pointer were over them, pressing them or they had keyboard focus, as Xcode's previews do;
    // galleries show every state side by side this way. None follows the real input.
    enum class InteractionPreview {
        None,
        Hovered,
        Pressed,
        // A text field with the insertion point after its text.
        Focused,
        // A text field with all its text selected.
        TextSelected,
    };

    // SwiftUI's backgroundProminence: increased on a selected row in a focused list, where primary content turns white.
    enum class BackgroundProminence {
        Standard,
        Increased,
    };

    // SwiftUI's windowToolbarStyle: the unified toolbar 52 pt tall, or the compact one of 38 pt (kit's Bars/Toolbar Mono);
    // the traffic lights, titles and items stay centered in it.
    enum class WindowToolbarStyle {
        Unified,
        UnifiedCompact,
    };

    // SwiftUI's badgeProminence: a list row's badge as a count in tertiary text, or increased, white in a red disc
    // (System Settings' sidebar).
    enum class BadgeProminence {
        Standard,
        Increased,
    };

    // How a scroll view marks its top edge while content is scrolled under the bar above it.
    enum class ScrollEdgeStyle {
        None,
        // A line, as under the search field of a System Settings sidebar.
        Line,
        // A hairline with a soft shadow, as under a System Settings toolbar.
        Shadow,
    };

    enum class AccentColor {
        Multicolor,
        Blue,
        Purple,
        Pink,
        Red,
        Orange,
        Yellow,
        Green,
        Graphite,
    };

    // The two columns of a Form in the columns style (ColumnsForm): labels end labelWidth points in, right-aligned, and
    // the content starts spacing points after them; the rows' text is textSize on lines lineHeight apart.
    struct ColumnsFormStyle {
        float labelWidth = 0.0f;
        float spacing = 8.0f;
        float textSize = 13.0f;
        float lineHeight = 16.0f;
    };

    // A calendar day; a year of 0 means none.
    struct CalendarDay {
        int year = 0;
        int month = 0;
        int day = 0;
    };

    // Values that flow down the view hierarchy, like SwiftUI's environment.
    struct EnvironmentValues {
        Platform platform = Platform::MacOS;
        Appearance appearance = Appearance::Light;
        AccentColor accent = AccentColor::Multicolor;
        ControlSize controlSize = ControlSize::Regular;
        // Pixels per point of the display (2 on Retina) and the user's interface zoom (0.75 ... 2).
        float displayScale = 1.0f;
        float zoom = 1.0f;
        // Desktop tint that macOS mixes into window materials ("Allow wallpaper tinting in windows"); white is neutral.
        Rgba wallpaperTint = Rgba(1.0f, 1.0f, 1.0f);
        ControlActiveState controlActiveState = ControlActiveState::Key;
        // How far controls have gone from the key look to controlActiveState's, 0 ... 1: a window fades over as its sheet
        // comes in and back as the sheet leaves.
        float controlActiveAmount = 1.0f;
        BackgroundProminence backgroundProminence = BackgroundProminence::Standard;
        BadgeProminence badgeProminence = BadgeProminence::Standard;
        // Set by Window from its options.
        WindowToolbarStyle windowToolbarStyle = WindowToolbarStyle::Unified;
        // Set by a sidebar List: its id, which a click on one of its rows gives the keyboard focus, and whether it has the
        // focus (SwiftUI's isFocused); without it the list's selection turns gray.
        ImGuiID focusScope = 0;
        bool isFocused = true;
        // The day calendars mark as today: the system's while unset (captures pin it).
        CalendarDay today;
        // A row's place in a sidebar outline: its depth under disclosure groups, whether its list gives every row a column
        // for the chevrons (a list with disclosure groups, Photos @2x), and the expansion a group's own row turns.
        int outlineLevel = 0;
        bool outlineColumn = false;
        bool* outlineExpansion = nullptr;
        // Set by a column for the scroll view that fills it: that scroll view reports its offset under this id and marks
        // its top edge in this style while scrolled. Its content goes without, so nested scroll views stay plain.
        ImGuiID scrollEdge = 0;
        ScrollEdgeStyle scrollEdgeStyle = ScrollEdgeStyle::None;
        // Set by Id in the frame its value changes: the views in it are new ones, and scroll views start at the top.
        bool identityChanged = false;
        bool enabled = true;
        InteractionPreview interactionPreview = InteractionPreview::None;
        // Set by Form: controls take their grouped-form look (switch toggles, borderless pop-ups).
        bool insideForm = false;
        // Set by ColumnsForm: a labelWidth over 0 puts labels and content in its two columns.
        ColumnsFormStyle columnsForm;
        // Set by Sheet: a Form keeps margins all around, and grows the sheet instead of scrolling unless the sheet has a
        // fixed height; a NavigationSplitView drops its toolbar.
        bool insideSheet = false;
        bool sheetGrows = false;
        // Set by SheetFooter: push buttons are at least this wide, in points.
        float buttonMinWidth = 0.0f;
        // Set by ListRowInsets: the space over, before and under the content of form rows, in points; negative keeps the
        // form's.
        float rowTopInset = -1.0f;
        float rowLeadingInset = -1.0f;
        float rowBottomInset = -1.0f;

        float Scale() const {
            return displayScale * zoom;
        }

        bool IsDark() const {
            return appearance == Appearance::Dark;
        }

        // How key the controls look, 0 ... 1: 1 in the key window, going to 0 as the window fades to controlActiveState.
        float KeyAmount() const {
            return controlActiveState == ControlActiveState::Key ? 1.0f : 1.0f - ImSaturate(controlActiveAmount);
        }
    };

    EnvironmentValues& Environment();

    // Changes environment values for everything drawn inside content, then restores them.
    void WithEnvironment(const std::function<void(EnvironmentValues&)>& modify, const std::function<void()>& content);

    // SwiftUI's disabled(_:): the controls in content take no input and draw dimmed.
    void Disabled(bool disabled, const std::function<void()>& content);

    // SwiftUI's controlSize(_:): the controls in content take the given size.
    void WithControlSize(ControlSize size, const std::function<void()>& content);

    // Draws the controls in content as if hovered, pressed or focused (InteractionPreview), or in a window that is not
    // key, whatever the real input and window.
    void WithInteractionPreview(InteractionPreview preview, const std::function<void()>& content);
    void WithControlActiveState(ControlActiveState state, const std::function<void()>& content);

    // SwiftUI's badgeProminence(_:) for the list rows in content.
    void WithBadgeProminence(BadgeProminence prominence, const std::function<void()>& content);

    // SwiftUI's listRowInsets(_:) for the form rows in content, in points: over and under them (the Region row of
    // Language & Region keeps 13 pt under its pop-up, a nearby device in Bluetooth 8 pt around its picture, @2x), and
    // before them, where the separator under a row starts too (the switches under a module of Control Center, 40 pt).
    void ListRowInsets(float top, float bottom, const std::function<void()>& content);
    void ListRowInsets(float top, float leading, float bottom, const std::function<void()>& content);

    float Px(float points);
    ImVec2 Px(ImVec2 points);
    float Pt(float pixels);
} // namespace Cupertino
