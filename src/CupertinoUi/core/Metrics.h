#pragma once

#include "Environment.h"

#include "imgui.h"
#include "imgui_internal.h"

// Sizes of controls and containers in points, from the macOS 15 and iOS 18 kits and calibrated screenshots.
namespace Cupertino::Metrics {
    // A switch's track and knob; its title stands 8 pt before it outside forms.
    struct SwitchMetrics {
        ImVec2 track;
        float knob = 0.0f;
        float labelSpacing = 8.0f;
    };

    struct CheckboxMetrics {
        float box = 14.0f;
        float radius = 3.5f;
        float height = 16.0f;
        float labelSpacing = 6.0f;
    };

    struct RadioMetrics {
        float diameter = 14.0f;
        float height = 16.0f;
        float labelSpacing = 6.0f;
        float rowPitch = 19.25f;
    };

    // chevron.up over chevron.down (pop-up indicators, steppers): the symbols' size in Heavy, the frame each is centered
    // in and the frames' tops from the indicator's top.
    struct ChevronPair {
        float size = 7.5f;
        ImVec2 frame = ImVec2(9.0f, 10.0f);
        float upperY = 0.0f;
        float lowerY = 6.5f;
    };

    // Borderless pop-up in a form row, measured on System Settings @2x: the value, a 3 pt gap and a 16 pt indicator
    // plate 12 pt from the section edge and 1 pt below the text line. The pressed underlay spans the whole button.
    struct FormPopUpMetrics {
        float height = 20.0f;
        float labelInset = 6.0f;
        float gap = 3.0f;
        float indicator = 16.0f;
        float indicatorRadius = 4.0f;
        float indicatorDrop = 1.0f;
        float trailingInset = 2.0f;
        ChevronPair chevrons;
        float rowTrailing = 12.0f;
        float pressedRadius = 5.0f;
        // The row's label and description wrap 18 pt before the button, 24 pt before its value: Lock Screen (dark @2x)
        // keeps a word 28.5 pt before one value and wraps one 22.5 pt before another.
        float descriptionTrailing = 18.0f;
        // An item image in front of the value (the Highlight color swatch), 4 pt before the text and centered on the
        // plate, a point under the text line (Appearance @2x).
        float imageSpacing = 4.0f;
    };

    // Pop-up button with a white bezel: a 20 pt pill in a 22 pt frame, the label, and an accent plate with chevrons.
    // Pop-up and pull-down buttons by control size (SwiftUI's controlSize on macOS, Fleeting Pixels @2x): the bezel 13, 16,
    // 20 and 28 pt with its plate 2 pt inside (9, 12, 16, 24 pt), labels 9, 11, 13 and 13 pt.
    struct BezelPopUpMetrics {
        float labelSize = 13.0f;
        float height = 22.0f;
        float bezelInset = 1.0f;
        float radius = 5.0f;
        float labelInset = 8.0f;
        float gap = 10.0f;
        float indicator = 16.0f;
        float indicatorRadius = 4.0f;
        float indicatorTop = 3.0f;
        float trailingInset = 3.0f;
        ChevronPair chevrons = ChevronPair{7.0f, ImVec2(9.0f, 9.0f), 0.5f, 7.0f};
        // A pull-down's plate sits 3.5 pt from the top with a single chevron.down, Heavy 7 in a 9 x 9 frame (kit).
        float pullDownIndicatorTop = 3.5f;
        // A pop-up's title stands 8 pt before its button outside forms.
        float titleSpacing = 8.0f;
        // The arrows-only pop-up (kit): a 22 pt square with Heavy 9 chevrons in 9 x 11 frames from (6.5, 2), 6.4 pt apart.
        float arrowsButton = 22.0f;
        float arrowsChevronX = 6.5f;
        ChevronPair arrowsChevrons = ChevronPair{9.0f, ImVec2(9.0f, 11.0f), 2.0f, 8.4f};
        // A combo box's plate stands 2 pt from its end, its chevron a point lower than a pull-down's (Connect to Server
        // @2x).
        float comboIndicatorTop = 3.0f;
        float comboTrailingInset = 2.0f;
        // A combo box's text stands 10 pt in, as a text field's, and its chevron is Heavy 8, a little above the plate's
        // middle (AppKit docs @2x: 7 x 4.5 pt).
        ImVec2 comboChevron = ImVec2(3.5f, 3.75f);
        float comboChevronSize = 8.0f;
        float comboTextInset = 10.0f;
        ImVec2 downChevron = ImVec2(4.0f, 3.5f);
        ImVec2 downChevronFrame = ImVec2(9.0f, 9.0f);
        // A pull-down showing a symbol (the add button of Displays @2x): the symbol 9.75 pt in, the plate 7.25 pt after
        // it, 2 pt from the end and 3 pt from the top.
        float symbolInset = 9.75f;
        float symbolGap = 7.25f;
        float symbolTrailingInset = 2.0f;
        float symbolIndicatorTop = 3.0f;
        // Without the plate: the symbol or title 8 pt in, chevron.down Heavy 8 in a 7.5 x 10 frame 6.5 pt after a
        // symbol (the actions pull-down of Network @2x) or 7 pt after a title (Add Photo in Wallpaper @2x), 6.25 pt
        // from the end.
        float plainInset = 8.0f;
        float plainChevronGap = 6.5f;
        float plainTitleGap = 7.0f;
        float plainChevronSize = 8.0f;
        ImVec2 plainChevronFrame = ImVec2(7.5f, 10.0f);
        float plainChevronTop = 6.0f;
        float plainTrailingInset = 6.25f;
    };

    // Single-select segmented control: a 22 pt well split evenly, the selected segment a 20 pt white pill 1 pt inside
    // it, labels 1 pt above the middle like button titles.
    struct SegmentedMetrics {
        float height = 22.0f;
        float inset = 1.0f;
        float wellRadius = 6.0f;
        float pillRadius = 5.0f;
        float labelPadding = 8.0f;
        ImVec2 separator = ImVec2(1.0f, 12.0f);
        // In a toolbar (Activity Monitor @2x): an outline with 6 pt corners, segments 1 pt apart with the separators in
        // the gaps, the selected one filled edge to edge.
        float toolbarRadius = 6.0f;
        float toolbarLabelPadding = 9.0f;
        ImVec2 toolbarSeparator = ImVec2(1.0f, 20.0f);
        // Segments that toggle on their own (kit's Multi Select): a 20 pt push button bezel.
        float togglesHeight = 20.0f;
    };

    // NSTokenField (Finder's tags field @2x): a 1 pt border, the text 1.5 pt in and its first line 0.5 pt down; tokens
    // with 5 pt of padding, 3 pt apart, lines 2 pt apart, their corners 2 pt as Console's search tokens (@2x).
    struct TokenFieldMetrics {
        float border = 1.0f;
        float textInset = 1.5f;
        float lineTop = 0.5f;
        float tokenInset = 0.0f;
        float tokenPadding = 5.0f;
        float tokenRadius = 2.0f;
        float tokenSpacing = 3.0f;
        float lineSpacing = 2.0f;
    };

    // ControlGroup (Connect to Server @2x): one 20 pt bezel of momentary segments, 24.5 pt for a symbol and 37 pt for a
    // menu (its symbol centered 13 pt in, a chevron.down 28 pt in), split by 0.5 x 11 pt lines in the separator color.
    struct ControlGroupMetrics {
        float height = 20.0f;
        float radius = 5.0f;
        float symbolSegment = 24.5f;
        float menuSegment = 37.0f;
        float menuSymbolX = 13.0f;
        float menuChevronX = 28.0f;
        float chevronSize = 9.0f;
        float labelPadding = 8.0f;
        ImVec2 separator = ImVec2(0.5f, 11.0f);
    };

    // Linear slider: a 4 pt track through a 20 pt round knob, the fill to its center; with ticks an 8 x 20 bar knob and 2 x 8
    // ticks. System Settings @2x: 10 pt captions on a baseline 12 pt under it in 15 pt, centered on a form row's label line.
    // By control size (Fleeting Pixels @2x): knobs 13, 16 and 20 pt (large as regular), the track 3 pt at mini.
    struct SliderMetrics {
        float height = 20.0f;
        float trackHeight = 4.0f;
        float knob = 20.0f;
        ImVec2 barKnob = ImVec2(8.0f, 20.0f);
        ImVec2 tick = ImVec2(2.0f, 8.0f);
        float minWidth = 100.0f;
        // Between the track and the symbols at its ends (Sound @2x).
        float symbolSpacing = 4.0f;
        // Large symbols stand in a 20.5 pt frame, half a point taller than the slider, which centers on them (Brightness
        // in Displays @2x: a 39.5 pt row).
        float largeSymbolHeight = 20.5f;
        float labelSpacing = 6.0f;
        float captionSize = 10.0f;
        float captionBaseline = 12.0f;
        float captionHeight = 15.0f;
        // In a form row the slider's frame starts 9 pt from the row's top, its track on the label's line (Trackpad @2x).
        float formTop = 9.0f;
        // A slider with ticks and no captions sits 0.75 pt lower in its row and keeps 2.25 pt under its frame, as
        // AppKit's does for its tick marks: the Alert volume row of Sound @2x is 41 pt.
        float tickedFormTop = 8.75f;
        float tickRoom = 2.25f;
        // The circular slider (kit): a 22 pt bezel with a 6 pt dot 7 pt from its center.
        float circular = 22.0f;
        float circularKnob = 6.0f;
        float circularKnobDistance = 7.0f;
    };

    // Stepper arrows: a 13 x 20 pill split in two, chevrons Heavy 7.5 in 8 x 9 frames 2.6 pt in. With a value, the value
    // sits in a 24 pt outline 6 pt from its leading edge, and the arrows follow 4 pt later, 2 pt from the trailing edge.
    struct StepperMetrics {
        ImVec2 size = ImVec2(13.0f, 20.0f);
        float radius = 5.0f;
        float chevronX = 2.6f;
        ChevronPair chevrons = ChevronPair{7.5f, ImVec2(8.0f, 9.0f), 0.5f, 10.5f};
        float labelSpacing = 6.0f;
        float fieldHeight = 24.0f;
        float fieldRadius = 6.0f;
        float fieldInset = 6.0f;
        float fieldGap = 4.0f;
        float fieldTrailing = 2.0f;
    };

    // Date pickers in the field and stepper style (Screen Time's Downtime): the stepper's value outline with the text on
    // its trailing side, the date and the time in two fields 8 pt apart; the selected part on an accent plate 1 pt wider
    // than its text on each side, r 3.
    struct DatePickerMetrics {
        float fieldSpacing = 8.0f;
        float partPadding = 1.0f;
        float partRadius = 3.0f;
        // The graphical calendar, in the look of Calendar's month in its sidebar (macOS 15 @2x): the month and year
        // Semibold 13 between chevrons, weekday letters Semibold 11, days 12 pt in 24 x 18 pt cells, the chosen day on an
        // 18 pt accent disc.
        float calendarColumn = 24.0f;
        float calendarRow = 18.0f;
        float calendarHeader = 24.0f;
        float calendarDisc = 18.0f;
        float calendarHeaderSize = 13.0f;
        float calendarWeekdaySize = 11.0f;
        float calendarDaySize = 12.0f;
        float calendarTimeSpacing = 8.0f;
    };

    // Text fields measured on Setup Assistant @2x: a 19 pt white field 1 pt below the top of a 22 pt frame, text 4 pt
    // in from its frame and centered on the white. In a form row the kit's outlined box: 24 pt, r 4, text 6.5 pt in.
    // Search fields follow the kit: r 5, magnifier at 7, text at 28, clear button 16 pt, 3 pt from the edge.
    struct TextFieldMetrics {
        float height = 22.0f;
        float fieldTop = 1.0f;
        float fieldHeight = 19.0f;
        float textInset = 4.0f;
        float minWidth = 60.0f;
        float formHeight = 24.0f;
        float formRadius = 4.0f;
        float formTextInset = 6.5f;
        float searchRadius = 5.0f;
        float searchSymbolX = 7.0f;
        float searchTextX = 28.0f;
        // In a toolbar (Activity Monitor and Passwords @2x): an outline on the bar, 6 pt corners, the magnifier and the
        // text closer to the edge and the text a point below the middle.
        float toolbarSearchRadius = 6.0f;
        float toolbarSearchSymbolX = 5.5f;
        float toolbarSearchTextX = 27.0f;
        float toolbarSearchTextOffset = 1.0f;
        float clearButton = 16.0f;
        float clearInset = 3.0f;
        // The insertion point (macOS 14 and later; Contacts, Safari Settings, Automator, Stage Manager @2x): a capsule
        // 2 pt wide centered on the insertion x, a text line tall.
        float caretWidth = 2.0f;
        // Between a field's label and the field when it stands outside a form.
        float labelSpacing = 8.0f;
        // A text editor's lines stand 5 pt in from its sides (NSTextContainer's lineFragmentPadding).
        float editorPadding = 5.0f;
        // A click this close around a recording key recorder keeps it recording (KeyboardShortcuts' clickMargin).
        float recorderClickMargin = 3.0f;
    };

    // Progress indicators (kit): bars 6 pt, 3 pt small, on the slider track; rings 32 / 16 pt with a 5 pt band; the
    // spinner's spokes are 2 x 5 pt in a 16 pt circle and scale with it.
    struct ProgressMetrics {
        float barHeight = 6.0f;
        float smallBarHeight = 3.0f;
        float minWidth = 100.0f;
        float ring = 32.0f;
        float smallRing = 16.0f;
        float ringBand = 5.0f;
        float spinner = 32.0f;
        float smallSpinner = 16.0f;
        int spokes = 8;
        ImVec2 spoke = ImVec2(2.0f, 5.0f);
        // The indeterminate bar's segment (HIG macOS art): a third of the track, the accent head fading linearly to nothing
        // at its tail, crossing the track from end to end every 1.5 s.
        float sweepLength = 1.0f / 3.0f;
        float sweepPeriod = 1.5f;
        // The labels 4 pt from the bar or the spinner.
        float labelSpacing = 4.0f;
    };

    // Path controls (Finder's path bar in the HIG @2x): 16 pt icons with the 11 pt name 4 pt after and half a point below
    // the middle, a chevron.right Semibold 7 in the label color 8 pt after a name, 6.5 pt before the next icon and 1.25 pt
    // below the middle; the pop-up's chevrons 21 pt from the end.
    struct PathControlMetrics {
        float height = 22.0f;
        float icon = 16.0f;
        float iconSpacing = 4.0f;
        float labelDrop = 0.5f;
        float fontSize = 11.0f;
        float chevronSize = 7.0f;
        float chevronWidth = 3.5f;
        float chevronLeading = 8.0f;
        float chevronTrailing = 6.5f;
        float chevronDrop = 1.25f;
        float popUpTrailing = 21.0f;
        ChevronPair chevrons = ChevronPair{7.5f, ImVec2(9.0f, 10.0f), 0.0f, 6.5f};
    };

    // Level indicators, in the proportions of the HIG's macOS art (no capture of one yet): a 16 pt bar with r 2.5 corners,
    // cells 1 pt apart; rating stars Regular 13, 16 pt apart.
    struct LevelIndicatorMetrics {
        float height = 16.0f;
        float radius = 2.5f;
        float cellGap = 1.0f;
        float minWidth = 96.0f;
        float starSize = 13.0f;
        float starPitch = 16.0f;
        float starHeight = 16.0f;
        float labelSpacing = 8.0f;
    };

    // Disclosure chevron (kit Lists/Forms): chevron.right Semibold 13 in an 8 x 16 frame. In a form row it sits 8 pt
    // from the row edge with the title at 23.5; elsewhere the title follows 15.5 pt after it and the content is
    // indented to the title.
    struct DisclosureMetrics {
        ImVec2 chevronFrame = ImVec2(8.0f, 16.0f);
        float formChevronX = 8.0f;
        float formLabelX = 23.5f;
        float labelX = 15.5f;
        float contentSpacing = 6.0f;
        // Where the kit's chevron.down of an open group sits relative to the turned chevron.right.
        ImVec2 openShift = ImVec2(2.5f, -1.0f);
        // The push disclosure button (kit): a 22 pt square, its chevron's frame at (5.5, 5).
        float button = 22.0f;
        ImVec2 buttonChevron = ImVec2(5.5f, 5.0f);
    };

    // Color well measured on System Settings > Accessibility > Display: a 44 x 24 push-button bezel, r 6, with the
    // color inset 3 pt (r 3). Its popover holds a grid of presets.
    struct ColorWellMetrics {
        ImVec2 size = ImVec2(44.0f, 24.0f);
        float radius = 6.0f;
        float inset = 3.0f;
        float swatchRadius = 3.0f;
        float labelSpacing = 8.0f;
        int columns = 10;
        int rows = 6;
        float swatch = 18.0f;
        float swatchSpacing = 3.0f;
        float gridPadding = 12.0f;
        float formTop = 8.0f;
        // AppKit's bordered well (kit): 28 x 20 with the color 4 pt inside.
        ImVec2 borderedSize = ImVec2(28.0f, 20.0f);
        float borderedInset = 4.0f;
    };

    // The Colors window (macOS guide art @1.6): a 299 x 479 pt panel; under the title the mode tiles, then the wheel, the
    // brightness track with its 12 x 22.5 knob, the opacity row and, under a line 66 pt from the bottom, the color and
    // two rows of ten saved colors.
    struct ColorPanelMetrics {
        ImVec2 size = ImVec2(299.0f, 479.0f);
        float screenMargin = 20.0f;
        float barHeight = 70.5f;
        ImVec2 modeTile = ImVec2(41.0f, 37.5f);
        float modeLeading = 4.0f;
        float modePitch = 42.0f;
        float modeTop = 27.0f;
        float wheelIcon = 25.0f;
        ImVec2 slidersIcon = ImVec2(29.0f, 24.0f);
        float wheelTop = 15.0f;
        float wheelRadius = 104.5f;
        float markerRadius = 7.25f;
        float markerArm = 10.0f;
        float inset = 15.0f;
        float trackSpacing = 19.5f;
        float trackHeight = 16.5f;
        float trackRadius = 3.5f;
        ImVec2 knob = ImVec2(12.0f, 22.5f);
        float knobRadius = 3.5f;
        ImVec2 tick = ImVec2(1.25f, 4.0f);
        float popUpTop = 14.0f;
        float popUpWidth = 160.0f;
        float channelTop = 58.0f;
        float channelPitch = 52.0f;
        float channelTrackDrop = 20.0f;
        float hexWidth = 80.0f;
        float opacityBaseline = 41.0f;
        float opacityCenter = 25.25f;
        float opacityLeading = 13.0f;
        float fieldWidth = 57.0f;
        float fieldSpacing = 8.0f;
        float bottomHeight = 66.0f;
        ImVec2 well = ImVec2(44.0f, 42.0f);
        float wellLeading = 10.25f;
        float wellTop = 9.0f;
        float wellRadius = 6.0f;
        float savedLeading = 91.5f;
        float savedTop = 12.0f;
        float savedSwatch = 16.25f;
        float savedPitch = 18.75f;
        float savedRadius = 2.5f;
        int savedColumns = 10;
        int savedRows = 2;
    };

    // The image well (kit): a 28 pt square with 2 pt corners 3 pt inside its 34 pt frame, a 1 pt rim, the image 2 pt
    // inside. A larger well (the current picture of Wallpaper @2x) sits 2.5 pt inside its frame with 4 pt corners and a
    // 2 pt rim, its picture 2 pt inside with 2 pt corners.
    struct ImageWellMetrics {
        float size = 34.0f;
        float inset = 3.0f;
        float radius = 2.0f;
        float imageInset = 2.0f;
        float rim = 1.0f;
        float imageRadius = 0.0f;
    };

    // Popovers (Safari page menu, window tiling popover @2x): r 10, an arrow 12.75 pt to its sharp tip over a 21.75 pt
    // base, the tip rounded over 2 pt of each side (12 pt to the drawn tip), the shoulders eased into the edge over 4 pt;
    // the sharp tip 2 pt from the anchor. They fade in and out in 0.15 s.
    struct PopoverMetrics {
        float radius = 10.0f;
        float arrowHeight = 12.75f;
        float arrowWidth = 21.75f;
        float arrowTip = 2.0f;
        float arrowShoulder = 4.0f;
        float gap = 2.0f;
        float margin = 5.0f;
        float fade = 0.15f;
    };

    // A settings window's tab bar (TV and Mail Settings @2x): 80.5 pt with its line, the title's baseline 18 pt down, cells
    // 45 pt tall from y 28, each its 11 pt label plus 7 pt a side (55 pt at least) with the 15.5 pt symbol above (the kit's
    // 13 pt is a fifth smaller), the selected one on a gray backing.
    struct TabBarMetrics {
        float height = 80.5f;
        float titleSize = 13.0f;
        float titleBaseline = 18.0f;
        float itemTop = 28.0f;
        float itemHeight = 45.0f;
        float labelSize = 11.0f;
        float labelPadding = 7.0f;
        float minItemWidth = 55.0f;
        float symbolSize = 15.5f;
        float symbolTop = 3.5f;
        float symbolLine = 24.0f;
        float labelTop = 27.0f;
        float backingRadius = 6.0f;
    };

    // Views/Boxes/Group Box: r 6 continuous; the content and a label above the box 12 pt in (Chess sheet @2x). Tabs on a
    // tab view's edge are 17.75 pt wider than their titles on each side (the kit's 136 pt for two "Label" tabs).
    struct GroupBoxMetrics {
        float radius = 6.0f;
        float padding = 12.0f;
        float tabPadding = 17.75f;
    };

    // Views/Boxes/Rounded and Standard Scroll Box: a 1 pt line just outside the frame, rounded 10 pt or square.
    // Its overlay scroller (macos_ui's MacosScrollbar): 6 pt, 9 pt over a track while the pointer is on it (0.1 s), 2 pt
    // from the edge, at least 18 pt long; it fades 1.2 s after scrolling stops, over 0.25 s.
    struct ScrollViewMetrics {
        float border = 1.0f;
        float roundedRadius = 10.0f;
        float scroller = 6.0f;
        float scrollerHover = 9.0f;
        float scrollerInset = 2.0f;
        float scrollerMinLength = 18.0f;
        float scrollerIdle = 1.2f;
        float scrollerFade = 0.25f;
        float scrollerGrow = 0.1f;
    };

    // Controls/Tooltip: Medium 11/13 text 6 pt from the sides, 3 pt from the top and 2 from the bottom, r 1; it wraps at
    // maxWidth and stands 18 pt under the pointer after a second of rest.
    struct TooltipMetrics {
        float textSize = 11.0f;
        float lineHeight = 13.0f;
        float horizontalInset = 6.0f;
        float top = 3.0f;
        float bottom = 2.0f;
        float radius = 1.0f;
        float maxWidth = 300.0f;
        float pointerOffset = 18.0f;
        // Where it would leave the screen it stands 4 pt over the pointer instead.
        float flippedGap = 4.0f;
        float delay = 1.0f;
        float fadeIn = 0.1f;
        float fadeOut = 0.075f;
    };

    // Alerts measured on macOS 15 alerts @2x: 260 pt wide, r 9.5 (circular), 16 pt margins, 28 pt buttons (r 6) 8 pt apart
    // side by side and 6 pt stacked, a cancel button 16 pt under the stack (Calendar and VS Code alerts, dark), the
    // suppression checkbox's line 16 pt under the buttons, a gray 20 pt help circle in the top-right corner.
    struct AlertMetrics {
        float width = 260.0f;
        float radius = 9.5f;
        float padding = 16.0f;
        // Title and message wrap in 220 pt: two alerts break their lines between 211 and 225 pt.
        float textInset = 20.0f;
        float top = 20.0f;
        float icon = 64.0f;
        float iconSpacing = 19.5f;
        // The caution triangle of a critical alert: its apex rounder than its base, in a 3 pt light rim.
        float cautionApexRadius = 6.0f;
        float cautionBaseRadius = 4.2f;
        float cautionRim = 3.0f;
        float messageSpacing = 10.0f;
        float buttonsSpacing = 16.0f;
        float buttonHeight = 28.0f;
        float buttonRadius = 6.0f;
        float buttonGap = 8.0f;
        // The room a title keeps on each side of a button in a row; titles that do not fit stack the buttons.
        float buttonTitleInset = 8.0f;
        float stackedGap = 6.0f;
        float cancelGap = 16.0f;
        float suppressionTrailing = 4.0f;
        float help = 20.0f;
        float helpTop = 15.5f;
        float helpTrailing = 16.0f;
        float helpSymbol = 15.0f;
        // Text fields (authorization dialog @2x): 19.5 pt tall across the buttons' row, r 5, the text 10 pt in; 8 pt under
        // the message, 29 pt apart, the buttons 19 pt under the last one.
        float fieldHeight = 19.5f;
        float fieldRadius = 5.0f;
        float fieldTextInset = 10.0f;
        float fieldsSpacing = 8.0f;
        float fieldPitch = 29.0f;
        float fieldsBottom = 19.0f;
    };

    // Notification banners on macOS 15 @2x (the Tahoe upgrade banner, Notification Center, Ars Technica's stack): 344 pt
    // wide, r 16, text from 58 pt in 16 pt lines, 12 pt under the top and 13 pt over the bottom, the 40 pt icon 9 pt in
    // and centered between them, actions 22 pt high 13 pt under the text and 12 pt over the bottom, across its column.
    struct NotificationMetrics {
        float width = 344.0f;
        float radius = 16.0f;
        float top = 12.0f;
        float bottom = 13.0f;
        float icon = 40.0f;
        float iconLeading = 9.0f;
        // An app icon's body keeps this margin of its canvas (Finder and Ars Technica icons: 32 pt).
        float iconMargin = 4.0f;
        float textLeading = 58.0f;
        float textTrailing = 14.0f;
        // The text is at least two lines high, so the icon keeps 8.5 pt above and below it.
        float minTextHeight = 32.0f;
        float timestampSpacing = 6.0f;
        // The kit's Image variant: a 32 pt thumbnail (r 6) 10 pt from the trailing edge and 9.5 pt over the bottom.
        float attachment = 32.0f;
        float attachmentRadius = 6.0f;
        float attachmentTrailing = 10.0f;
        float attachmentBottom = 9.5f;
        float attachmentSpacing = 8.0f;
        float actionHeight = 22.0f;
        float actionsBottom = 12.0f;
        float actionRadius = 6.0f;
        float actionGap = 8.0f;
        // A group's other notifications (Notification Center @2x, dark): up to two cards under the banner, each 8 pt lower,
        // 10.5 and 20.5 pt in on either side, with 12 pt corners.
        int stackCards = 2;
        float stackPeek = 8.0f;
        float stackInset = 10.5f;
        float stackInsetStep = 10.0f;
        float stackRadius = 12.0f;
        // Presented in the top-right corner of the screen, 16 pt from its edge and under the menu bar (Ars Technica and
        // Notification Center @2x), sliding in from the trailing edge; a banner without actions leaves after 5 s.
        float screenInset = 16.0f;
        float slide = 0.35f;
        float duration = 5.0f;
    };

    // Sheets on @2x System Settings and Finder captures: a 1 pt separator over the footer, whose buttons keep 20 pt from
    // the edges; the window behind keeps half its opacity under the dim.
    struct SheetMetrics {
        float separator = 1.0f;
        float footerPadding = 20.0f;
        // Done, Cancel and OK of the Battery Options, File Sharing and Wi-Fi details sheets (@2x).
        float footerButtonMinWidth = 64.0f;
        float backgroundOpacity = 0.5f;
        float fade = 0.25f;
        // A sheet sized by its content sits this much above the middle of its window, as if it were 16 pt taller
        // (Battery Options @2x: y 98 in 655); a sheet of fixed size is centered (Wi-Fi details: y 93 in 610).
        float contentLift = 8.0f;
    };

    struct FormMetrics {
        float margin = 20.0f;
        float topMargin = 0.0f;
        float sectionSpacing = 10.0f;
        // Circular corners (Battery and Software Update @2x; the kit draws 6 pt continuous ones).
        float sectionRadius = 5.0f;
        float rowHeight = 36.0f;
        float rowInset = 10.0f;
        float rowVerticalInset = 10.0f;
        float separatorInset = 10.0f;
        float separatorThickness = 1.0f;
        float valueTrailing = 10.0f;
        // A mini switch sits 12 pt from the row top (baseline-aligned with the label), which makes its row 37 pt.
        float switchTrailing = 10.0f;
        float switchTop = 12.0f;
        // Section headers (Desktop & Dock @2x): the line 30 pt under the previous section, 4 pt under the toolbar when
        // it opens the form, and 10 pt above its section.
        float headerSpacing = 10.0f;
        float headerTop = 30.0f;
        float firstHeaderTop = 4.0f;
        // A header's description starts 17.5 pt under the header line's top (Energy Mode @2x), half a point closer than
        // a row's description under its label.
        float headerDescriptionSpacing = 1.5f;
        // A pane header row (Bluetooth @2x): a 26 pt icon plate at (11, 12), its text from x 48.
        float largeIcon = 26.0f;
        float largeIconX = 11.0f;
        float largeIconTop = 12.0f;
        float largeIconTextX = 48.0f;
        float imageRowHeight = 50.0f;
        // A row that opens a page (Privacy & Security @2x): 42 pt with a 20 pt icon plate at the row inset and the title
        // from 40 pt, the value ending 26 pt before the row's end and a chevron at the trailing inset.
        float navigationRowHeight = 42.0f;
        float smallIcon = 20.0f;
        float smallIconTextX = 40.0f;
        float navigationValueTrailing = 26.0f;
        // A row description: Subheadline in the secondary color 2 pt under the label line. Label and description wrap 10 pt
        // before the accessory or the chevron: a word ending 11 pt before the switch stays (Wallet @2x), one ending 9 pt
        // before it wraps (Wi-Fi @2x).
        float descriptionSpacing = 2.0f;
        float descriptionTrailing = 10.0f;
        // Radio groups under a title: 14 pt circles from y 30.5, 19 pt apart, 11.5 pt below the last one (Appearance
        // @2x). Side by side on the trailing side of a row, the options stand 17.5 pt apart (Lock Screen @2x).
        float radioGroupTop = 30.5f;
        float radioGroupBottom = 11.5f;
        float radioRowSpacing = 17.5f;
        // Between a description's symbol and its text (the warning of Lock Screen @2x).
        float descriptionSymbolSpacing = 3.0f;
    };

    // Swift Charts in System Settings (Battery @2x of 15.0 and 15.1): 1 pt gridlines and verticals (dashed 2 and 2), bars
    // with 1.5 pt top corners, 12 pt labels; under the plot 20 pt rows for the charging band, the hours and the dates. The
    // full rules are in pickers-and-charts.md.
    struct ChartMetrics {
        float gridline = 1.0f;
        float barRadius = 1.5f;
        float dashWidth = 1.0f;
        float dash = 2.0f;
        float labelSpacing = 5.0f;
        float axisRow = 20.0f;
        float axisBottom = 8.0f;
        float bandGap = 9.0f;
        float bandCapsule = 4.0f;
        float bandOpacity = 0.2f;
        float xLabelBaseline = 17.0f;
        float xLabelInset = 3.0f;
        float dateSpacing = 21.0f;
        // Swift Charts' marks (the Charts documentation's renders @2x): labels 4 pt after the plot, below it 4.5 pt after
        // their line on a baseline 15 pt down; 2 pt lines and rules, 11.5 pt points; the legend 3 pt in, 8 pt dots 5.5 pt
        // before their names, entries 8 pt apart (as iOS 17 spaces them; the 2022 renders spread them wider).
        float markLabelSpacing = 4.0f;
        float markLabelInset = 4.5f;
        float markLabelBaseline = 15.0f;
        float markAxisRow = 17.5f;
        float lineWidth = 2.0f;
        float pointSize = 11.5f;
        float ruleWidth = 2.0f;
        float legendInset = 3.0f;
        float legendDot = 8.0f;
        float legendDotSpacing = 5.5f;
        float legendSpacing = 8.0f;
        float legendGap = 5.0f;
        // Selection: sectors not chosen fade to half (a donut's selection, iOS 17); the rule at the chosen value is gray
        // at 30 % and its annotation sits over the plot in a box of gray at 12 %, r 4, padded 6 pt.
        float fadedOpacity = 0.5f;
        float selectionRuleOpacity = 0.3f;
        float annotationOpacity = 0.12f;
        float annotationRadius = 4.0f;
        float annotationPadding = 6.0f;
    };

    // SwiftUI's Gauge (Use Your Loaf's renders, iOS 17, 1 px/pt): bars of 16, 4 and 8 pt with the label's baseline, the
    // value's capitals and the bounds set around them; rings 59 pt in 6 pt strokes (the iOS kit's widgets: 58 and 5.52) with
    // the kit's Medium 24.52 value and 12.26 labels, the open one's 5.5 pt dot in a 9.9 pt cut, the closed one's track 37 %.
    struct GaugeMetrics {
        float capacityThickness = 16.0f;
        float capacityLabelBaseline = 18.5f;
        float capacityValueSpacing = 14.0f;
        float capacityBoundSpacing = 8.0f;
        float capacityBoundLift = 1.0f;
        float accessoryCapacityThickness = 4.0f;
        float accessoryLabelBaseline = 9.0f;
        float accessoryValueSpacing = 6.5f;
        float accessoryCapacityBoundSpacing = 10.0f;
        float accessoryCapacityBoundLift = 3.5f;
        float accessoryThickness = 8.0f;
        float accessoryBoundSpacing = 9.5f;
        float markerClearance = 4.0f;
        float ringDiameter = 59.0f;
        float ringStroke = 6.0f;
        float ringMarker = 5.5f;
        float ringMarkerCut = 9.9f;
        float ringTrackOpacity = 0.37f;
        float ringValueSize = 24.52f;
        float ringLabelSize = 12.26f;
        float ringValueLift = 2.0f;
        float ringLabelBaseline = 22.0f;
        float ringValueInset = 6.0f;
        float ringBoundSpacing = 2.0f;
    };

    // Tables in the inset style (Activity Monitor @2x): a 28 pt header with 1 pt dividers 16 pt tall and a 1 pt line
    // under it, titles 12.5 pt in from the column's edges; 24 pt rows 10 pt in from the sides and 5 pt under the header,
    // r 5; cell text 9.5 pt after a column's start and 10 pt before its end; the sort chevron 8.5 pt from the end.
    struct TableMetrics {
        float headerHeight = 28.0f;
        float headerLine = 1.0f;
        float divider = 1.0f;
        float dividerHeight = 16.0f;
        float titleInset = 12.5f;
        float rowHeight = 24.0f;
        float rowInset = 10.0f;
        float contentTop = 5.0f;
        float rowRadius = 5.0f;
        float cellLeading = 9.5f;
        float cellTrailing = 10.0f;
        float sortChevronSize = 11.0f;
        float sortChevronInset = 8.5f;
        float sortChevronSpacing = 4.0f;
        // A divider takes a drag 3 pt to either side; a column is at least 20 pt wide.
        float dividerGrab = 3.0f;
        float minColumnWidth = 20.0f;
        float selectionSplit = 1.0f;
        // The full-width style (Sound @2x): titles 8 pt and cells 10.5 pt after their column's start.
        float fullWidthTitleInset = 8.0f;
        float fullWidthCellLeading = 10.5f;
        // The bordered style (Force Quit, TV Settings @2x): content 8 pt in from the table's edge, 7 inside its border;
        // cells keep the top 19 pt of the row, so a centered name stands 1.5 pt above the row's middle.
        float border = 1.0f;
        float borderedRowHeight = 22.0f;
        float borderedCellHeight = 19.0f;
        float borderedCellLeading = 7.0f;
        // + and − under a bordered table's rows (TV Settings @2x): a line and an 18 pt bar inside the border, + over the
        // first 18 pt from the table's edge and − over the next 15, each followed by a divider across the bar; Regular 11.
        float barHeight = 19.0f;
        ImVec2 barButtons = ImVec2(18.0f, 15.0f);
        float barSymbolSize = 11.0f;
        // An outline in the first column (kit's Lists/Tables): 15 pt deeper per level, chevron.right Bold 9 centered 8 pt
        // into the row, turned down while the row is expanded.
        float outlineIndent = 15.0f;
        float outlineChevronX = 8.0f;
        float outlineChevronSize = 9.0f;
    };

    // Bordered lists (File Sharing, Login Items, Language & Region @2x): rows 8 pt in from the leading edge and 12.5 pt from
    // the trailing one, a 24 pt bar of + and − under them, a 28 pt column header or a 36 pt title row over them.
    struct BorderedListMetrics {
        float rowLeading = 8.0f;
        float rowTrailing = 12.5f;
        float headerHeight = 28.0f;
        float headerFontSize = 11.0f;
        float titleHeight = 37.0f;
        float titleLeading = 11.0f;
        float footerHeight = 24.0f;
        float buttonWidth = 24.0f;
        float buttonDivider = 16.0f;
        float buttonSymbolSize = 11.0f;
        // Kit Lists/Small List: rows 8 pt in on both sides; + and − 28 pt wide in a plain list's bar; the actions pull-down
        // 40 pt with gear Medium 11 from 8 pt and chevron.down Bold 7 from 23 pt; an inset list's rows 4 pt below its top
        // and its selection 4 pt in with 5 pt corners, its + and − in a 41 x 20 bezel 9 pt under the box with a 14 pt
        // divider.
        float plainButtonWidth = 28.0f;
        float actionWidth = 40.0f;
        float actionSymbolSize = 11.0f;
        float actionSymbolX = 8.0f;
        float actionChevronSize = 7.0f;
        float actionChevronX = 23.0f;
        float actionChevronWidth = 9.0f;
        float insetRadius = 10.0f;
        float insetTop = 4.0f;
        float insetSelectionInset = 4.0f;
        float insetSelectionRadius = 5.0f;
        ImVec2 insetBezel = ImVec2(41.0f, 20.0f);
        float insetBezelSpacing = 9.0f;
        float insetBezelDivider = 14.0f;
    };

    // Push buttons by control size, the bezel being the layout frame: regular 20 pt, r 5, a 13 pt title a point above the
    // middle 8 pt from each side (native @2x); small 16 pt, r 4, 11 pt (Finder's copy dialog @2x); large 28 pt, r 6 as
    // alert buttons; mini 13 pt, r 3, 9 pt after AppKit.
    struct PushButtonMetrics {
        float height = 20.0f;
        float radius = 5.0f;
        float titleInset = 8.0f;
        float titleRaise = 1.0f;
        float titleSize = 13.0f;
        float titleLineHeight = 16.0f;
        // A symbol before the title stands this far from it (NSButton's image leading).
        float symbolGap = 4.0f;
    };

    // Accessory bars (Finder's search scope bar @2x): 30 pt over a point of separator; the title 7 pt in and 6 pt before
    // the scopes, items 2 pt apart and 10 pt from the end. Buttons are 16 pt, r 4, 7.5 pt around a 12 pt title (Bold on a
    // scope, Regular on an action) half a point above the middle, 18 pt with a small-scale symbol.
    struct AccessoryBarMetrics {
        float height = 30.0f;
        float leading = 7.0f;
        float trailing = 10.0f;
        float titleSpacing = 6.0f;
        float spacing = 2.0f;
        float buttonHeight = 16.0f;
        float radius = 4.0f;
        float padding = 7.5f;
        float titleRaise = 0.5f;
        float fontSize = 12.0f;
        float actionFontSize = 12.0f;
        float symbolWidth = 18.0f;
    };

    struct HelpButtonMetrics {
        float diameter = 20.0f;
        float symbolSize = 13.0f;
    };

    struct SidebarMetrics {
        float width = 215.0f;
        float divider = 1.0f;
        float topInset = 52.0f;
        float searchHeight = 28.0f;
        float searchInset = 9.25f;
        float searchRadius = 6.0f;
        float rowHeight = 28.0f;
        float platterInset = 10.0f;
        float platterRadius = 5.0f;
        float iconSize = 20.0f;
        float iconRadius = 4.5f;
        // Five native @2x System Settings captures agree: plates from x 16, labels from x 41.
        float iconX = 16.0f;
        float labelX = 41.0f;
        float searchTop = 1.0f;
        float searchSymbolX = 5.35f;
        float searchTextX = 27.0f;
        // With text the field ends in its clear button, centered 14 pt from its edge and 4 pt after the text, which
        // otherwise ends 8 pt in (search results @2x, dark); a result that wraps keeps 4 pt over and under its lines.
        float searchClearCenter = 14.0f;
        float searchClearSize = 16.0f;
        float searchClearGap = 4.0f;
        float searchTextTrailing = 8.0f;
        float wrappedRowPadding = 4.0f;
        // The search field stays put; the list scrolls from 9 pt under it, where a line marks rows under the field
        // (Privacy & Security @2x). The account is the list's first row: the avatar 2 pt under the list's top edge in a
        // 44 pt block, the first group 13 pt under it (Family row: user's screenshot).
        float searchBottom = 9.0f;
        // In a sheet the list starts 10 pt from the top (Wi-Fi details @2x).
        float sheetTop = 10.0f;
        // Selected, the account's platter reaches 2 pt over its block (Apple Account @2x: 46 pt).
        float accountTop = 2.0f;
        float accountPlatterRise = 2.0f;
        float accountTextX = 62.0f;
        // The name and its subtitle stand 1 pt apart; they and the avatar are centered 1 pt above the row's middle
        // (Battery, Bluetooth, Displays and Apple Account @2x).
        float accountLineSpacing = 1.0f;
        float accountRaise = 1.0f;
        float groupGap = 13.0f;
        // A group's header (Finder @2x): Semibold 11 from x 16 on a baseline 14.25 pt down a 19.25 pt row; the first
        // group's header stands right under the toolbar.
        float headerHeight = 19.25f;
        float headerBaseline = 14.25f;
        float headerX = 16.0f;
        float headerSize = 11.0f;
        float profileHeight = 44.0f;
        float avatar = 38.0f;
        // Icons fade as a whole out of the key window: gray plates of inactive System Settings @2x are exactly half way
        // to the sidebar material.
        float inactiveIconOpacity = 0.5f;
        // A row's badge of increased prominence (Software Update Available @2x): an 18 pt red disc 16.5 pt from the row's
        // end, the count in Medium 11; of standard prominence (Notes, Reminders @2x) the count in 13 pt tertiary text ends
        // 18 pt from it.
        float badge = 18.0f;
        float badgeTrailing = 16.5f;
        float badgeFontSize = 11.0f;
        float countTrailing = 18.0f;
        // A row with a line under its title (kit's Sidebar List): 42 pt, the title 6 pt down and the 11 pt line from 22; a
        // plate or picture then grows to 25.6 pt centered 29 pt in, and the text starts at 51.5.
        float subtitleRowHeight = 42.0f;
        float subtitleTitleTop = 6.0f;
        float subtitleTop = 22.0f;
        float largeIcon = 25.6f;
        float largeIconCenter = 29.0f;
        float largeLabelX = 51.5f;
        // An outline (Photos @2x): every row after an 8.5 pt column for the chevrons, 13 pt deeper per level; a group's
        // chevron.right Bold 9 centered 17.75 pt into its level turns down while the group is open.
        float outlineColumn = 8.5f;
        float outlineIndent = 13.0f;
        float outlineChevronX = 17.75f;
        float outlineChevronSize = 9.0f;
    };

    // Split views (Xcode @2x): a 1 pt divider the pointer takes within 3 pt of it; an inspector column 270 pt wide by
    // default (SwiftUI's inspectorColumnWidth).
    struct SplitViewMetrics {
        float divider = 1.0f;
        float grip = 3.0f;
        float inspectorWidth = 270.0f;
    };

    // ContentUnavailableView (Passwords @2x, macOS 15): a 34 pt symbol, a Bold 16 title, the description in 12/15 and the
    // actions, in a column kept 12 pt from the edges.
    struct ContentUnavailableMetrics {
        float padding = 12.0f;
        float symbolSize = 34.0f;
        float symbolSpacing = 23.5f;
        float titleSize = 16.0f;
        float descriptionSize = 12.0f;
        float descriptionLineHeight = 15.0f;
        float descriptionSpacing = 10.0f;
        float actionsSpacing = 10.5f;
    };

    struct WindowMetrics {
        float radius = 10.0f;
        float toolbarHeight = 52.0f;
        float compactToolbarHeight = 38.0f;
        // Traffic lights in a 28 pt title bar (Views/Windows/Window Controls/With Title or Tab Bar). A title that would
        // cross them when centered starts 7 pt after them (Get Info @2x).
        float titleBarHeight = 28.0f;
        float compactLightInset = 8.0f;
        float titleAfterLights = 7.0f;
        // The band along a resizable edge that takes the pointer.
        float resizeGrip = 4.0f;
        float trafficLight = 12.0f;
        float trafficPitch = 20.0f;
        // Measured on System Settings @2x: back and forward are adjacent 34 pt symbol slots after the 12 pt margin,
        // and the title follows 8 pt later.
        float toolbarMargin = 12.0f;
        float toolbarItemSpacing = 8.0f;
        float symbolButtonWidth = 34.0f;
        float symbolButtonSlot = 38.0f;
        float symbolCenterY = 18.25f;
        // Without history buttons the title starts 20 pt after the toolbar margin.
        float titleInset = 20.0f;
        // The traffic lights of a toolbar window end 72 pt from its edge; a hidden sidebar's toggle follows them 8 pt
        // later, and the sidebar slides in 0.25 s.
        float lightsEnd = 72.0f;
        float sidebarSlide = 0.25f;
        float titleSize = 15.0f;
        // The title line is centered 25.5 pt below the window top (Bluetooth @2x), half a point above the bar middle.
        float titleCenter = 25.5f;
        // With a subtitle (Battery @2x): a 13 pt title on a baseline 24 pt down, an 11 pt subtitle on one 38 pt down,
        // its symbol 3.5 pt before the text.
        float subtitledTitleSize = 13.0f;
        float subtitledTitleBaseline = 24.0f;
        float subtitleSize = 11.0f;
        float subtitleBaseline = 38.0f;
        float subtitleSymbolSpacing = 3.5f;
        // Between the parts of a subtitle (AirPods @2x: the buds' charge, then the case's).
        float subtitlePartSpacing = 8.0f;
        // A window without a sidebar (Activity Monitor @2x): the title 91 pt from the window's edge, a 0.5 pt hairline
        // under the toolbar.
        float plainTitleX = 91.0f;
        float toolbarLine = 0.5f;
        // Its items (Activity Monitor @2x): the titles take 5.25 pt past their text, symbol buttons are 30.5 pt wide and
        // controls 28 pt tall, 8 pt apart. A menu is 50 pt wide with its symbol centered 18 pt in and a 9 pt Bold
        // chevron.down 38 pt in, both half a point lower than a button's symbol.
        float titleTrailing = 5.25f;
        float toolbarButtonWidth = 30.5f;
        float toolbarControlHeight = 28.0f;
        float menuWidth = 50.0f;
        float menuSymbolX = 18.0f;
        float menuChevronX = 38.0f;
        float menuChevronSize = 9.0f;
        float menuChevronFrame = 16.0f;
        float menuOffsetY = 0.5f;
        // Titled toolbar items (kit's Bars/Toolbar): a button's title 6 pt in from both ends; a pull-down's title 8 pt in
        // and its 9 pt chevron frame 5 pt after the title and 5 pt from the end; a pop-up's two chevron.up / .down Heavy
        // 7.5, 7 pt apart, 10 pt after the title and 6 pt from the end.
        float toolbarTitlePadding = 6.0f;
        float toolbarMenuTitleX = 8.0f;
        float toolbarChevronFrame = 9.0f;
        float toolbarMenuChevronGap = 5.0f;
        float toolbarMenuTrailing = 5.0f;
        float toolbarPopUpChevronGap = 10.0f;
        float toolbarPopUpTrailing = 6.0f;
        // A toolbar item lights a plate 5 pt short of its slot above and below, with 6 pt corners; its symbol is Medium
        // 13 at the large scale in a 20 x 16 pt frame.
        float toolbarPlateInset = 5.0f;
        float toolbarPlateRadius = 6.0f;
        float toolbarSymbolSize = 13.0f;
        ImVec2 toolbarSymbolFrame = ImVec2(20.0f, 16.0f);
        // Their frames meet at the item's middle.
        ChevronPair toolbarPopUpChevrons = ChevronPair{7.5f, ImVec2(9.0f, 7.0f), -7.0f, 0.0f};
    };

    // Menus (File Sharing and Battery @2x): a 0.5 pt border inside the frame, then 5 pt of padding around the items;
    // titles start 9.5 pt into an item (15 pt from the frame's edge), after a 9.5 pt checkmark column when an item is
    // checked, and end at least 15 pt before the other edge of a frame a whole number of points wide.
    struct MenuMetrics {
        float border = 0.5f;
        float padding = 5.0f;
        float radius = 8.0f;
        float itemHeight = 22.0f;
        // A context menu from System Settings (File Sharing @2x) sets its items in 14 pt, 23 pt apart and half a point
        // below the middle; Finder's stay 22.
        float contextFontSize = 14.0f;
        float contextItemHeight = 23.0f;
        float contextLabelDrop = 0.5f;
        float itemRadius = 5.0f;
        float labelInset = 9.5f;
        float checkmarkColumn = 9.5f;
        float trailingInset = 15.0f;
        // Separators: an 11 pt row with a 1 pt line 5 pt down, 14.5 pt in from both edges (Notes' File menu @2x).
        float separatorHeight = 11.0f;
        float separatorLine = 5.0f;
        float separatorInset = 14.5f;
        float minWidth = 60.0f;
        float pullDownGap = 2.0f;
        float imageSpacing = 5.0f;
        // Section headers: 11 pt Semibold on the items' baseline (the kit, Safari's Move & Resize).
        float headerFontSize = 11.0f;
        // Symbols before titles: a column as wide as the widest symbol, at least the kit's 16 pt image, titles 6 pt after it.
        float symbolColumn = 16.0f;
        float symbolSpacing = 6.0f;
        // Shortcut keys are centered in a column as wide as the menu's widest key, 14 pt from the trailing edge; each
        // modifier is centered in its own cell 13.75 pt further left. The chevron of a submenu is centered 18.75 pt from the
        // edge. Titles keep shortcutGap and chevronGap from them (the Apple, Finder, Go and Notes File menus @2x).
        float shortcutPitch = 13.75f;
        float shortcutMargin = 14.0f;
        float shortcutGap = 23.0f;
        float chevronTrailing = 18.75f;
        float chevronGap = 20.0f;
        // A combo box's list (AppKit docs @2x): rows 18 pt with their titles 2 pt above the middle and 17 pt in (2 more than
        // a menu's), 0.5 pt under the bezel, reaching 6.75 pt before the control and 10.25 pt past it.
        float listItemHeight = 18.0f;
        float listLabelInset = 11.5f;
        float listLabelRaise = 2.0f;
        float listGap = 0.5f;
        float listExtraWidth = 17.0f;
        // An item's second line (Safari's File menu @2x, dark): 11 pt 14 pt under the title's baseline, the row as much taller.
        float subtitleSize = 11.0f;
        float subtitleLine = 14.0f;
        // An item's badge (NSMenuItemBadge @2x, both appearances): a 16 pt capsule ending 13 pt from the trailing edge, its
        // text Semibold 10 with 8 pt either side; the badge column keeps 28 pt from the widest title.
        float badgeHeight = 16.0f;
        float badgePadding = 8.0f;
        float badgeTrailing = 13.0f;
        float badgeFontSize = 10.0f;
        float badgeGap = 28.0f;
        // A menu taller than the screen scrolls under an item-high band with arrowtriangle.up.fill or .down.fill Regular 11
        // at either end (kit's Overflow Arrow), 12 items a second while the pointer rests on a band.
        float overflowArrowSize = 11.0f;
        float overflowSpeed = 12.0f;
        // The ring around an item while its context menu is open (Mail's sidebar @2x).
        float targetRing = 2.0f;
    };

    // The menu bar (512 Pixels' full-screen captures of macOS 15): 24 pt tall, titles 10 pt either side of their text from
    // x 10, the open title on a plate 5 pt wider than its slot, its menu 0.5 pt under the bar from the plate's edge.
    struct MenuBarMetrics {
        float height = 24.0f;
        float leading = 10.0f;
        float titlePadding = 10.0f;
        float baseline = 16.0f;
        float symbolSize = 15.0f;
        float plateOutset = 5.0f;
        float plateInset = 0.5f;
        float plateRadius = 3.0f;
        float menuInset = 0.5f;
        float menuGap = 0.5f;
        float statusTrailing = 19.5f;
    };

    SwitchMetrics Switch(ControlSize size);
    // In System Settings forms checkbox and radio labels start 5 pt after the control (the kit has 6).
    CheckboxMetrics Checkbox(ControlSize size, bool in_form);
    RadioMetrics Radio(ControlSize size, bool in_form);
    const FormPopUpMetrics& FormPopUp();
    // For the environment's control size.
    BezelPopUpMetrics BezelPopUp();
    const SegmentedMetrics& Segmented();
    // For the environment's control size.
    SliderMetrics Slider();
    const StepperMetrics& Stepper();
    const DatePickerMetrics& DatePicker();
    const LevelIndicatorMetrics& LevelIndicator();
    const PathControlMetrics& PathControl();
    const TextFieldMetrics& TextField();
    const ProgressMetrics& Progress();
    const DisclosureMetrics& Disclosure();
    const ColorWellMetrics& ColorWell();
    const ColorPanelMetrics& ColorPanel();
    const ImageWellMetrics& ImageWell(bool large = false);
    const PopoverMetrics& Popover();
    const AlertMetrics& Alert();
    const TooltipMetrics& Tooltip();
    const NotificationMetrics& Notification();
    const ControlGroupMetrics& ControlGroup();
    const TokenFieldMetrics& TokenField();
    const GroupBoxMetrics& GroupBox();
    const ScrollViewMetrics& ScrollView();
    const TabBarMetrics& TabBar();
    const SheetMetrics& Sheet();
    const FormMetrics& Form();
    const ChartMetrics& Chart();
    const GaugeMetrics& Gauge();
    const TableMetrics& Table();
    const BorderedListMetrics& BorderedList();
    PushButtonMetrics PushButton(ControlSize size = ControlSize::Regular);
    const HelpButtonMetrics& HelpButton();
    const AccessoryBarMetrics& AccessoryBar();
    const SidebarMetrics& Sidebar();
    const WindowMetrics& Window();
    const SplitViewMetrics& SplitView();
    const ContentUnavailableMetrics& ContentUnavailable();
    const MenuMetrics& Menu();
    const MenuBarMetrics& MenuBar();
} // namespace Cupertino::Metrics
