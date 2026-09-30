#include "ColorPanel.h"

#include "Window.h"
#include "controls/Picker.h"
#include "controls/Slider.h"
#include "controls/TextField.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/State.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"
#include "layout/Stacks.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace Cupertino {
    enum class ColorPanelMode {
        Wheel,
        Sliders,
    };

    struct FieldText {
        std::string text;
        bool editing = false;
    };

    // The window's own values. The wheel works in Display P3 and the HSB sliders in sRGB, each keeping its hue while the
    // color is gray and its saturation while it is black; shown is the color they were last taken from.
    struct ColorPanelState {
        ColorPanelMode mode = ColorPanelMode::Wheel;
        int sliders = 0;
        float wheel[3] = {0.0f, 0.0f, 1.0f};
        float hsb[3] = {0.0f, 0.0f, 1.0f};
        std::optional<Rgba> shown;
        std::vector<std::optional<Rgba>> saved;
        FieldText fields[5];
    };

    static Rgba FromHsv(const float (&hsv)[3], float alpha) {
        return FromHsb({hsv[0], hsv[1], hsv[2]}, alpha);
    }

    static Rgba WheelColor(float hue, float saturation, float brightness) {
        const float hsv[3] = {hue, saturation, brightness};
        return ColorSpace::DisplayP3ToSrgb(FromHsv(hsv, 1.0f));
    }

    // Takes brightness, then saturation and hue from color where it has them: black keeps the hue and saturation, gray
    // the hue.
    static void Derive(float (&hsv)[3], Rgba color) {
        const Hsb hsb = ToHsb(color);
        hsv[2] = hsb.brightness;
        if (hsb.brightness <= 0.0f)
            return;
        hsv[1] = hsb.saturation;
        if (hsb.saturation > 0.0f)
            hsv[0] = hsb.hue;
    }

    // Takes the models from color, but not the one that has just set it.
    static void Follow(ColorPanelState& state, Rgba color, bool wheel, bool hsb) {
        if (wheel)
            Derive(state.wheel, ColorSpace::SrgbToDisplayP3(color));
        if (hsb)
            Derive(state.hsb, color);
        state.shown = color;
    }

    // The wheel as a mesh of rings, so its saturation clips outward as P3 on an sRGB screen: hue counterclockwise from red
    // at three o'clock, saturation from the middle out, all at one brightness. It goes over a solid disc for its edge.
    static void DrawWheel(ImDrawList* draw, ImVec2 center, float radius, float brightness) {
        constexpr int Rings = 40;
        constexpr int Segments = 180;
        constexpr int Columns = Segments + 1;
        Draw::FillCircle(draw, center, radius, WheelColor(0.0f, 0.0f, brightness * 0.8f));
        // The vertices' places on a unit wheel never change and their colors only with the brightness; the wheel and its
        // tile's icon each keep theirs.
        struct Colored {
            float brightness = -1.0f;
            ImVector<Rgba> colors;
        };
        static ImVector<ImVec2> places;
        static Colored cache[2];
        if (places.empty()) {
            for (int ring = 0; ring <= Rings; ++ring) {
                for (int segment = 0; segment < Columns; ++segment) {
                    const float angle = -2.0f * IM_PI * float(segment % Segments) / float(Segments);
                    places.push_back(ImVec2(std::cos(angle), std::sin(angle)) * (float(ring) / float(Rings)));
                }
            }
        }
        Colored& entry = cache[0].brightness == brightness ? cache[0] : cache[1].brightness == brightness ? cache[1] : cache[brightness == 1.0f ? 1 : 0];
        if (entry.brightness != brightness) {
            entry.colors.resize(0);
            for (int ring = 0; ring <= Rings; ++ring) {
                for (int segment = 0; segment < Columns; ++segment)
                    entry.colors.push_back(WheelColor(float(segment % Segments) / float(Segments), float(ring) / float(Rings), brightness));
            }
            entry.brightness = brightness;
        }
        const ImVector<Rgba>& colors = entry.colors;
        const ImVec2 uv = ImGui::GetFontTexUvWhitePixel();
        draw->PrimReserve(Rings * Segments * 6, (Rings + 1) * Columns);
        const unsigned base = draw->_VtxCurrentIdx;
        for (int i = 0; i < places.Size; ++i)
            draw->PrimWriteVtx(center + places[i] * radius, uv, colors[i].Packed());
        for (int ring = 0; ring < Rings; ++ring) {
            for (int segment = 0; segment < Segments; ++segment) {
                const ImDrawIdx a = ImDrawIdx(base + unsigned(ring * Columns + segment));
                const ImDrawIdx d = ImDrawIdx(a + Columns);
                draw->PrimWriteIdx(a);
                draw->PrimWriteIdx(ImDrawIdx(a + 1));
                draw->PrimWriteIdx(ImDrawIdx(d + 1));
                draw->PrimWriteIdx(a);
                draw->PrimWriteIdx(ImDrawIdx(d + 1));
                draw->PrimWriteIdx(d);
            }
        }
    }

    // A rounded track colored by position from its leading to its trailing end; gradients with more than two stops (hue)
    // add strips between the rounded ends.
    static void DrawGradientTrack(ImDrawList* draw, const ImRect& track, const std::function<Rgba(float)>& gradient, int stops) {
        const float radius = Px(Metrics::ColorPanel().trackRadius);
        const auto at = [&](float x) { return gradient(ImSaturate((x - track.Min.x) / track.GetWidth())); };
        const int first = draw->VtxBuffer.Size;
        Draw::FillRoundedRect(draw, track, CornerRadii(radius), Rgba::White(1.0f));
        Draw::ShadeVertices(draw, first, [&](ImVec2 point) { return at(point.x); });
        for (int i = 0; stops > 2 && i + 1 < stops; ++i) {
            const float x0 = ImLerp(track.Min.x + radius, track.Max.x - radius, float(i) / float(stops - 1));
            const float x1 = ImLerp(track.Min.x + radius, track.Max.x - radius, float(i + 1) / float(stops - 1));
            const ImU32 left = at(x0).Packed(), right = at(x1).Packed();
            draw->AddRectFilledMultiColor(ImVec2(x0, track.Min.y), ImVec2(x1, track.Max.y), left, right, right, left);
        }
        Draw::StrokeRoundedRect(draw, track, CornerRadii(radius), Theme::Colors().colorWellLine, Px(1.0f));
    }

    // The gradient track with a white bar knob at value (0 to 1 from the leading end), and a tick over its middle for the
    // brightness; dragging anywhere along it sets the value.
    static bool GradientSlider(ImGuiID id, const ImRect& track, float* value, const std::function<Rgba(float)>& gradient, int stops, bool tick) {
        const Metrics::ColorPanelMetrics& metrics = Metrics::ColorPanel();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const ImVec2 knob_size = Px(metrics.knob);
        const float travel_left = track.Min.x + knob_size.x * 0.5f;
        const float travel = track.GetWidth() - knob_size.x;
        const ImRect hit(track.Min.x, track.GetCenter().y - knob_size.y * 0.5f, track.Max.x, track.GetCenter().y + knob_size.y * 0.5f);
        const float previous = *value;
        if (Interaction::Button(id, hit, ImGuiButtonFlags_PressedOnClick).held)
            *value = ImSaturate((ImGui::GetIO().MousePos.x - travel_left) / travel);

        DrawGradientTrack(draw, track, gradient, stops);
        if (tick) {
            const ImVec2 size = Px(metrics.tick);
            Draw::FillRect(draw, Draw::Snap(ImRect(track.GetCenter().x - size.x * 0.5f, track.Min.y, track.GetCenter().x + size.x * 0.5f, track.Min.y + size.y)), Theme::Colors().label);
        }
        const float center = travel_left + travel * *value;
        const ImRect knob(center - knob_size.x * 0.5f, hit.Min.y, center + knob_size.x * 0.5f, hit.Max.y);
        const CornerRadii radii(Px(metrics.knobRadius));
        Draw::DropShadows(draw, knob, radii, Theme::KnobShadows(false));
        Draw::FillRoundedRect(draw, knob, radii, Theme::Colors().knob);
        return *value != previous;
    }

    // A bezeled field showing shown until it is edited; parse takes each edit and tells whether it set a value.
    static bool EditedField(const char* label, const ImRect& rect, FieldText& field, const char* shown, const std::function<bool(const char*)>& parse) {
        if (!field.editing)
            field.text = shown;
        bool changed = false;
        Layout::Region(ImGui::GetID(label), rect, Layout::ContainerSpec{}, [&] {
            if (TextField(label, &field.text, {.width = Pt(rect.GetWidth()), .alignment = TextAlignment::Trailing}) && !Layout::IsMeasuring())
                changed = parse(field.text.c_str());
        });
        field.editing = ImGui::IsItemActive();
        return changed;
    }

    // A value as a whole number with a suffix; a typed number sets it within [0, limit].
    static bool ValueField(const char* label, const ImRect& rect, FieldText& field, float* value, float limit, const char* suffix) {
        char shown[16];
        std::snprintf(shown, sizeof(shown), "%d%s", int(std::lround(*value)), suffix);
        return EditedField(label, rect, field, shown, [&](const char* text) {
            char* end = nullptr;
            const float typed = std::strtof(text, &end);
            if (end == text)
                return false;
            *value = ImClamp(typed, 0.0f, limit);
            return true;
        });
    }

    // The color as six hex digits; typing six sets it.
    static bool HexField(const ImRect& rect, FieldText& field, Rgba* color) {
        const auto byte = [](float channel) { return unsigned(std::lround(ImSaturate(channel) * 255.0f)); };
        char shown[8];
        std::snprintf(shown, sizeof(shown), "%02X%02X%02X", byte(color->r), byte(color->g), byte(color->b));
        return EditedField("##hex", rect, field, shown, [&](const char* text) {
            char* end = nullptr;
            const unsigned long rgb = std::strtoul(text, &end, 16);
            if (end - text != 6)
                return false;
            *color = Rgba::Hex(unsigned(rgb), color->a);
            return true;
        });
    }

    // The mode tiles' pictures: a small wheel, and red, green and blue bars with white pointers.
    static void DrawSlidersIcon(ImDrawList* draw, const ImRect& frame) {
        const Rgba bars[] = {Rgba(1.0f, 0.0f, 0.0f), Rgba(0.0f, 0.75f, 0.0f), Rgba(0.2f, 0.2f, 1.0f)};
        const float pointers[] = {0.37f, 0.5f, 0.64f};
        const float pitch = frame.GetHeight() / 3.0f;
        const float height = pitch * 0.55f;
        for (int i = 0; i < 3; ++i) {
            const float top = frame.Min.y + pitch * float(i) + (pitch - height) * 0.5f;
            const ImRect bar(frame.Min.x, top, frame.Max.x, top + height);
            const int first = draw->VtxBuffer.Size;
            Draw::FillRoundedRect(draw, bar, CornerRadii(height * 0.3f), bars[i]);
            Draw::ShadeVertices(draw, first, [&](ImVec2 point) { return Blend::Mix(bars[i], Rgba::White(1.0f), 0.55f * (point.x - bar.Min.x) / bar.GetWidth()); });
            Draw::StrokeRoundedRect(draw, bar, CornerRadii(height * 0.3f), bars[i].Opacity(0.5f), Px(0.5f));
            const float x = ImLerp(bar.Min.x, bar.Max.x, pointers[i]);
            const float half = height * 0.6f;
            const ImVec2 pointer[] = {ImVec2(x, bar.Min.y + height * 0.1f), ImVec2(x + half, bar.Min.y + height * 0.5f), ImVec2(x + half, bar.Max.y + height * 0.35f), ImVec2(x - half, bar.Max.y + height * 0.35f), ImVec2(x - half, bar.Min.y + height * 0.5f)};
            draw->AddConvexPolyFilled(pointer, IM_ARRAYSIZE(pointer), Rgba::White(1.0f).Packed());
            draw->AddPolyline(pointer, IM_ARRAYSIZE(pointer), Rgba::Black(0.25f).Packed(), ImDrawFlags_Closed, Px(0.5f));
        }
    }

    // The crosshair on the wheel: a ring with a cross through it, in dark gray.
    static void DrawMarker(ImDrawList* draw, ImVec2 point) {
        const Metrics::ColorPanelMetrics& metrics = Metrics::ColorPanel();
        const Rgba ink = Rgba::Black(0.55f);
        const float arm = Px(metrics.markerArm);
        Draw::StrokeCircle(draw, point, Px(metrics.markerRadius + 0.5f), ink, Px(1.0f));
        Draw::HorizontalLine(draw, point.x - arm, point.x + arm, point.y - Px(0.5f), Px(1.0f), ink);
        Draw::VerticalLine(draw, point.x - Px(0.5f), point.y - arm, point.y + arm, Px(1.0f), ink);
    }

    // The wheel and under it the brightness from full to black.
    static bool WheelMode(ImDrawList* draw, const ImRect& panel, ColorPanelState& state) {
        const Metrics::ColorPanelMetrics& metrics = Metrics::ColorPanel();
        const float radius = Px(metrics.wheelRadius);
        const ImVec2 center(panel.GetCenter().x, panel.Min.y + Px(metrics.barHeight + metrics.wheelTop) + radius);
        float (&wheel)[3] = state.wheel;
        DrawWheel(draw, center, radius, wheel[2]);
        Draw::StrokeCircle(draw, center, radius, Rgba::Black(0.2f), Px(0.5f), StrokeAlignment::Outside);
        bool changed = false;
        if (Interaction::Button(ImGui::GetID("wheel"), ImRect(center - ImVec2(radius, radius), center + ImVec2(radius, radius)), ImGuiButtonFlags_PressedOnClick).held) {
            const ImVec2 offset = ImGui::GetIO().MousePos - center;
            wheel[0] = std::fmod(-std::atan2(offset.y, offset.x) / (2.0f * IM_PI) + 1.0f, 1.0f);
            wheel[1] = ImSaturate(ImSqrt(offset.x * offset.x + offset.y * offset.y) / radius);
            if (wheel[2] <= 0.0f)
                wheel[2] = 1.0f;
            changed = true;
        }
        const float angle = -2.0f * IM_PI * wheel[0];
        DrawMarker(draw, center + ImVec2(std::cos(angle), std::sin(angle)) * radius * wheel[1]);

        const float top = center.y + radius + Px(metrics.trackSpacing);
        const ImRect track(panel.Min.x + Px(metrics.inset), top, panel.Max.x - Px(metrics.inset), top + Px(metrics.trackHeight));
        float darkness = 1.0f - wheel[2];
        const auto gradient = [&](float t) { return WheelColor(wheel[0], wheel[1], 1.0f - t); };
        if (GradientSlider(ImGui::GetID("brightness"), Draw::Snap(track), &darkness, gradient, 2, true)) {
            wheel[2] = 1.0f - darkness;
            changed = true;
        }
        return changed;
    }

    // RGB or HSB: a pop-up for the model, then a labeled gradient slider and a field per channel, and for RGB the hex.
    static void SlidersMode(ImDrawList* draw, const ImRect& panel, ColorPanelState& state, Rgba* color) {
        const Metrics::ColorPanelMetrics& metrics = Metrics::ColorPanel();
        const float pop_up_top = panel.Min.y + Px(metrics.barHeight + metrics.popUpTop);
        const float pop_up_left = panel.GetCenter().x - Px(metrics.popUpWidth * 0.5f);
        Layout::Region(ImGui::GetID("model"), ImRect(pop_up_left, pop_up_top, pop_up_left + Px(metrics.popUpWidth), pop_up_top + Px(Metrics::Slider().height)), Layout::ContainerSpec{}, [&] {
            Picker("##model", &state.sliders, {"RGB Sliders", "HSB Sliders"}, {.width = metrics.popUpWidth});
        });

        const bool rgb = state.sliders == 0;
        static const char* const RgbNames[] = {"Red", "Green", "Blue"};
        static const char* const HsbNames[] = {"Hue", "Saturation", "Brightness"};
        const Font font = Font::Style(TextStyle::Body);
        const float field_left = panel.Max.x - Px(metrics.inset + metrics.fieldWidth);
        const float field_half = Px(Metrics::TextField().height * 0.5f);
        for (int c = 0; c < 3; ++c) {
            ImGui::PushID(c);
            const float baseline = panel.Min.y + Px(metrics.barHeight + metrics.channelTop + metrics.channelPitch * float(c));
            const float label_top = baseline - Typography::Baseline(font);
            Typography::Draw(draw, font, ImRect(panel.Min.x + Px(metrics.inset), label_top, panel.Max.x, label_top + Px(font.lineHeight)), Theme::Colors().label, rgb ? RgbNames[c] : HsbNames[c]);
            const float center = baseline + Px(metrics.channelTrackDrop);
            const ImRect track = Draw::Snap(ImRect(panel.Min.x + Px(metrics.inset), center - Px(metrics.trackHeight * 0.5f), field_left - Px(metrics.fieldSpacing), center + Px(metrics.trackHeight * 0.5f)));
            float* channels[] = {&color->r, &color->g, &color->b};
            float& channel = rgb ? *channels[c] : state.hsb[c];
            const Rgba opaque = color->WithAlpha(1.0f);
            const auto gradient = [&](float t) {
                if (rgb) {
                    Rgba end = opaque;
                    float* ends[] = {&end.r, &end.g, &end.b};
                    *ends[c] = t;
                    return end;
                }
                float hsv[3] = {state.hsb[0], state.hsb[1], state.hsb[2]};
                hsv[c] = t;
                // The hue track stays colorful for grays and dark colors.
                if (c == 0) {
                    hsv[1] = ImMax(hsv[1], 0.5f);
                    hsv[2] = ImMax(hsv[2], 0.75f);
                }
                return FromHsv(hsv, 1.0f);
            };
            bool moved = GradientSlider(ImGui::GetID("slider"), track, &channel, gradient, !rgb && c == 0 ? 37 : 2, false);
            const float limit = rgb ? 255.0f : c == 0 ? 360.0f : 100.0f;
            float shown = channel * limit;
            if (ValueField("##value", ImRect(field_left, center - field_half, panel.Max.x - Px(metrics.inset), center + field_half), state.fields[c], &shown, limit, "")) {
                channel = shown / limit;
                moved = true;
            }
            ImGui::PopID();
            if (!moved)
                continue;
            if (!rgb)
                *color = FromHsv(state.hsb, color->a);
            Follow(state, *color, true, rgb);
        }
        if (!rgb)
            return;
        const float center = panel.Min.y + Px(metrics.barHeight + metrics.channelTop + metrics.channelPitch * 3.0f);
        const float label_top = center - Px(font.lineHeight * 0.5f);
        Typography::Draw(draw, font, ImRect(panel.Min.x + Px(metrics.inset), label_top, panel.Max.x, label_top + Px(font.lineHeight)), Theme::Colors().label, "Hex Color #");
        const float hex_left = panel.Max.x - Px(metrics.inset + metrics.hexWidth);
        if (HexField(ImRect(hex_left, center - field_half, panel.Max.x - Px(metrics.inset), center + field_half), state.fields[4], color))
            Follow(state, *color, true, true);
    }

    bool ColorPanel(bool* is_presented, Rgba* color) {
        if (!*is_presented)
            return false;
        const Metrics::ColorPanelMetrics& metrics = Metrics::ColorPanel();
        const char* title = "Colors";
        ColorPanelState& state = State::Get<ColorPanelState>(ImHashStr(title));
        state.saved.resize(size_t(metrics.savedColumns * metrics.savedRows));
        if (!state.shown || !SameColor(*state.shown, *color))
            Follow(state, *color, true, true);
        const Rgba previous = *color;

        const ImVec2 position(Pt(ImGui::GetMainViewport()->Size.x) - metrics.size.x - metrics.screenMargin, metrics.screenMargin * 2.0f);
        Window(title, is_presented, {.size = metrics.size, .position = position, .minimizable = false, .compactTitleBar = true, .floating = true}, [&] {
            Canvas(metrics.size, [&](ImDrawList* draw, const ImRect& panel) {
                const Palette& colors = Theme::Colors();

                // The title with the mode tiles under it.
                DrawTitleBar(draw, ImRect(panel.Min, ImVec2(panel.Max.x, panel.Min.y + Px(metrics.barHeight))), title);
                for (int m = 0; m < 2; ++m) {
                    const ImVec2 min = panel.Min + Px(ImVec2(metrics.modeLeading + metrics.modePitch * float(m), metrics.modeTop));
                    const ImRect tile(min, min + Px(metrics.modeTile));
                    if (Interaction::Button(ImGui::GetID(m), tile, ImGuiButtonFlags_PressedOnClick).pressed)
                        state.mode = ColorPanelMode(m);
                    if (state.mode == ColorPanelMode(m))
                        Draw::FillRoundedRect(draw, tile, CornerRadii(Px(Metrics::TabBar().backingRadius)), colors.quaternaryFill);
                    if (m == 0) {
                        DrawWheel(draw, tile.GetCenter(), Px(metrics.wheelIcon * 0.5f), 1.0f);
                        Draw::StrokeCircle(draw, tile.GetCenter(), Px(metrics.wheelIcon * 0.5f), Rgba::Black(0.2f), Px(0.5f), StrokeAlignment::Outside);
                    } else {
                        const ImVec2 half = Px(metrics.slidersIcon) * 0.5f;
                        DrawSlidersIcon(draw, ImRect(tile.GetCenter() - half, tile.GetCenter() + half));
                    }
                }

                if (state.mode == ColorPanelMode::Sliders) {
                    SlidersMode(draw, panel, state, color);
                } else if (WheelMode(draw, panel, state)) {
                    *color = WheelColor(state.wheel[0], state.wheel[1], state.wheel[2]).WithAlpha(color->a);
                    Follow(state, *color, false, true);
                }

                // Opacity: a slider with ticks at its ends and middle, and the percent in a field.
                const float line = panel.Max.y - Px(metrics.bottomHeight);
                const Font font = Font::Style(TextStyle::Body);
                const float label_top = line - Px(metrics.opacityBaseline) - Typography::Baseline(font);
                Typography::Draw(draw, font, ImRect(panel.Min.x + Px(metrics.inset), label_top, panel.Max.x, label_top + Px(font.lineHeight)), colors.label, "Opacity");
                const float center = line - Px(metrics.opacityCenter);
                const float field_left = panel.Max.x - Px(metrics.inset + metrics.fieldWidth);
                const float slider_half = Px(Metrics::Slider().height * 0.5f);
                Layout::Region(ImGui::GetID("opacity"), ImRect(panel.Min.x + Px(metrics.opacityLeading), center - slider_half, field_left - Px(metrics.fieldSpacing), center + slider_half), Layout::ContainerSpec{}, [&] {
                    Slider("##opacity", &color->a, 0.0f, 1.0f, {.ticks = 3});
                });
                float percent = color->a * 100.0f;
                const float field_half = Px(Metrics::TextField().height * 0.5f);
                if (ValueField("##alpha", ImRect(field_left, center - field_half, panel.Max.x - Px(metrics.inset), center + field_half), state.fields[3], &percent, 100.0f, "%"))
                    color->a = percent / 100.0f;

                // The color, then the saved ones.
                Draw::HorizontalLine(draw, panel.Min.x, panel.Max.x, line, Px(1.0f), colors.separator);
                const ImVec2 well_min = ImVec2(panel.Min.x, line) + Px(ImVec2(metrics.wellLeading, metrics.wellTop));
                const ImRect well(well_min, well_min + Px(metrics.well));
                const CornerRadii well_radii(Px(metrics.wellRadius));
                Draw::FillRoundedRect(draw, well, well_radii, *color);
                Draw::StrokeRoundedRect(draw, well, well_radii, colors.colorWellLine, Px(1.0f));
                const CornerRadii saved_radii(Px(metrics.savedRadius));
                for (int i = 0; i < int(state.saved.size()); ++i) {
                    const ImVec2 cell(float(i % metrics.savedColumns), float(i / metrics.savedColumns));
                    const ImVec2 min = ImVec2(panel.Min.x, line) + Px(ImVec2(metrics.savedLeading, metrics.savedTop) + cell * metrics.savedPitch);
                    const ImRect swatch(min, min + Px(ImVec2(metrics.savedSwatch, metrics.savedSwatch)));
                    std::optional<Rgba>& saved = state.saved[size_t(i)];
                    if (Interaction::Button(ImGui::GetID(100 + i), swatch).pressed) {
                        if (saved)
                            *color = *saved;
                        else
                            saved = *color;
                    }
                    if (saved)
                        Draw::FillRoundedRect(draw, swatch, saved_radii, *saved);
                    Draw::StrokeRoundedRect(draw, swatch, saved_radii, colors.colorWellLine, Px(1.0f));
                }
            });
        });
        return previous.r != color->r || previous.g != color->g || previous.b != color->b || previous.a != color->a;
    }

    namespace SharedColorPanel {
        struct Binding {
            ImGuiID well = 0;
            Rgba color;
            bool presented = false;
            bool edited = false;
            int frame = 0;
        };

        static Binding& Shared() {
            return State::Get<Binding>(ImHashStr("##CupertinoSharedColorPanel"));
        }

        bool Bind(ImGuiID well, Rgba* color, bool clicked) {
            Binding& binding = Shared();
            if (clicked) {
                const bool active = binding.well == well;
                binding = Binding{.well = active ? 0u : well, .color = *color, .presented = !active};
            }
            if (binding.well != well)
                return false;
            binding.frame = ImGui::GetFrameCount();
            if (!binding.edited) {
                binding.color = *color;
                return false;
            }
            *color = binding.color;
            binding.edited = false;
            return true;
        }

        bool IsActive(ImGuiID well) {
            return well != 0 && Shared().well == well;
        }

        void Present() {
            Binding& binding = Shared();
            // A well that was not drawn this frame is gone, and the window with it.
            if (binding.well == 0 || binding.frame != ImGui::GetFrameCount())
                return;
            if (ColorPanel(&binding.presented, &binding.color))
                binding.edited = true;
            if (!binding.presented)
                binding.well = 0;
        }
    } // namespace SharedColorPanel
} // namespace Cupertino
