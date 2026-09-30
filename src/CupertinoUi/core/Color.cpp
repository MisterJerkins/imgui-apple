#include "Color.h"

#include <cmath>

namespace Cupertino {
    ImU32 Rgba::Packed() const {
        const float alpha = ImSaturate(a * ImGui::GetStyle().Alpha);
        return IM_COL32(int(ImSaturate(r) * 255.0f + 0.5f), int(ImSaturate(g) * 255.0f + 0.5f), int(ImSaturate(b) * 255.0f + 0.5f), int(alpha * 255.0f + 0.5f));
    }

    ImVec4 Rgba::Vec4() const {
        return ImVec4(r, g, b, a);
    }

    Rgba FromHsb(const Hsb& hsb, float alpha) {
        Rgba color(0.0f, 0.0f, 0.0f, alpha);
        ImGui::ColorConvertHSVtoRGB(hsb.hue, hsb.saturation, hsb.brightness, color.r, color.g, color.b);
        return color;
    }

    Hsb ToHsb(Rgba color) {
        Hsb hsb;
        ImGui::ColorConvertRGBtoHSV(color.r, color.g, color.b, hsb.hue, hsb.saturation, hsb.brightness);
        return hsb;
    }

    bool SameColor(Rgba a, Rgba b) {
        return ImFabs(a.r - b.r) < 0.004f && ImFabs(a.g - b.g) < 0.004f && ImFabs(a.b - b.b) < 0.004f;
    }

    namespace Blend {
        Rgba Over(Rgba top, Rgba bottom) {
            const float alpha = top.a + bottom.a * (1.0f - top.a);
            if (alpha <= 0.0f)
                return Rgba();
            const float weight = bottom.a * (1.0f - top.a);
            return Rgba((top.r * top.a + bottom.r * weight) / alpha, (top.g * top.a + bottom.g * weight) / alpha, (top.b * top.a + bottom.b * weight) / alpha, alpha);
        }

        // kCGBlendModePlusDarker on an opaque backdrop: max(D * (1 - a), D + S * a - a).
        Rgba PlusDarker(Rgba source, Rgba backdrop) {
            const float a = source.a;
            const auto channel = [a](float s, float d) {
                return ImMax(d * (1.0f - a), d + s * a - a);
            };
            return Rgba(channel(source.r, backdrop.r), channel(source.g, backdrop.g), channel(source.b, backdrop.b), 1.0f);
        }

        Rgba PlusLighter(Rgba source, Rgba backdrop) {
            const float a = source.a;
            return Rgba(ImMin(1.0f, backdrop.r + source.r * a), ImMin(1.0f, backdrop.g + source.g * a), ImMin(1.0f, backdrop.b + source.b * a), 1.0f);
        }

        Rgba Mix(Rgba from, Rgba to, float amount) {
            return Rgba(ImLerp(from.r, to.r, amount), ImLerp(from.g, to.g, amount), ImLerp(from.b, to.b, amount), ImLerp(from.a, to.a, amount));
        }

        Rgba Crossfade(Rgba from, Rgba to, float amount) {
            const float alpha = ImLerp(from.a, to.a, amount);
            if (alpha <= 0.0f)
                return Rgba(to.r, to.g, to.b, 0.0f);
            const auto channel = [&](float a, float b) { return ImLerp(a * from.a, b * to.a, amount) / alpha; };
            return Rgba(channel(from.r, to.r), channel(from.g, to.g), channel(from.b, to.b), alpha);
        }
    } // namespace Blend

    namespace ColorSpace {
        static float Linear(float value) {
            return value <= 0.04045f ? value / 12.92f : std::pow((value + 0.055f) / 1.055f, 2.4f);
        }

        static float Encoded(float value) {
            value = ImSaturate(value);
            return value <= 0.0031308f ? value * 12.92f : 1.055f * std::pow(value, 1.0f / 2.4f) - 0.055f;
        }

        // Linear channels through a 3 x 3 matrix, clipped to the target gamut.
        static Rgba Convert(Rgba color, const float (&m)[9]) {
            const float r = Linear(color.r), g = Linear(color.g), b = Linear(color.b);
            return Rgba(Encoded(m[0] * r + m[1] * g + m[2] * b), Encoded(m[3] * r + m[4] * g + m[5] * b), Encoded(m[6] * r + m[7] * g + m[8] * b), color.a);
        }

        Rgba DisplayP3ToSrgb(Rgba color) {
            static const float ToSrgb[9] = {1.2249401f, -0.2249404f, 0.0f, -0.0420569f, 1.0420571f, 0.0f, -0.0196376f, -0.0786361f, 1.0982735f};
            return Convert(color, ToSrgb);
        }

        Rgba SrgbToDisplayP3(Rgba color) {
            static const float ToP3[9] = {0.8224621f, 0.1775380f, 0.0f, 0.0331941f, 0.9668058f, 0.0f, 0.0170827f, 0.0723974f, 0.9105199f};
            return Convert(color, ToP3);
        }
    } // namespace ColorSpace
} // namespace Cupertino
