#include "Settings.h"

#include "core/Metrics.h"

namespace Examples::Settings {
    using namespace Cupertino;

    struct TrackpadModel {
        int tab = 0;
        float trackingSpeed = 3.0f / 9.0f;
        float click = 0.5f;
        bool quietClick = false;
        bool forceClick = true;
        int lookUp = 0;
        int secondaryClick = 0;
        bool tapToClick = false;
        bool naturalScrolling = true;
        bool zoom = true;
        bool smartZoom = true;
        bool rotate = true;
        int swipePages = 0;
        int swipeApps = 0;
        bool notificationCenter = true;
        int missionControl = 0;
        int appExpose = 0;
        bool launchpad = true;
        bool showDesktop = true;
    };

    // The trackpad as the Point & Click illustration draws it: a rounded outline with two fingertips on it, centered on
    // a panel like a section's box (512 Pixels @2x).
    static void PaintTrackpad(ImDrawList* draw, const ImRect& rect) {
        DrawSectionBox(draw, rect);
        const ImVec2 center = rect.GetCenter();
        const ImVec2 half = Px(ImVec2(48.5f, 36.75f));
        const ImRect pad(center - half, center + half);
        Draw::StrokeRoundedRect(draw, pad, CornerRadii(Px(12.0f)), Rgba::Black(0.25f), Px(2.0f), StrokeAlignment::Inside);
        for (const float x : {-8.25f, 8.0f})
            PaintFingertip(draw, center + Px(ImVec2(x, -12.0f)));
    }

    // The Trackpad gesture video's first frame: a document window with shapes, a note and columns of text over the
    // gesture desktop.
    static void PaintGesturePreview(ImDrawList* draw, const ImRect& rect) {
        PaintGestureDesktop(draw, rect);
        const DrawingSpace space = PointSpace(rect);
        Draw::FillRoundedRect(draw, space.Box(31.25f, 13.25f, 193.75f, 117.0f), CornerRadii(Px(2.0f)), Rgba::Hex(0xE1F2FC));
        Draw::FillRoundedRect(draw, space.Box(31.25f, 13.25f, 193.75f, 18.5f), CornerRadii(Px(2.0f), Px(2.0f), 0.0f, 0.0f), Rgba::White(1.0f));
        Draw::FillRect(draw, space.Box(31.25f, 18.5f, 193.75f, 24.0f), Rgba::Hex(0xE0F4FD));
        Draw::FillRect(draw, space.Box(62.25f, 24.0f, 164.0f, 78.5f), Rgba::Hex(0xF9FFFF));

        Draw::FillVerticalGradient(draw, space.Box(100.75f, 29.25f, 130.75f, 59.25f), CornerRadii(Px(2.5f)), Rgba::Hex(0x34D165), Rgba::Hex(0x2CBB37));
        Draw::FillVerticalGradient(draw, space.Box(80.75f, 40.25f, 114.75f, 74.25f), CornerRadii(Px(17.0f)), Rgba::Hex(0x34B4FF), Rgba::Hex(0x267CFD), CornerStyle::Circular);
        Draw::FillRoundedTriangle(draw, {space.At(112.0f, 72.1f), space.At(131.75f, 40.2f), space.At(151.5f, 72.1f)}, {Px(3.0f), Px(3.0f), Px(3.0f)}, Rgba::Hex(0xFBC303), Rgba::Hex(0xFBA000));
        Draw::FillRoundedRect(draw, space.Box(83.75f, 82.9f, 141.0f, 85.9f), CornerRadii(Px(1.5f)), Rgba::Hex(0xB6C8D0));

        // The note: lines of text on a warm card with a soft shadow.
        const ImRect note = space.Box(136.1f, 49.4f, 160.6f, 85.8f);
        Draw::DropShadow(draw, note, CornerRadii(Px(2.0f)), Shadow{Rgba::Black(0.12f), ImVec2(0.0f, 0.5f), 2.5f});
        Draw::FillVerticalGradient(draw, note, CornerRadii(Px(2.0f)), Rgba::Hex(0xFDF3D3), Rgba::Hex(0xF8F6EE));
        static const float note_lines[] = {158.3f, 150.8f, 158.5f, 153.1f, 146.7f, 153.1f, 158.5f, 157.3f};
        for (int line = 0; line < IM_ARRAYSIZE(note_lines); ++line) {
            const float y = 51.9f + 4.28f * float(line);
            Draw::FillRoundedRect(draw, space.Box(138.3f, y, note_lines[line], y + 1.25f), CornerRadii(Px(0.6f)), Rgba::Hex(line < 4 ? 0xD2C9AC : 0xCCCBC4));
        }

        // Three columns of ten lines, a few of them short.
        struct Column {
            float left;
            float widths[10];
        };
        static const Column columns[] = {
            {62.25f, {28.75f, 28.75f, 28.75f, 28.75f, 28.75f, 28.75f, 22.45f, 28.75f, 28.75f, 28.75f}},
            {98.75f, {28.75f, 28.75f, 8.25f, 28.75f, 28.75f, 28.75f, 28.75f, 22.25f, 28.75f, 28.75f}},
            {135.0f, {29.0f, 26.8f, 29.0f, 29.0f, 29.0f, 29.0f, 29.0f, 29.0f, 29.0f, 13.8f}},
        };
        for (const Column& column : columns) {
            for (int line = 0; line < 10; ++line) {
                const float y = 90.25f + 2.15f * float(line);
                Draw::FillRect(draw, space.Box(column.left, y, column.left + column.widths[line], y + 1.0f), Rgba::Hex(0xB7C8D0));
            }
        }
        PaintPointer(draw, space.At(130.75f, 49.5f));
    }

    static void PointAndClick(TrackpadModel& model) {
        Section([&] {
            Slider("Tracking speed", &model.trackingSpeed, 0.0f, 1.0f, {.width = 241.0f, .ticks = 10, .snapsToTicks = true, .captions = {"Slow", "Fast"}});
            Slider("Click", &model.click, 0.0f, 1.0f, {.width = 241.0f, .ticks = 3, .snapsToTicks = true, .captions = {"Light", "Medium", "Firm"}});
            Toggle("Quiet Click", &model.quietClick);
            Toggle("Force Click and haptic feedback", &model.forceClick, {.description = "Click then press firmly for Quick Look, Look up, and variable speed media controls."});
            Picker("Look up & data detectors", &model.lookUp, {"Force Click with One Finger", "Tap with Three Fingers", "Off"});
            Picker("Secondary click", &model.secondaryClick, {"Click with Two Fingers", "Click in Bottom Right Corner", "Click in Bottom Left Corner", "Off"});
            Toggle("Tap to click", &model.tapToClick, {.description = "Tap with one finger"});
        });
    }

    static void ScrollAndZoom(TrackpadModel& model) {
        Section([&] {
            Toggle("Natural scrolling", &model.naturalScrolling, {.description = "Content tracks finger movement"});
            Toggle("Zoom in or out", &model.zoom, {.description = "Pinch with two fingers"});
            Toggle("Smart zoom", &model.smartZoom, {.description = "Double-tap with two fingers"});
            Toggle("Rotate", &model.rotate, {.description = "Rotate with two fingers"});
        });
    }

    static void MoreGestures(TrackpadModel& model) {
        Section([&] {
            Picker("Swipe between pages", &model.swipePages, {"Scroll Left or Right with Two Fingers", "Swipe with Three Fingers", "Swipe with Two or Three Fingers", "Off"});
            Picker("Swipe between full-screen applications", &model.swipeApps, {"Swipe Left or Right with Three Fingers", "Swipe Left or Right with Four Fingers", "Off"});
            Toggle("Notification Center", &model.notificationCenter, {.description = "Swipe left from the right edge with two fingers"});
            Picker("Mission Control", &model.missionControl, {"Swipe Up with Three Fingers", "Swipe Up with Four Fingers", "Off"});
            Picker("App Expos\xC3\xA9", &model.appExpose, {"Off", "Swipe Down with Three Fingers", "Swipe Down with Four Fingers"});
            Toggle("Launchpad", &model.launchpad, {.description = "Pinch with thumb and three fingers"});
            Toggle("Show Desktop", &model.showDesktop, {.description = "Spread with thumb and three fingers"});
        });
    }

    // Trackpad as 512 Pixels captured it: the gesture illustration and its video over the tabs, the tab's settings,
    // and the Bluetooth trackpad button.
    void TrackpadPane() {
        static TrackpadModel model;
        Form([] {
            GestureHeader(PaintTrackpad, PaintGesturePreview, &model.tab, {"Point & Click", "Scroll & Zoom", "More Gestures"});
            if (model.tab == 0)
                PointAndClick(model);
            else if (model.tab == 1)
                ScrollAndZoom(model);
            else
                MoreGestures(model);
            TrailingButtons([] {
                Button("Set Up Bluetooth Trackpad\xE2\x80\xA6");
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
