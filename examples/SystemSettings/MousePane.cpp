#include "Settings.h"

#include "core/Metrics.h"

#include <cmath>

namespace Examples::Settings {
    using namespace Cupertino;

    struct MouseModel {
        int tab = 0;
        float trackingSpeed = 3.0f / 9.0f;
        bool naturalScrolling = true;
        int secondaryClick = 2;
        bool smartZoom = false;
        int swipePages = 0;
        int swipeApps = 0;
        bool missionControl = true;
    };

    // The Magic Mouse as Point & Click draws it: its outline, a superellipse 45.75 x 87 pt with a 1.75 pt line, a
    // fingertip on the left button and the Apple logo, centered on a panel like a section's box (512 Pixels @2x).
    static void PaintMouse(ImDrawList* draw, const ImRect& rect) {
        DrawSectionBox(draw, rect);
        const ImVec2 center = rect.GetCenter() + Px(ImVec2(0.0f, 1.0f));
        const ImVec2 radius = Px(ImVec2(22.0f, 42.625f));
        const float exponent = 2.0f / 3.3f;
        ImVec2 outline[128];
        for (int i = 0; i < IM_ARRAYSIZE(outline); ++i) {
            const float angle = 2.0f * IM_PI * float(i) / float(IM_ARRAYSIZE(outline));
            const float c = std::cos(angle);
            const float s = std::sin(angle);
            outline[i] = center + ImVec2(radius.x * std::copysign(std::pow(std::fabs(c), exponent), c), radius.y * std::copysign(std::pow(std::fabs(s), exponent), s));
        }
        const Rgba ink = Rgba::Black(0.25f);
        Draw::Polyline(draw, outline, ink, Px(1.75f), true);
        PaintFingertip(draw, center + Px(ImVec2(-0.5f, -21.5f)));
        const ImVec2 logo = center + Px(ImVec2(0.0f, 23.5f));
        Typography::DrawSymbol(draw, Symbols::AppleLogo, Font::System(12.0f), ImRect(logo - Px(ImVec2(10.0f, 10.0f)), logo + Px(ImVec2(10.0f, 10.0f))), ink);
    }

    // The Mouse gesture video's first frame: a window paging through shapes on cards over the gesture desktop.
    static void PaintMousePreview(ImDrawList* draw, const ImRect& rect) {
        PaintGestureDesktop(draw, rect);
        const DrawingSpace space = PointSpace(rect);
        const ImRect window = space.Box(31.25f, 12.25f, 194.25f, 116.0f);
        Draw::FillRoundedRect(draw, window, CornerRadii(Px(2.0f)), Rgba::Hex(0xE1F3FD));
        Draw::FillRoundedRect(draw, space.Box(31.25f, 12.25f, 194.25f, 17.75f), CornerRadii(Px(2.0f), Px(2.0f), 0.0f, 0.0f), Rgba::White(1.0f));
        draw->PushClipRect(window.Min, window.Max, true);
        for (const float left : {-10.5f, 77.0f, 164.5f})
            Draw::FillRoundedRect(draw, space.Box(left, 27.25f, left + 74.5f, 101.5f), CornerRadii(Px(4.0f)), Rgba::White(1.0f));
        Draw::FillVerticalGradient(draw, space.Box(3.25f, 40.25f, 51.25f, 88.25f), CornerRadii(Px(24.0f)), Rgba::Hex(0xA798FF), Rgba::Hex(0x6166D9), CornerStyle::Circular);
        Draw::FillVerticalGradient(draw, space.Box(92.5f, 42.75f, 136.0f, 86.0f), CornerRadii(Px(5.0f)), Rgba::Hex(0x70D7FD), Rgba::Hex(0x55BFF3));
        Draw::FillRoundedTriangle(draw, {space.At(168.9f, 87.25f), space.At(197.1f, 40.25f), space.At(225.3f, 87.25f)}, {Px(7.0f), Px(7.0f), Px(7.0f)}, Rgba::Hex(0xF9806B), Rgba::Hex(0xF85735));
        draw->PopClipRect();
        PaintPointer(draw, space.At(125.25f, 92.5f));
    }

    static void PointAndClick(MouseModel& model) {
        Section([&] {
            Slider("Tracking speed", &model.trackingSpeed, 0.0f, 1.0f, {.width = 241.0f, .ticks = 10, .snapsToTicks = true, .captions = {"Slow", "Fast"}});
            Toggle("Natural scrolling", &model.naturalScrolling, {.description = "Content tracks finger movement"});
            Picker("Secondary click", &model.secondaryClick, {"Click on Right Side", "Click on Left Side", "Off"});
            Toggle("Smart zoom", &model.smartZoom, {.description = "Double-tap with One Finger"});
        });
    }

    static void MoreGestures(MouseModel& model) {
        Section([&] {
            Picker("Swipe between pages", &model.swipePages, {"Scroll Left or Right with One Finger", "Swipe Left or Right with Two Fingers", "Swipe with One or Two Fingers", "Off"});
            Picker("Swipe between full-screen applications", &model.swipeApps, {"Swipe Left or Right with Two Fingers", "Off"});
            Toggle("Mission Control", &model.missionControl, {.description = "Double-tap with two fingers"});
        });
    }

    // Mouse as 512 Pixels captured it (a Magic Mouse at 57%): the device and its gesture video over the tabs, the tab's
    // settings, and the buttons for advanced settings and pairing.
    void MousePane() {
        static MouseModel model;
        Form([] {
            GestureHeader(PaintMouse, PaintMousePreview, &model.tab, {"Point & Click", "More Gestures"});
            if (model.tab == 0)
                PointAndClick(model);
            else
                MoreGestures(model);
            // The buttons stand 10 pt apart.
            TrailingButtons({.spacing = 10.0f}, [] {
                Button("Advanced\xE2\x80\xA6");
                Button("Set Up Bluetooth Mouse\xE2\x80\xA6");
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
