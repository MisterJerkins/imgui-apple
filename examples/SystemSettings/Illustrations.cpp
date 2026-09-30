#include "Settings.h"

#include "core/Metrics.h"

#include <cmath>

// Illustrations the panes share: the gesture videos of Mouse and Trackpad, and the MacBook Air of Displays and About.
namespace Examples::Settings {
    using namespace Cupertino;

    // A Dock tile's color at its top and bottom.
    struct DockTile {
        unsigned top;
        unsigned bottom;
    };

    void GestureHeader(Painter device, Painter video, int* tab, std::initializer_list<const char*> tabs) {
        HStack({.spacing = 11.0f}, [&] {
            Frame({.maxWidth = Infinity}, [&] { Canvas(ImVec2(225.0f, 142.0f), device); });
            Frame({.maxWidth = Infinity}, [&] { Canvas(ImVec2(225.0f, 142.0f), video); });
        });
        // The tabs stand 16 pt under the pictures and 18 pt over their settings, apart from the form's sections.
        Padding(EdgeInsets{6.0f, 0.0f, 8.0f, 0.0f}, [&] { Picker("##tab", tab, tabs, {.style = PickerStyle::Segmented}); });
    }

    // Colors are sRGB: the 512 Pixels captures carry a wide-gamut display profile.
    void PaintGestureDesktop(ImDrawList* draw, const ImRect& rect) {
        const CornerRadii radii(Px(Metrics::Form().sectionRadius));
        const DrawingSpace space = PointSpace(rect);
        Draw::FillVerticalGradient(draw, rect, radii, Rgba::Hex(0x3892D5), Rgba::Hex(0x6EBAF4), CornerStyle::Circular);
        Draw::FillRoundedRect(draw, ImRect(rect.Min, ImVec2(rect.Max.x, rect.Min.y + Px(5.75f))), CornerRadii(radii.topLeft, radii.topRight, 0.0f, 0.0f), Rgba::Hex(0x2C79B1), CornerStyle::Circular);

        // The Dock: a translucent shelf of app tiles with a divider before the last three.
        Draw::FillRoundedRect(draw, space.Box(45.5f, 125.0f, 178.5f, 139.5f), CornerRadii(Px(4.0f)), Rgba::White(0.4f));
        static const DockTile tiles[] = {
            {0x30B2FC, 0x2886FE}, {0xFCC700, 0xFAA700}, {0xF8678C, 0xF8495E}, {0x34D168, 0x30BD3C}, {0xFCD800, 0xFCCA00}, {0xFFFFFF, 0xF4F4F4},
            {0x6CD9FC, 0x5CC3F4}, {0xA396FF, 0x6F70E3}, {0x898D8B, 0x5F5F5F}, {0x34D164, 0x30BD40}, {0xC8C7C8, 0xA2A3A2},
        };
        float x = 47.75f;
        for (int i = 0; i < IM_ARRAYSIZE(tiles); ++i) {
            if (i == 8) {
                Draw::FillRect(draw, space.Box(x + 0.5f, 127.5f, x + 1.0f, 137.0f), Rgba::White(0.6f));
                x += 3.0f;
            }
            Draw::FillVerticalGradient(draw, space.Box(x, 127.0f, x + 9.75f, 137.25f), CornerRadii(Px(2.5f)), Rgba::Hex(tiles[i].top), Rgba::Hex(tiles[i].bottom));
            x += 11.5f;
        }
        DrawSectionBorder(draw, rect);
    }

    void PaintFingertip(ImDrawList* draw, ImVec2 center) {
        Draw::FillCircle(draw, center, Px(6.5f), Rgba::Hex(0xB8CDF0));
        Draw::StrokeCircle(draw, center, Px(6.5f), Rgba::Hex(0x70A3FA), Px(1.0f));
    }

    void PaintPointer(ImDrawList* draw, ImVec2 tip) {
        static const ImVec2 shape[] = {{0.0f, 0.0f}, {0.0f, 4.25f}, {1.0f, 3.35f}, {1.7f, 4.8f}, {2.35f, 4.5f}, {1.7f, 3.05f}, {3.0f, 3.05f}};
        ImVec2 points[IM_ARRAYSIZE(shape)];
        for (int i = 0; i < IM_ARRAYSIZE(shape); ++i)
            points[i] = tip + Px(shape[i]);
        draw->AddPolyline(points, IM_ARRAYSIZE(points), Rgba::White(0.9f).Packed(), ImDrawFlags_Closed, Px(0.75f));
        draw->AddConcavePolyFilled(points, IM_ARRAYSIZE(points), Rgba::Black(0.9f).Packed());
    }

    // The MacBook Air of Displays @2x at 126 x 78 pt, scaled to rect: a black lid around the screen, the keyboard's edge
    // under it and the silver base over its shadow.
    void PaintMacBookAir(ImDrawList* draw, const ImRect& rect, Painter screen) {
        const float scale = rect.GetWidth() / Px(126.0f);
        const DrawingSpace space = PointSpace(rect, scale);
        const CornerRadii lid(space.Length(4.0f), space.Length(4.0f), 0.0f, 0.0f);
        Draw::DropShadow(draw, space.Box(3.0f, 72.0f, 123.0f, 74.5f), CornerRadii(space.Length(1.0f)), Shadow{Rgba::Black(0.35f), ImVec2(0.0f, scale), 2.5f * scale});
        Draw::FillRoundedRect(draw, space.Box(1.5f, 72.5f, 124.5f, 75.5f), CornerRadii(0.0f, 0.0f, space.Length(2.0f), space.Length(2.0f)), Rgba::Hex(0x5C5C5C));
        Draw::FillVerticalGradient(draw, space.Box(0.5f, 68.75f, 125.5f, 71.5f), CornerRadii(space.Length(1.0f), space.Length(1.0f), 0.0f, 0.0f), Rgba::Hex(0xE4E4E4), Rgba::Hex(0xFBFBFB));
        Draw::FillVerticalGradient(draw, space.Box(0.5f, 71.5f, 125.5f, 73.25f), CornerRadii(0.0f, 0.0f, space.Length(1.5f), space.Length(1.5f)), Rgba::Hex(0xF2F2F2), Rgba::Hex(0x9A9A9A));
        Draw::FillRect(draw, space.Box(12.5f, 67.5f, 113.5f, 69.0f), Rgba::Hex(0x1C1C1C));
        Draw::FillRoundedRect(draw, space.Box(13.0f, 0.5f, 113.0f, 67.5f), lid, Rgba::Hex(0x0A0A0A));
        Draw::StrokeRoundedRect(draw, space.Box(13.0f, 0.5f, 113.0f, 67.5f), lid, Rgba::Hex(0x2A2A2A), Px(0.5f));
        Draw::FillRect(draw, space.Box(57.0f, 65.75f, 69.0f, 66.75f), Rgba::Hex(0x6A6A6A));
        screen(draw, space.Box(16.75f, 4.25f, 109.25f, 62.25f));
    }
} // namespace Examples::Settings
