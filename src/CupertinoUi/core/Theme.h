#pragma once

#include "Color.h"
#include "Environment.h"
#include "Shape.h"

#include <array>
#include <span>

namespace Cupertino {
    // Semantic colors for the current platform, appearance, accent and window state.
    // Vibrant (plus-darker / plus-lighter) tokens are already flattened against the material they sit on.
    struct Palette {
        Rgba label;
        Rgba secondaryLabel;
        Rgba tertiaryLabel;
        Rgba quaternaryLabel;
        Rgba quinaryLabel;

        Rgba fill;
        Rgba secondaryFill;
        Rgba tertiaryFill;
        Rgba quaternaryFill;
        Rgba quinaryFill;
        Rgba separator;

        Rgba accent;
        // The accent as control bezels draw it (default button, checkbox, radio, pop-up plate, slider fill).
        Rgba controlAccent;
        Rgba accentPressed;
        // The track of an "on" switch, a gradient from top to bottom.
        Rgba switchOnTop;
        Rgba switchOnBottom;
        Rgba selection;
        Rgba unfocusedSelection;
        Rgba selectedContent;
        // Secondary text and glyphs on a selected row: the selected content at 70%.
        Rgba selectedSecondaryContent;

        Rgba windowBackground;
        // The light edge inside a window: a faint rim all around (half a point in light, a point in dark) and a brighter
        // highlight along the top (window captures @2x).
        Rgba windowRim;
        Rgba windowHighlight;
        // The crisp dark line just outside every window.
        Rgba windowOutline;
        Rgba sidebarBackground;
        Rgba sidebarDivider;
        // A split view's divider (Xcode @2x: #C0C0C0 on white, black in dark).
        Rgba splitDivider;
        // The overlay scroller's thumb, and its track under the pointer (the thumb's color at 15%).
        Rgba scrollerThumb;
        Rgba scrollerTrack;
        Rgba sidebarLabel;
        Rgba sidebarSecondaryLabel;
        // The vibrant tertiary text over the sidebar: group headers and row counts (Finder, Notes @2x: 160 over 224).
        Rgba sidebarTertiaryLabel;
        Rgba searchField;
        Rgba searchFieldBorder;
        // A unified toolbar over content (Activity Monitor @2x: #FCFCFC over the white table).
        Rgba toolbarBackground;
        // The title bar of a titled window and a settings window's tab bar: the toolbar's color, but its material lets
        // the wallpaper through a quarter more (#F5F5F5@0.80 against the window's @0.84; 512 Pixels @2x: 240 where the
        // window is 236).
        Rgba titleBarBackground;
        Rgba toolbarTitle;
        Rgba toolbarSymbol;
        Rgba toolbarSymbolDisabled;
        Rgba toolbarLine;
        // Content scrolled under a bar (Privacy & Security @2x): the toolbar takes a darker background over a hairline and
        // the scroll edge shadow, and a line shows under the sidebar's search field.
        Rgba scrollEdgeBackground;
        Rgba scrollEdgeLine;
        Rgba sidebarScrollEdge;

        Rgba sectionBackground;
        // The section edge is darker in its outer half point than in its inner one.
        Rgba sectionBorder;
        Rgba sectionInnerBorder;
        Rgba rowSeparator;
        // The line over the + and − bar of a plain list (kit Lists/Small List).
        Rgba listBarLine;

        Rgba controlBackground;
        // A combo box's field: white in light, a faint lift of what is under it in dark (AppKit docs @2x in sRGB: 54 over 43).
        Rgba comboBoxBackground;
        Rgba controlBorder;
        Rgba controlPressed;
        Rgba knob;
        Rgba switchTrack;
        // The recessed track of sliders and progress indicators, measured on System Settings; slider ticks are one
        // shape with the track, only darker.
        Rgba track;
        // The unlit part of a level indicator (HIG art: #E5E5E5 on #F7F7F7, #5B5B5B in dark).
        Rgba levelIndicatorTrack;
        Rgba trackTick;
        Rgba sliderKnob;
        Rgba popUpIndicator;
        Rgba popUpPressed;
        // Bezeled text and search fields: an opaque fill so the hard shade under the bottom edge never shows through.
        Rgba fieldBackground;
        // A token of a token or search field (Console's search tokens @2x: #E9E8EC on the white field, 13 levels over the
        // dark one).
        Rgba tokenBackground;
        Rgba fieldBorder;
        Rgba fieldSymbol;
        // The clear button of a field with text (Finder's search field @2x: black at 55% on the field).
        Rgba fieldClearButton;
        // The image well (kit): the sunken fill, the white ring around it and the faint lines inside and outside.
        Rgba imageWellFill;
        Rgba imageWellRim;
        Rgba imageWellEdge;
        // The bordered color well (kit): the gradient frame from top to bottom and its line.
        Rgba colorWellTop;
        Rgba colorWellBottom;
        Rgba colorWellLine;
        Rgba textSelection;
        // NSColor.linkColor: inline links such as "Learn more…" (Bluetooth @2x).
        Rgba link;
        // Accessory bars (Finder's search scopes @2x): the plate of a scope that is on (#D9D9D9 on white) and its title
        // (#414141), and the outline of an action.
        Rgba accessoryBarPlate;
        Rgba accessoryBarSelectedText;
        Rgba accessoryBarOutline;
        // Tables (Activity Monitor @2x): a white ground, #F4F5F5 stripes, dividers, the selection (from the accent,
        // #0064E1 for blue) with light lines between selected neighbours, and the sort chevron.
        Rgba tableBackground;
        Rgba tableStripe;
        Rgba tableDivider;
        Rgba tableSelection;
        Rgba tableSelectionSplit;
        Rgba tableSortIndicator;
        // An AppKit box's border and the lines inside it (Activity Monitor @2x: #C6C6C6).
        Rgba boxBorder;
        // Chart gridlines and verticals (Battery @2x: 211 on a 242 section).
        Rgba chartGrid;

        // Sheets: the panel, and the veil over the window behind (Battery Options @2x, Wi-Fi sheet @2x).
        Rgba sheetBackground;
        Rgba sheetDim;
        // A sidebar inside a sheet (Wi-Fi details @2x): lighter in dark than the window's, a black divider, and a lighter
        // selection than the window sidebar's.
        Rgba sheetSidebarBackground;
        Rgba sheetSidebarDivider;
        Rgba sheetSelection;

        // Alerts: the panel and the flat gray of buttons that are not the default one (macOS 15 alerts @2x). An alert on
        // its own over the desktop takes the wallpaper tint (Empty Trash @2x: 216 = 224 x 0.964), one attached to its
        // window does not (Mail @2x: 224).
        Rgba alertBackground;
        Rgba attachedAlertBackground;
        // Alert title and message: the label in light, white in dark (dark alerts @2x).
        Rgba alertText;
        // A destructive default button: system red in light, #E63C41 under the bezel's light in dark (VS Code alert @2x).
        Rgba destructiveFill;
        Rgba alertButton;
        Rgba alertButtonPressed;
        Rgba alertButtonText;
        // A text field in an alert: white with a line inside its edge in light; in dark a veil of white over the panel,
        // a brighter line inside the edge and a light line under it (authorization dialog @2x).
        Rgba alertField;
        Rgba alertFieldRim;
        Rgba alertFieldHighlight;

        // Popover material over light content (Safari page menu @2.3x), darker than a menu.
        Rgba popoverBackground;
        Rgba popoverBorder;

        Rgba tooltipBackground;
        Rgba tooltipText;
        Rgba tooltipBorder;

        // Notification banners: the material flattened over the desktop, a light rim inside its edge, the text, the
        // timestamp in the material's vibrant secondary gray and action buttons in its vibrant quinary fill (macOS 15 @2x).
        Rgba notificationBackground;
        Rgba notificationRim;
        Rgba notificationText;
        Rgba notificationTimestamp;
        Rgba notificationButton;
        Rgba notificationButtonPressed;
        // The stacked cards under a banner, the nearer and the farther: the material darkened, much more so in dark
        // (Notification Center @2x: 41, 28 and 19 over a dark wallpaper; Big Sur's light stack about 4 and 7% darker).
        Rgba notificationStackNear;
        Rgba notificationStackFar;

        Rgba menuBackground;
        Rgba menuBorder;
        Rgba menuRim;
        Rgba menuSeparator;
        // The capsule of a menu item's badge (NSMenuItemBadge @2x: 201 over 223, 103 over 85).
        Rgba menuBadge;
        // The highlighted item: the sidebar's selection in light, lighter than the window sidebar's in dark (#418CE6,
        // Wi-Fi details @2x).
        Rgba menuSelection;
        Rgba menuHeader;
        Rgba menuShortcut;
        // The menu bar: a translucent band over the desktop, its titles, and the plate under the title of the open menu
        // (White 0.2 on a dark bar, 512 Pixels).
        Rgba menuBarBackground;
        Rgba menuBarTitle;
        Rgba menuBarSelection;

        Rgba trafficClose;
        Rgba trafficMinimize;
        Rgba trafficZoom;
        Rgba trafficInactive;
        // A disabled button of a main window, like close behind a sheet: an opaque disc over the dim (@2x sheets).
        Rgba trafficDisabled;
        Rgba trafficBorder;

        // A control's title, value or glyph: the label color, tertiary while disabled.
        Rgba LabelColor(bool enabled) const {
            return enabled ? label : tertiaryLabel;
        }
    };

    namespace Theme {
        // Colors for the current environment; recomputed only when the environment changes.
        const Palette& Colors();

        // controlAccentColor of a System Settings accent preset (Multicolor draws system controls in blue).
        Rgba Accent(AccentColor accent, Appearance appearance);

        // The text highlight color that goes with an accent (Appearance's Highlight color); Multicolor highlights in blue.
        Rgba Highlight(AccentColor accent, Appearance appearance);

        // Sidebar and menu selection platter for an accent: the Selection Focused material flattened to a solid color.
        Rgba Selection(Rgba accent, Appearance appearance);
        // A table's or list's selected rows as focused as focus says (Interaction::ListFocus): gray at 0, accented at 1.
        Rgba ListSelection(float focus);
        // An accented or destructive button's fill while it is pressed.
        Rgba Pressed(Rgba fill);

        // The macOS 15 system colors (Color.red, .orange ...), opaque variants.
        Rgba SystemRed();
        Rgba SystemOrange();
        Rgba SystemYellow();
        Rgba SystemGreen();
        Rgba SystemCyan();
        Rgba SystemBlue();
        Rgba SystemPurple();
        Rgba SystemPink();
        Rgba SystemGray();

        // Shadow stacks from the kit's layer styles, for the current appearance.
        std::span<const Shadow> WindowShadows();
        std::span<const Shadow> SheetShadows();
        std::span<const Shadow> MenuShadows();
        std::span<const Shadow> BezelShadows();
        // The lighter top edge inside a dark control's bezel, over the accent or over gray; none in light.
        std::span<const Shadow> BezelHighlights(bool accent);
        std::span<const Shadow> TooltipShadows();
        std::span<const Shadow> NotificationShadows();
        std::span<const Shadow> KnobShadows(bool on);
        std::span<const Shadow> SwitchTrackInnerShadows(bool on);
        std::span<const Shadow> WellInnerShadows();
        std::span<const Shadow> SegmentedWellInnerShadows();
        std::span<const Shadow> TrackInnerShadows();
        std::span<const Shadow> ImageWellInnerShadows();
        std::span<const Shadow> FieldShadows();
        // Under a plain or inset list (kit Lists/Small List): a hard line a point below its bottom edge.
        std::span<const Shadow> ListShadows();
        // Under an icon plate (System Settings sidebar @2x): a soft shade a pixel wide around it, heavier below.
        std::span<const Shadow> IconPlateShadows();
        // Cast by a bar over content scrolled under it (Privacy & Security @2x).
        std::span<const Shadow> ScrollEdgeShadows();

        // Under checkboxes, radios and accent-filled controls: in light a soft glow in the accent color (AirPods @2x), in
        // dark a rim darker below (Spotlight and the Wi-Fi details sheet @2x).
        std::array<Shadow, 2> ControlShadows(Rgba accent);
    } // namespace Theme
} // namespace Cupertino
