#include "Theme.h"

namespace Cupertino::Theme {
    static Rgba Tinted(Rgba color, Rgba tint) {
        return Rgba(color.r * tint.r, color.g * tint.g, color.b * tint.b, color.a);
    }

    // A tint strength times as strong, for a material the wallpaper shows through more.
    static Rgba Tinted(Rgba color, Rgba tint, float strength) {
        return Tinted(color, Rgba(1.0f - strength * (1.0f - tint.r), 1.0f - strength * (1.0f - tint.g), 1.0f - strength * (1.0f - tint.b)));
    }

    Rgba Accent(AccentColor accent, Appearance appearance) {
        const bool dark = appearance == Appearance::Dark;
        switch (accent) {
            case AccentColor::Purple:
                return Rgba::Hex(dark ? 0xA550A7 : 0x953D96);
            case AccentColor::Pink:
                return Rgba::Hex(0xF74F9E);
            case AccentColor::Red:
                return Rgba::Hex(dark ? 0xFF5257 : 0xE0383E);
            case AccentColor::Orange:
                return Rgba::Hex(0xF7821B);
            case AccentColor::Yellow:
                return Rgba::Hex(dark ? 0xFFC600 : 0xFFC726);
            case AccentColor::Green:
                return Rgba::Hex(0x62BA46);
            case AccentColor::Graphite:
                return Rgba::Hex(dark ? 0x8C8C8C : 0x989898);
            case AccentColor::Multicolor:
            case AccentColor::Blue:
                break;
        }
        // Blue is the one accent the same in both appearances (the swatches and selection rings of Appearance @2x).
        return Rgba::Hex(0x007AFF);
    }

    // selectedTextBackgroundColor: the accent at 30% over white, in dark over #5A5A5A (Appearance's highlight swatches
    // @2x, #B4D7FF and #3F638B for blue).
    static Rgba HighlightOf(Rgba accent, Appearance appearance) {
        return Blend::Mix(appearance == Appearance::Dark ? Rgba::Hex(0x5A5A5A) : Rgba::White(1.0f), accent, 0.3f);
    }

    Rgba Highlight(AccentColor accent, Appearance appearance) {
        return HighlightOf(Accent(accent, appearance), appearance);
    }

    // The accent with its saturation and brightness scaled and its hue turned (degrees), the way macOS derives control
    // colors from it.
    static Rgba ScaledAccent(Rgba accent, float saturation, float brightness, float hue_shift = 0.0f) {
        const Hsb hsb = ToHsb(accent);
        return FromHsb({ImFmod(hsb.hue + hue_shift / 360.0f + 1.0f, 1.0f), hsb.saturation * saturation, hsb.brightness * brightness});
    }

    // Measured on System Settings: blue #007AFF becomes #3E92FC in light (guide art and 512 Pixels @2x) and #3A72C9 in the
    // dark window sidebar (Software Update @2x). Saturation and brightness alone leave green too high: the hue also turns
    // toward blue.
    Rgba Selection(Rgba accent, Appearance appearance) {
        return appearance == Appearance::Dark ? ScaledAccent(accent, 0.711f, 0.79f, 5.23f) : ScaledAccent(accent, 0.754f, 0.988f, 2.1f);
    }

    // selectedContentBackgroundColor, the selection of a focused table or list, follows the accent too: blue #007AFF
    // becomes #0064E1 in light and #0058D0 in dark (Activity Monitor @2x), a little darker and turned toward violet.
    static Rgba ContentSelection(Rgba accent, Appearance appearance) {
        return appearance == Appearance::Dark ? ScaledAccent(accent, 1.0f, 0.816f, 3.3f) : ScaledAccent(accent, 1.0f, 0.882f, 2.0f);
    }

    Rgba Pressed(Rgba fill) {
        return Blend::Over(Rgba::Black(0.15f), fill);
    }

    Rgba ListSelection(float focus) {
        const Palette& colors = Colors();
        return Blend::Crossfade(colors.unfocusedSelection, colors.tableSelection, focus);
    }

    // The selection of a sheet's sidebar and, in dark, of menus (Wi-Fi details @2x): #418CE6 in dark, about #568FF4 in a
    // light sheet.
    static Rgba FocusedSelection(Rgba accent, Appearance appearance) {
        return appearance == Appearance::Dark ? ScaledAccent(accent, 0.717f, 0.902f, 1.33f) : ScaledAccent(accent, 0.65f, 0.96f, 7.0f);
    }

    Rgba SystemRed() {
        return Rgba::Hex(Environment().IsDark() ? 0xFF453A : 0xFF3B30);
    }

    Rgba SystemOrange() {
        return Rgba::Hex(Environment().IsDark() ? 0xFF9F0A : 0xFF9500);
    }

    Rgba SystemYellow() {
        return Rgba::Hex(Environment().IsDark() ? 0xFFD60A : 0xFFCC00);
    }

    Rgba SystemGreen() {
        if (Environment().platform == Platform::IOS)
            return Rgba::Hex(Environment().IsDark() ? 0x30D158 : 0x34C759);
        return Rgba::Hex(Environment().IsDark() ? 0x32D74B : 0x28CD41);
    }

    Rgba SystemCyan() {
        return Rgba::Hex(Environment().IsDark() ? 0x5AC8F5 : 0x55BEF0);
    }

    Rgba SystemBlue() {
        return Rgba::Hex(Environment().IsDark() ? 0x0A84FF : 0x007AFF);
    }

    Rgba SystemPurple() {
        return Rgba::Hex(Environment().IsDark() ? 0xBF5AF2 : 0xAF52DE);
    }

    Rgba SystemPink() {
        return Rgba::Hex(Environment().IsDark() ? 0xFF375F : 0xFF2D55);
    }

    Rgba SystemGray() {
        return Rgba::Hex(Environment().IsDark() ? 0x98989D : 0x8E8E93);
    }

    // Light values are calibrated on Apple's System Settings screenshots with wallpaper tinting off.
    static Palette MacLight(Rgba accent, ControlActiveState state, Rgba tint) {
        Palette palette = {
            .label = Rgba::Black(0.85f),
            .secondaryLabel = Rgba::Black(0.5f),
            .tertiaryLabel = Rgba::Black(0.25f),
            .quaternaryLabel = Rgba::Black(0.1f),
            .quinaryLabel = Rgba::Black(0.05f),
            .fill = Rgba::Black(0.1f),
            .secondaryFill = Rgba::Black(0.08f),
            .tertiaryFill = Rgba::Black(0.05f),
            .quaternaryFill = Rgba::Black(0.03f),
            .quinaryFill = Rgba::Black(0.02f),
            .separator = Rgba::Black(0.1f),
            .selectedContent = Rgba::White(1.0f),
            .selectedSecondaryContent = Rgba::White(0.7f),
            .windowBackground = Tinted(Rgba::Hex(0xF6F6F6), tint),
            .windowRim = Rgba::White(0.1f),
            .windowHighlight = Rgba::White(0.64f),
            .windowOutline = Rgba::Black(0.13f),
            .sidebarBackground = Tinted(Rgba::Hex(0xE9E9E9), tint),
            .sidebarDivider = Rgba::Black(0.1f),
            .splitDivider = Rgba::Black(0.25f),
            .scrollerThumb = Rgba::Black(0.4f),
            .scrollerTrack = Rgba::Black(0.06f),
            .sidebarLabel = Rgba::Black(0.7f),
            .searchField = Rgba::Black(0.056f),
            .searchFieldBorder = Rgba::Black(0.1f),
            .toolbarBackground = Tinted(Rgba::Hex(0xFCFCFC), tint),
            .titleBarBackground = Tinted(Rgba::Hex(0xFCFCFC), tint, 1.25f),
            .toolbarTitle = Rgba::Black(0.7f),
            .toolbarSymbol = Rgba::Black(0.55f),
            .toolbarSymbolDisabled = Rgba::Black(0.25f),
            .toolbarLine = Rgba::Black(0.15f),
            .scrollEdgeBackground = Tinted(Rgba::Hex(0xF1F1F1), tint),
            .scrollEdgeLine = Rgba::Black(0.024f),
            .sidebarScrollEdge = Rgba::Black(0.11f),
            .sectionBackground = Rgba::Black(0.017f),
            .sectionBorder = Rgba::Black(0.055f),
            .sectionInnerBorder = Rgba::Black(0.02f),
            .rowSeparator = Rgba::Black(0.045f),
            .listBarLine = Rgba::Black(0.28f),
            .controlBackground = Rgba::White(1.0f),
            .comboBoxBackground = Rgba::White(1.0f),
            .controlBorder = Rgba::Black(0.04f),
            .controlPressed = Rgba::Black(0.08f),
            .knob = Rgba::White(1.0f),
            .switchTrack = Rgba::Black(0.1f),
            .track = Rgba::Black(0.05f),
            .levelIndicatorTrack = Rgba::Black(0.075f),
            .trackTick = Rgba::Black(0.15f),
            .sliderKnob = Rgba::White(1.0f),
            .popUpIndicator = Rgba::Black(0.052f),
            .popUpPressed = Rgba::Black(0.04f),
            .fieldBackground = Rgba::White(1.0f),
            .tokenBackground = Rgba::Hex(0xE9E8EC),
            .fieldBorder = Rgba::Black(0.08f),
            .fieldSymbol = Rgba::Black(0.7f),
            .fieldClearButton = Rgba::Black(0.55f),
            .imageWellFill = Rgba::Black(0.04f),
            .imageWellRim = Rgba::White(1.0f),
            .imageWellEdge = Rgba::Black(0.06f),
            .colorWellTop = Rgba::Hex(0xE3E3E3),
            .colorWellBottom = Rgba::Hex(0xF7F7F7),
            .colorWellLine = Rgba::Hex(0xAFAFAF),
            .link = Rgba::Hex(0x0068DA),
            .accessoryBarPlate = Rgba::Black(0.15f),
            .accessoryBarSelectedText = Rgba::Black(0.7f),
            .accessoryBarOutline = Rgba::Black(0.3f),
            .tableBackground = Rgba::White(1.0f),
            .tableStripe = Rgba::Hex(0xF4F5F5),
            .tableDivider = Rgba::Black(0.1f),
            .tableSelectionSplit = Rgba::White(0.16f),
            .tableSortIndicator = Rgba::Black(0.37f),
            .boxBorder = Rgba::Hex(0xC6C6C6),
            .chartGrid = Rgba::Black(0.13f),
            .sheetBackground = Rgba::White(1.0f),
            .sheetDim = Rgba::Black(0.185f),
            .alertBackground = Tinted(Rgba::Hex(0xE0E0E0), tint),
            .attachedAlertBackground = Rgba::Hex(0xE0E0E0),
            .alertText = Rgba::Black(0.85f),
            .destructiveFill = Rgba::Hex(0xFF3B30),
            .alertButton = Rgba::Black(0.116f),
            .alertButtonPressed = Rgba::Black(0.2f),
            .alertButtonText = Rgba::Black(0.75f),
            .alertField = Rgba::White(1.0f),
            .alertFieldRim = Rgba::Black(0.1f),
            .alertFieldHighlight = Rgba(),
            .popoverBackground = Rgba::Hex(0xDCDCDD),
            .popoverBorder = Rgba::Black(0.2f),
            // Controls/Tooltip: the Thick material #F6F6F6@0.72 as a solid fill, Vibrant Primary text, a hairline.
            .tooltipBackground = Rgba::Hex(0xF6F6F6),
            .tooltipText = Rgba::Hex(0x4C4C4C),
            .tooltipBorder = Rgba::Black(0.12f),
            // The window captures of banners (512 Pixels, Ask Different @2x) hold the material at #D5D5D4 and #626262;
            // darker wallpapers darken it through the tint.
            .notificationBackground = Tinted(Rgba::Hex(0xD5D5D4), tint),
            .notificationRim = Rgba::White(0.1f),
            .notificationText = Rgba::Black(0.85f),
            .menuBackground = Tinted(Rgba::Hex(0xE7E7E7), tint),
            .menuBorder = Rgba::Black(0.22f),
            .menuRim = Rgba::White(0.1f),
            .menuSeparator = Rgba::Black(0.1f),
            .menuBadge = Rgba::Black(0.1f),
            .menuBarBackground = Rgba::White(0.6f),
            .menuBarTitle = Rgba::Black(0.85f),
            .menuBarSelection = Rgba::Black(0.1f),
            .trafficClose = Rgba::Hex(0xFF5F57),
            .trafficMinimize = Rgba::Hex(0xFEBC2E),
            .trafficZoom = Rgba::Hex(0x28C840),
            .trafficInactive = Rgba::Black(0.1f),
            .trafficDisabled = Rgba::Hex(0xCFCFCF),
            .trafficBorder = Rgba::Black(0.2f),
        };
        palette.sidebarSecondaryLabel = Blend::PlusDarker(Rgba::Hex(0x808080), palette.sidebarBackground);
        palette.sidebarTertiaryLabel = Blend::PlusDarker(Rgba::Hex(0xBFBFBF), palette.sidebarBackground);
        palette.menuHeader = Blend::PlusDarker(Rgba::Hex(0xBFBFBF), palette.menuBackground);
        palette.menuShortcut = Blend::PlusDarker(Rgba::Black(0.25f), palette.menuBackground);
        palette.notificationTimestamp = Blend::PlusDarker(Rgba::Hex(0xBFBFBF), palette.notificationBackground);
        palette.notificationButton = Blend::PlusDarker(Rgba::Hex(0xFBFBFB), palette.notificationBackground);
        palette.notificationButtonPressed = Blend::PlusDarker(Rgba::Hex(0xE6E6E6), palette.notificationBackground);
        palette.notificationStackNear = Blend::Mix(palette.notificationBackground, Rgba::Black(1.0f), 0.04f);
        palette.notificationStackFar = Blend::Mix(palette.notificationBackground, Rgba::Black(1.0f), 0.07f);

        // Out of the key window accented controls turn gray (the switches of inactive windows @2x) and the sidebar drops
        // vibrancy; the toolbar dims only when the window is not main either (a window behind its sheet keeps it,
        // Battery Options @2x).
        if (state != ControlActiveState::Key) {
            accent = Rgba::Hex(0xA9A9A9);
            palette.sidebarBackground = Tinted(Rgba::Hex(0xF0F0F0), tint);
            palette.sidebarLabel = Rgba::Black(0.28f);
        }
        if (state == ControlActiveState::Inactive) {
            palette.toolbarTitle = Rgba::Black(0.27f);
            palette.toolbarSymbol = Rgba::Black(0.27f);
            palette.toolbarSymbolDisabled = Rgba::Black(0.13f);
        }
        palette.accent = accent;
        palette.controlAccent = accent;
        palette.accentPressed = Pressed(accent);
        palette.switchOnTop = accent;
        palette.switchOnBottom = Blend::Mix(accent, Rgba::White(1.0f), 0.135f);
        palette.textSelection = state == ControlActiveState::Key ? HighlightOf(accent, Appearance::Light) : Rgba::Hex(0xDCDCDC);
        palette.selection = state == ControlActiveState::Key ? Selection(accent, Appearance::Light) : Rgba::Black(0.105f);
        palette.unfocusedSelection = Rgba::Black(0.105f);
        palette.tableSelection = ContentSelection(accent, Appearance::Light);
        palette.sheetSidebarBackground = palette.sidebarBackground;
        palette.sheetSidebarDivider = palette.sidebarDivider;
        palette.sheetSelection = FocusedSelection(accent, Appearance::Light);
        palette.menuSelection = Selection(accent, Appearance::Light);
        // A light that does not work is lighter in the key window than under a sheet (Finder's copy dialog @2x: 220).
        if (state == ControlActiveState::Key)
            palette.trafficDisabled = Rgba::Hex(0xDCDCDB);
        return palette;
    }

    // Dark values come from the dark System Settings screenshots and still need a neutral reference.
    static Palette MacDark(Rgba accent, ControlActiveState state, Rgba tint) {
        Palette palette = {
            .label = Rgba::White(0.85f),
            .secondaryLabel = Rgba::White(0.55f),
            .tertiaryLabel = Rgba::White(0.25f),
            .quaternaryLabel = Rgba::White(0.1f),
            .quinaryLabel = Rgba::White(0.05f),
            .fill = Rgba::White(0.1f),
            .secondaryFill = Rgba::White(0.08f),
            .tertiaryFill = Rgba::White(0.05f),
            .quaternaryFill = Rgba::White(0.03f),
            .quinaryFill = Rgba::White(0.02f),
            .separator = Rgba::White(0.1f),
            .selectedContent = Rgba::White(1.0f),
            .selectedSecondaryContent = Rgba::White(0.7f),
            .windowBackground = Tinted(Rgba::Hex(0x2D2D2D), tint),
            .windowRim = Rgba::White(0.2f),
            .windowHighlight = Rgba::White(0.2f),
            .windowOutline = Rgba::Black(0.9f),
            .sidebarBackground = Tinted(Rgba::Hex(0x353535), tint),
            .sidebarDivider = Rgba::Black(1.0f),
            .splitDivider = Rgba::Black(1.0f),
            .scrollerThumb = Rgba::White(0.5f),
            .scrollerTrack = Rgba::White(0.075f),
            .sidebarLabel = Rgba::White(0.92f),
            .searchField = Rgba::White(0.06f),
            .searchFieldBorder = Rgba::White(0.08f),
            .toolbarBackground = Tinted(Rgba::Hex(0x272727), tint),
            .titleBarBackground = Tinted(Rgba::Hex(0x272727), tint, 1.25f),
            .toolbarTitle = Rgba::White(0.92f),
            .toolbarSymbol = Rgba::White(0.55f),
            .toolbarSymbolDisabled = Rgba::White(0.22f),
            .toolbarLine = Rgba::Black(0.5f),
            .scrollEdgeBackground = Tinted(Rgba::Hex(0x333333), tint),
            .scrollEdgeLine = Rgba::Black(1.0f),
            .sidebarScrollEdge = Rgba::White(0.1f),
            .sectionBackground = Rgba::White(0.02f),
            .sectionBorder = Rgba::White(0.15f),
            .sectionInnerBorder = Rgba::White(0.05f),
            .rowSeparator = Rgba::White(0.047f),
            .listBarLine = Rgba::White(0.2f),
            .controlBackground = Rgba::White(0.25f),
            .comboBoxBackground = Rgba::White(0.05f),
            .controlBorder = Rgba::Black(0.07f),
            .controlPressed = Rgba::White(0.1f),
            .knob = Rgba::Hex(0xC9C9C9),
            .switchTrack = Rgba::White(0.1f),
            .track = Rgba::White(0.1f),
            .levelIndicatorTrack = Rgba::White(0.25f),
            .trackTick = Rgba::White(0.2f),
            .sliderKnob = Rgba::Hex(0x949597),
            .popUpIndicator = Rgba::White(0.1f),
            .popUpPressed = Rgba::White(0.1f),
            .fieldBackground = Rgba::Hex(0x1E1E1E),
            .tokenBackground = Rgba::White(0.065f),
            .fieldBorder = Rgba::White(0.12f),
            .fieldSymbol = Rgba::White(0.55f),
            .fieldClearButton = Rgba::White(0.55f),
            .imageWellFill = Rgba::White(0.04f),
            .imageWellRim = Rgba::White(0.1f),
            .imageWellEdge = Rgba::Black(0.3f),
            .colorWellTop = Rgba::Hex(0x5A5A5A),
            .colorWellBottom = Rgba::Hex(0x4A4A4A),
            .colorWellLine = Rgba::Hex(0x1E1E1E),
            .link = Rgba::Hex(0x419CFF),
            .accessoryBarPlate = Rgba::White(0.15f),
            .accessoryBarSelectedText = Rgba::White(0.85f),
            .accessoryBarOutline = Rgba::White(0.3f),
            .tableBackground = Rgba::Hex(0x1E1E1E),
            .tableStripe = Rgba::White(0.03f),
            .tableDivider = Rgba::White(0.1f),
            .tableSelectionSplit = Rgba::White(0.16f),
            .tableSortIndicator = Rgba::White(0.4f),
            .boxBorder = Rgba::White(0.15f),
            .chartGrid = Rgba::White(0.15f),
            .sheetBackground = Rgba::Hex(0x1E1E1E),
            .sheetDim = Rgba::Black(0.38f),
            .alertBackground = Tinted(Rgba::Hex(0x303030), tint).Opacity(0.91f),
            .attachedAlertBackground = Rgba::Hex(0x303030).Opacity(0.91f),
            .alertText = Rgba::White(1.0f),
            .destructiveFill = Rgba::Hex(0xE63C41),
            .alertButton = Rgba::White(0.29f),
            .alertButtonPressed = Rgba::White(0.25f),
            .alertButtonText = Rgba::White(0.85f),
            .alertField = Rgba::White(0.085f),
            .alertFieldRim = Rgba::White(0.13f),
            .alertFieldHighlight = Rgba::White(0.39f),
            .popoverBackground = Rgba::Hex(0x2F2F2F),
            .popoverBorder = Rgba::Black(0.85f),
            // Dark/Materials/Tooltip #282828@0.6 over a dark window, and the dark vibrant text.
            .tooltipBackground = Rgba::Hex(0x2A2A2A),
            .tooltipText = Rgba::Hex(0xE5E5E5),
            .tooltipBorder = Rgba::Black(0.6f),
            // Over a dark forest the material goes to #303030 (Ars Technica @2x); the text stays an opaque #F0F0F0.
            .notificationBackground = Tinted(Rgba::Hex(0x626262), tint),
            .notificationRim = Rgba::White(0.15f),
            .notificationText = Rgba::Hex(0xF0F0F0),
            .menuBackground = Tinted(Rgba::Hex(0x2B2B2B), tint),
            .menuBorder = Rgba::Black(0.9f),
            .menuRim = Rgba::White(0.2f),
            .menuSeparator = Rgba::White(0.14f),
            .menuBadge = Rgba::White(0.1f),
            .menuBarBackground = Rgba::Black(0.2f),
            .menuBarTitle = Rgba::White(1.0f),
            .menuBarSelection = Rgba::White(0.2f),
            .trafficClose = Rgba::Hex(0xFF5F57),
            .trafficMinimize = Rgba::Hex(0xFEBC2E),
            .trafficZoom = Rgba::Hex(0x28C840),
            .trafficInactive = Rgba::White(0.17f),
            .trafficDisabled = Rgba::Hex(0x696969),
            .trafficBorder = Rgba::Black(0.2f),
        };
        palette.sidebarSecondaryLabel = Blend::PlusLighter(Rgba::Hex(0x7C7C7C), palette.sidebarBackground);
        palette.sidebarTertiaryLabel = Blend::PlusLighter(Rgba::Hex(0x414141), palette.sidebarBackground);
        palette.menuHeader = Blend::PlusLighter(Rgba::Hex(0x414141), palette.menuBackground);
        palette.menuShortcut = Rgba::White(0.25f);
        palette.notificationTimestamp = Blend::PlusLighter(Rgba::Hex(0x7C7C7C), palette.notificationBackground);
        palette.notificationButton = Blend::PlusLighter(Rgba::Hex(0x070707), palette.notificationBackground);
        palette.notificationButtonPressed = Blend::PlusLighter(Rgba::Hex(0x141414), palette.notificationBackground);
        palette.notificationStackNear = Blend::Mix(palette.notificationBackground, Rgba::Black(1.0f), 0.3f);
        palette.notificationStackFar = Blend::Mix(palette.notificationBackground, Rgba::Black(1.0f), 0.53f);

        // Out of the key window the sidebar takes the window's background (Appearance behind Notes @2x).
        if (state != ControlActiveState::Key) {
            accent = Rgba::Hex(0x6E6E6E);
            palette.sidebarBackground = palette.windowBackground;
            palette.sidebarLabel = Rgba::White(0.3f);
        }
        if (state == ControlActiveState::Inactive) {
            palette.toolbarTitle = Rgba::White(0.3f);
            palette.toolbarSymbol = Rgba::White(0.25f);
            palette.toolbarSymbolDisabled = Rgba::White(0.12f);
        }
        palette.accent = accent;
        palette.controlAccent = ScaledAccent(accent, 0.903f, 0.88f, -1.17f);
        palette.accentPressed = Blend::Over(Rgba::White(0.15f), palette.controlAccent);
        // The on track darkens toward the top by 7% (Wi-Fi details @2x); gray out of the key window it is flat
        // (Appearance @2x).
        palette.switchOnTop = state == ControlActiveState::Key ? ScaledAccent(palette.controlAccent, 1.0f, 0.93f) : palette.controlAccent;
        palette.switchOnBottom = palette.controlAccent;
        palette.textSelection = state == ControlActiveState::Key ? HighlightOf(accent, Appearance::Dark) : Rgba::Hex(0x464646);
        palette.selection = state == ControlActiveState::Key ? Selection(accent, Appearance::Dark) : Rgba::White(0.1f);
        palette.unfocusedSelection = Rgba::White(0.1f);
        palette.tableSelection = ContentSelection(accent, Appearance::Dark);
        palette.sheetSidebarBackground = Tinted(Rgba::Hex(0x464646), tint);
        palette.sheetSidebarDivider = Rgba::Black(1.0f);
        palette.sheetSelection = FocusedSelection(accent, Appearance::Dark);
        palette.menuSelection = palette.sheetSelection;
        return palette;
    }

    // Every field of a palette is a color, so two palettes mix field by field.
    static Palette Mix(const Palette& from, const Palette& to, float amount) {
        static_assert(sizeof(Palette) % sizeof(Rgba) == 0);
        Palette mixed = from;
        Rgba* colors = reinterpret_cast<Rgba*>(&mixed);
        const Rgba* targets = reinterpret_cast<const Rgba*>(&to);
        for (size_t i = 0; i < sizeof(Palette) / sizeof(Rgba); ++i)
            colors[i] = Blend::Crossfade(colors[i], targets[i], amount);
        return mixed;
    }

    struct PaletteKey {
        Platform platform;
        Appearance appearance;
        AccentColor accent;
        ControlActiveState controlActiveState;
        float controlActiveAmount;
        float tint[3];

        bool operator==(const PaletteKey&) const = default;
    };

    // A sheet and the window behind it take turns while both draw, so the last few palettes stay cached.
    const Palette& Colors() {
        struct CachedPalette {
            PaletteKey key;
            Palette palette;
            bool valid = false;
        };
        static CachedPalette cache[4];
        static int next = 0;

        const EnvironmentValues& environment = Environment();
        const float amount = environment.controlActiveState == ControlActiveState::Key ? 1.0f : ImSaturate(environment.controlActiveAmount);
        const PaletteKey key = {environment.platform, environment.appearance, environment.accent, environment.controlActiveState, amount, {environment.wallpaperTint.r, environment.wallpaperTint.g, environment.wallpaperTint.b}};
        for (const CachedPalette& entry : cache) {
            if (entry.valid && entry.key == key)
                return entry.palette;
        }

        const Rgba accent = Accent(environment.accent, environment.appearance);
        const auto palette_for = [&](ControlActiveState state) {
            return environment.IsDark() ? MacDark(accent, state, environment.wallpaperTint) : MacLight(accent, state, environment.wallpaperTint);
        };
        CachedPalette& entry = cache[next];
        next = (next + 1) % IM_ARRAYSIZE(cache);
        entry.palette = palette_for(environment.controlActiveState);
        if (amount < 1.0f)
            entry.palette = Mix(palette_for(ControlActiveState::Key), entry.palette, amount);
        entry.key = key;
        entry.valid = true;
        return entry.palette;
    }

    // Light: fitted to the shadow alpha of six key windows captured with screencapture (512 Pixels @2x). Dark: fitted to
    // a window capture along its edges (the kit's blur 100 at offset 36 spreads far wider than the screen).
    std::span<const Shadow> WindowShadows() {
        static const Shadow light[] = {
            {Rgba::Black(0.4f), ImVec2(0.0f, 18.0f), 40.0f},
        };
        static const Shadow dark[] = {
            {Rgba::Black(0.49f), ImVec2(0.0f, 17.5f), 38.0f},
        };
        return Environment().IsDark() ? std::span<const Shadow>(dark) : std::span<const Shadow>(light);
    }

    // A sheet casts a lighter shadow on its window than a window on the desktop (File Sharing @2x).
    std::span<const Shadow> SheetShadows() {
        static const Shadow light[] = {
            {Rgba::Black(0.287f), ImVec2(0.0f, 17.5f), 38.0f},
        };
        return Environment().IsDark() ? WindowShadows() : std::span<const Shadow>(light);
    }

    // Fitted to native full-screen captures of menus (512 Pixels, Notes' File menu over its window, light and dark): a
    // soft shadow 5 pt low that fades out over 15 pt at the sides.
    std::span<const Shadow> MenuShadows() {
        static const Shadow light[] = {
            {Rgba::Black(0.235f), ImVec2(0.0f, 5.0f), 16.0f},
        };
        static const Shadow dark[] = {
            {Rgba::Black(0.28f), ImVec2(0.0f, 5.0f), 16.0f},
        };
        return Environment().IsDark() ? std::span<const Shadow>(dark) : std::span<const Shadow>(light);
    }

    // Controls/Tooltip: a soft shadow 2 pt low; the kit's hairline shadow is the border.
    std::span<const Shadow> TooltipShadows() {
        static const Shadow light[] = {
            {Rgba::Black(0.2f), ImVec2(0.0f, 2.0f), 6.0f},
        };
        static const Shadow dark[] = {
            {Rgba::Black(0.4f), ImVec2(0.0f, 2.0f), 6.0f},
        };
        return Environment().IsDark() ? std::span<const Shadow>(dark) : std::span<const Shadow>(light);
    }

    // Fitted to the alpha of window captures of banners (Eject warning, Tahoe @2x): a hairline of shadow along the edge
    // and a halo that fades out 10 pt away, both centered.
    std::span<const Shadow> NotificationShadows() {
        static const Shadow light[] = {
            {Rgba::Black(0.34f), ImVec2(0.0f, 0.0f), 1.3f},
            {Rgba::Black(0.19f), ImVec2(0.0f, 0.0f), 8.2f},
        };
        static const Shadow dark[] = {
            {Rgba::Black(1.0f), ImVec2(0.0f, 0.0f), 1.0f},
            {Rgba::Black(0.24f), ImVec2(0.0f, 0.0f), 8.2f},
        };
        return Environment().IsDark() ? std::span<const Shadow>(dark) : std::span<const Shadow>(light);
    }

    // Fitted on native @2x push and help buttons (File Sharing sheet): a tight shadow that darkens the lower edge and a
    // soft halo all around, both darker than the kit's layer styles.
    std::span<const Shadow> BezelShadows() {
        static const Shadow light[] = {
            {Rgba::Black(0.2f), ImVec2(0.0f, 0.625f), 0.8f},
            {Rgba::Black(0.12f), ImVec2(0.0f, 0.0f), 2.5f},
        };
        static const Shadow dark[] = {
            {Rgba::Black(0.3f), ImVec2(0.0f, 0.5f), 0.5f},
        };
        return Environment().IsDark() ? std::span<const Shadow>(dark) : std::span<const Shadow>(light);
    }

    // Dark checkboxes (Spotlight @2x) and buttons (Wi-Fi details sheet @2x): the top edge lighter for half a point, by
    // 30% over the accent and 20% over gray.
    std::span<const Shadow> BezelHighlights(bool accent) {
        static const Shadow over_accent[] = {
            {Rgba::White(0.3f), ImVec2(0.0f, 0.5f), 0.0f},
        };
        static const Shadow over_gray[] = {
            {Rgba::White(0.2f), ImVec2(0.0f, 0.5f), 0.0f},
        };
        if (!Environment().IsDark())
            return {};
        return accent ? std::span<const Shadow>(over_accent) : std::span<const Shadow>(over_gray);
    }

    std::span<const Shadow> KnobShadows(bool on) {
        static const Shadow off_shadows[] = {
            {Rgba::Black(0.05f), ImVec2(0.0f, 1.0f), 0.75f},
            {Rgba::Black(0.15f), ImVec2(0.0f, 0.25f), 0.25f},
        };
        static const Shadow on_shadows[] = {
            {Rgba::Black(0.15f), ImVec2(0.0f, 0.2f), 0.25f},
        };
        return on ? std::span<const Shadow>(on_shadows) : std::span<const Shadow>(off_shadows);
    }

    // Light tracks are shaded from the top edge; dark ones have a light rim all around, on or off (Wi-Fi details and
    // Appearance @2x).
    std::span<const Shadow> SwitchTrackInnerShadows(bool on) {
        static const Shadow off_shadows[] = {
            {Rgba::Black(0.12f), ImVec2(0.0f, 0.5f), 1.5f, 0.25f},
            {Rgba::Black(0.02f), ImVec2(0.0f, 0.0f), 1.0f},
        };
        static const Shadow on_shadows[] = {
            {Rgba::Black(0.12f), ImVec2(0.0f, 0.5f), 0.0f},
        };
        static const Shadow dark_rim[] = {
            {Rgba::White(0.2f), ImVec2(0.0f, 0.0f), 0.6f, 0.5f},
        };
        if (Environment().IsDark())
            return dark_rim;
        return on ? std::span<const Shadow>(on_shadows) : std::span<const Shadow>(off_shadows);
    }

    std::span<const Shadow> WellInnerShadows() {
        static const Shadow shadows[] = {
            {Rgba::Black(0.1f), ImVec2(0.0f, 0.0f), 2.0f},
            {Rgba::Black(0.1f), ImVec2(0.0f, 1.0f), 2.0f},
        };
        return shadows;
    }

    std::span<const Shadow> SegmentedWellInnerShadows() {
        static const Shadow shadows[] = {
            {Rgba::Black(0.05f), ImVec2(0.0f, 0.0f), 2.0f},
            {Rgba::Black(0.05f), ImVec2(0.0f, 0.0f), 4.0f},
            {Rgba::Black(0.05f), ImVec2(0.0f, 0.0f), 2.0f},
        };
        return shadows;
    }

    std::span<const Shadow> ListShadows() {
        static const Shadow shadows[] = {
            {Rgba::Black(0.1f), ImVec2(0.0f, 1.0f), 0.0f},
        };
        return shadows;
    }

    std::span<const Shadow> IconPlateShadows() {
        static const Shadow shadows[] = {
            {Rgba::Black(0.3f), ImVec2(0.0f, 0.25f), 0.6f},
            {Rgba::Black(0.06f), ImVec2(0.0f, 0.0f), 0.5f},
        };
        return shadows;
    }

    // Measured on Setup Assistant @2x: under a text field's bottom edge 0.5 pt at 0.24 and 0.5 pt at 0.12 with the
    // outline, which are the kit's hard shadows of a focused field.
    std::span<const Shadow> FieldShadows() {
        static const Shadow light[] = {
            {Rgba::Black(0.12f), ImVec2(0.0f, 1.0f), 0.0f},
            {Rgba::Black(0.06f), ImVec2(0.0f, 0.5f), 0.0f},
        };
        static const Shadow dark[] = {
            {Rgba::Black(0.3f), ImVec2(0.0f, 1.0f), 0.0f},
        };
        return Environment().IsDark() ? std::span<const Shadow>(dark) : std::span<const Shadow>(light);
    }

    // Fading out within 3 pt in a main window (Privacy & Security @2x: 0.15, 0.09, 0.055, 0.03 per pixel row), within one
    // point in an inactive one (Desktop & Dock inactive @2x: 0.154, 0.049).
    std::span<const Shadow> ScrollEdgeShadows() {
        static const Shadow light[] = {
            {Rgba::Black(0.31f), ImVec2(0.0f, 0.0f), 2.7f},
        };
        static const Shadow light_inactive[] = {
            {Rgba::Black(0.39f), ImVec2(0.0f, 0.0f), 1.2f},
        };
        static const Shadow dark[] = {
            {Rgba::Black(0.6f), ImVec2(0.0f, 0.0f), 2.7f},
        };
        if (Environment().IsDark())
            return dark;
        return Environment().controlActiveState == ControlActiveState::Inactive ? std::span<const Shadow>(light_inactive) : std::span<const Shadow>(light);
    }

    std::span<const Shadow> ImageWellInnerShadows() {
        static const Shadow shadows[] = {
            {Rgba::Black(0.14f), ImVec2(0.0f, 0.5f), 1.5f},
        };
        return shadows;
    }

    std::span<const Shadow> TrackInnerShadows() {
        static const Shadow shadows[] = {
            {Rgba::Black(0.04f), ImVec2(0.0f, 0.0f), 2.0f},
            {Rgba::Black(0.03f), ImVec2(0.0f, 0.0f), 2.0f},
            {Rgba::Black(0.02f), ImVec2(0.0f, 1.0f), 2.0f},
        };
        return shadows;
    }

    // Fitted to a checked checkbox (AirPods @2x) and a default button (Battery Options @2x): the kit's three glows spread
    // twice as far as the screen's.
    std::array<Shadow, 2> ControlShadows(Rgba accent) {
        if (Environment().IsDark())
            return {Shadow{Rgba::Black(0.15f), ImVec2(0.0f, 0.0f), 0.0f, 0.5f}, Shadow{Rgba::Black(0.2f), ImVec2(0.0f, 0.5f), 0.5f}};
        return {Shadow{accent.WithAlpha(0.3f), ImVec2(0.0f, 0.35f), 1.3f}, Shadow{}};
    }
} // namespace Cupertino::Theme
