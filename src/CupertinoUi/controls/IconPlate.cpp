#include "IconPlate.h"

#include "core/Color.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Theme.h"
#include "core/Typography.h"

namespace Cupertino {
    struct PlateGradient {
        Rgba top;
        Rgba bottom;
    };

    // Measured on native @2x System Settings sidebars (Bluetooth, Privacy & Security): the system color at the bottom
    // and a lighter top. Orange, yellow, white and dark gray still come from the kit.
    static PlateGradient GradientOf(IconPlate plate) {
        switch (plate) {
            case IconPlate::Black:
                return {Rgba::Hex(0x2B2A2A), Rgba::Hex(0x010101)};
            case IconPlate::Blue:
                return {Rgba::Hex(0x3296FF), Rgba::Hex(0x007AFF)};
            case IconPlate::Cyan:
                return {Rgba::Hex(0x8BE1FC), Rgba::Hex(0x55BEF0)};
            case IconPlate::DarkGray:
                return {Rgba::Hex(0x818181), Rgba::Hex(0x4A4A4A)};
            case IconPlate::Gray:
                return {Rgba::Hex(0xB4B3B4), Rgba::Hex(0x8E8E93)};
            case IconPlate::Green:
                return {Rgba::Hex(0x53DD67), Rgba::Hex(0x28CD41)};
            case IconPlate::Orange:
                return {Rgba::Hex(0xFFC700), Rgba::Hex(0xFF9500)};
            case IconPlate::Pink:
                return {Rgba::Hex(0xFF6284), Rgba::Hex(0xFF2D55)};
            case IconPlate::Purple:
                return {Rgba::Hex(0x8180EB), Rgba::Hex(0x5856D6)};
            case IconPlate::Red:
                return {Rgba::Hex(0xFF6D65), Rgba::Hex(0xFF3B30)};
            case IconPlate::White:
                return {Rgba::Hex(0xFFFFFF), Rgba::Hex(0xF0F0F0)};
            case IconPlate::Yellow:
                return {Rgba::Hex(0xFBE300), Rgba::Hex(0xF5C200)};
            case IconPlate::None:
                break;
        }
        return {Rgba(), Rgba()};
    }

    void DrawIcon(ImDrawList* draw, const ImRect& frame, const Icon& icon) {
        if (!icon.image.IsEmpty()) {
            Draw::Image(draw, frame, icon.image);
            return;
        }
        // The plate and the symbol keep their square in the middle of a frame of another shape.
        const float side = ImMin(frame.GetWidth(), frame.GetHeight());
        const ImRect rect(frame.GetCenter() - ImVec2(side, side) * 0.5f, frame.GetCenter() + ImVec2(side, side) * 0.5f);
        // Master: 40 pt plate with radius 9 and a Regular 22 symbol in a 26 pt frame.
        const float scale = rect.GetWidth() / Px(40.0f);
        const float points = Pt(rect.GetWidth());
        if (icon.plate != IconPlate::None) {
            // The color brightens from 70% of the height to the top and holds below.
            const PlateGradient gradient = GradientOf(icon.plate);
            const float radius = Px(9.0f) * scale;
            const ImRect upper(rect.Min, ImVec2(rect.Max.x, rect.Min.y + rect.GetHeight() * 0.7f));
            Draw::DropShadows(draw, rect, CornerRadii(radius), Theme::IconPlateShadows());
            Draw::FillRoundedRect(draw, rect, CornerRadii(radius), gradient.bottom);
            Draw::FillVerticalGradient(draw, upper, CornerRadii(radius, radius, 0.0f, 0.0f), gradient.top, gradient.bottom);
        }
        const Rgba color = icon.color.a > 0.0f ? icon.color : icon.plate == IconPlate::White || icon.plate == IconPlate::None ? Theme::Colors().accent : Rgba::White(1.0f);
        const ImVec2 center = rect.GetCenter();
        const float size = Px(26.0f) * scale;
        const ImRect symbol_frame(center - ImVec2(size, size) * 0.5f, center + ImVec2(size, size) * 0.5f);
        if (icon.paint)
            icon.paint(draw, symbol_frame, color);
        else if (icon.plate == IconPlate::None && icon.symbol)
            // Without a plate the symbol is a sidebar item's: 13 pt at the medium scale in a 20 pt icon (the kit's Item
            // with Icon, Finder @2x).
            Typography::DrawSymbol(draw, icon.symbol, Font::System(13.0f * points / 20.0f * icon.symbolSize).AsPicture(), symbol_frame, color);
        else if (icon.symbol)
            Typography::DrawSymbol(draw, icon.symbol, Font::System(22.0f * points / 40.0f * icon.symbolSize).ImageScale(icon.scale).AsPicture(), symbol_frame, color);
    }

    void DrawAvatar(ImDrawList* draw, const ImRect& rect, const char* initials, const Bitmap& picture) {
        const float radius = ImMin(rect.GetWidth(), rect.GetHeight()) * 0.5f;
        if (!picture.IsEmpty()) {
            Draw::Image(draw, rect, picture, radius);
            return;
        }
        Draw::FillVerticalGradient(draw, rect, CornerRadii(radius), Rgba::Hex(0x9FD9F5), Rgba::Hex(0x52B5EA), CornerStyle::Circular);
        Typography::Draw(draw, Font::System(Pt(rect.GetHeight()) * 17.0f / 38.0f, FontWeight::Medium), rect, Rgba::White(1.0f), initials, TextAlignment::Center);
    }

    void DrawFolder(ImDrawList* draw, const ImRect& rect) {
        const auto at = [&](float x, float y) { return rect.Min + ImVec2(x / 14.0f * rect.GetWidth(), y / 13.0f * rect.GetHeight()); };
        const float unit = rect.GetWidth() / 14.0f;
        Draw::FillRoundedRect(draw, ImRect(at(0.0f, 0.0f), at(6.0f, 3.0f)), CornerRadii(unit, unit, 0.0f, 0.0f), Rgba::Hex(0x4AA2E7));
        Draw::FillRoundedRect(draw, ImRect(at(0.0f, 1.5f), at(14.0f, 6.0f)), CornerRadii(unit), Rgba::Hex(0x1891D0));
        Draw::FillVerticalGradient(draw, ImRect(at(0.0f, 3.0f), at(14.0f, 13.0f)), CornerRadii(unit), Rgba::Hex(0x7CD4F9), Rgba::Hex(0x55BEF0));
    }

    void PaintFolderIcon(ImDrawList* draw, const ImRect& frame, Rgba) {
        const float unit = frame.GetWidth() * 40.0f / 26.0f / 16.0f;
        const ImVec2 middle = frame.GetCenter() + ImVec2(unit, 0.0f);
        DrawFolder(draw, ImRect(middle - ImVec2(6.5f, 5.5f) * unit, middle + ImVec2(6.5f, 5.5f) * unit));
    }
} // namespace Cupertino
