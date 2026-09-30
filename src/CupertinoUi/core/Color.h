#pragma once

#include "imgui.h"
#include "imgui_internal.h"

namespace Cupertino {
    // Straight (non-premultiplied) sRGB color, the space AppKit, UIKit and ImGui blend in.
    struct Rgba {
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        float a = 0.0f;

        constexpr Rgba() = default;

        constexpr Rgba(float red, float green, float blue, float alpha = 1.0f) : r(red), g(green), b(blue), a(alpha) {
        }

        static constexpr Rgba Hex(unsigned int rgb, float alpha = 1.0f) {
            return Rgba(((rgb >> 16) & 0xFF) / 255.0f, ((rgb >> 8) & 0xFF) / 255.0f, (rgb & 0xFF) / 255.0f, alpha);
        }

        static constexpr Rgba Black(float alpha) {
            return Rgba(0.0f, 0.0f, 0.0f, alpha);
        }

        static constexpr Rgba White(float alpha) {
            return Rgba(1.0f, 1.0f, 1.0f, alpha);
        }

        constexpr Rgba Opacity(float factor) const {
            return Rgba(r, g, b, a * factor);
        }

        constexpr Rgba WithAlpha(float alpha) const {
            return Rgba(r, g, b, alpha);
        }

        // Packs for ImDrawList; the ImGui style alpha (used for fades) is applied here.
        ImU32 Packed() const;
        ImVec4 Vec4() const;
    };

    // NSColor's hue (0 ... 1 around the wheel), saturation and brightness.
    struct Hsb {
        float hue = 0.0f;
        float saturation = 0.0f;
        float brightness = 0.0f;
    };
    Rgba FromHsb(const Hsb& hsb, float alpha = 1.0f);
    Hsb ToHsb(Rgba color);

    // Whether two colors look alike, alpha aside: every channel within a 255th.
    bool SameColor(Rgba a, Rgba b);

    // Solid results of Apple blend modes over an opaque backdrop, used to replace vibrancy without shaders.
    namespace Blend {
        Rgba Over(Rgba top, Rgba bottom);
        Rgba PlusDarker(Rgba source, Rgba backdrop);
        Rgba PlusLighter(Rgba source, Rgba backdrop);
        Rgba Mix(Rgba from, Rgba to, float amount);
        // A color changing into another, as a layer fading out over one fading in: premultiplied, so a translucent color
        // (an unfocused selection) does not darken the way to an opaque one.
        Rgba Crossfade(Rgba from, Rgba to, float amount);
    } // namespace Blend

    // Display P3 and sRGB share the sRGB transfer curve. macOS draws the color wheel in P3, and an sRGB screen shows it
    // clipped: in the Colors window cyan is full at 60% of the radius, red at 80%.
    namespace ColorSpace {
        Rgba DisplayP3ToSrgb(Rgba color);
        Rgba SrgbToDisplayP3(Rgba color);
    } // namespace ColorSpace
} // namespace Cupertino
