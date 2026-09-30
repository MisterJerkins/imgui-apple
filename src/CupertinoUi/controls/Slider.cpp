#include "Slider.h"

#include "Bezel.h"
#include "LabeledContent.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/State.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"

#include <cmath>

namespace Cupertino {
    static int TickCount(const SliderOptions& options) {
        return options.tickValues.size() > 0 ? int(options.tickValues.size()) : options.ticks;
    }

    // Where a tick sits as a fraction of the range: a given value, or an even step.
    static float TickFraction(const SliderOptions& options, int index) {
        if (options.tickValues.size() > 0)
            return ImSaturate(options.tickValues.begin()[index]);
        return options.ticks > 1 ? float(index) / float(options.ticks - 1) : 0.5f;
    }

    // A round knob, or a bar on a slider with ticks; in points.
    static ImVec2 KnobSize(const SliderOptions& options) {
        const Metrics::SliderMetrics metrics = Metrics::Slider();
        return TickCount(options) > 0 ? metrics.barKnob : ImVec2(metrics.knob, metrics.knob);
    }

    // Left edge of a tick on the pixel grid: the ticks run from one end of the track to the other.
    static float TickLeft(const ImRect& bar, float fraction) {
        return Draw::Snap(bar.Min.x + (bar.GetWidth() - Px(Metrics::Slider().tick.x)) * fraction);
    }

    // bar is the kit's 20 pt high slider frame. Every layer is drawn only where it shows (the fill hides the track, the
    // knob hides both), so a disabled slider fades as one layer, as the kit's group does.
    static void DrawSlider(ImDrawList* draw, const ImRect& bar, float fraction, const SliderOptions& options, bool pressed, bool focused) {
        const Metrics::SliderMetrics metrics = Metrics::Slider();
        const Palette& colors = Theme::Colors();
        const int ticks = TickCount(options);
        const ImVec2 knob = Px(KnobSize(options));
        const float center = bar.Min.x + knob.x * 0.5f + (bar.GetWidth() - knob.x) * fraction;
        const float middle = bar.GetCenter().y;
        const float radius = Px(metrics.trackHeight) * 0.5f;
        const ImRect track(bar.Min.x, middle - radius, bar.Max.x, middle + radius);
        const CornerRadii track_radii(radius);
        // The span of the track the knob covers completely.
        const float covered = ticks > 0 ? knob.x * 0.5f : ImSqrt(knob.x * knob.x * 0.25f - radius * radius);

        const Interaction::DisabledFade fade;
        const auto visible = [&](float x0, float x1, const auto& paint) {
            for (const ImVec2 part : {ImVec2(x0, ImMin(x1, center - covered)), ImVec2(ImMax(x0, center + covered), x1)}) {
                if (part.y - part.x < 0.5f)
                    continue;
                draw->PushClipRect(ImVec2(part.x, bar.Min.y), ImVec2(part.y, bar.Max.y), true);
                paint();
                draw->PopClipRect();
            }
        };
        const auto paint_track = [&] { Bezel::Track(draw, track); };
        const auto paint_fill = [&] { Draw::FillRoundedRect(draw, track, track_radii, colors.controlAccent, CornerStyle::Circular); };
        if (ticks == 0) {
            visible(track.Min.x, center, paint_fill);
            visible(center, track.Max.x, paint_track);
        } else {
            // Ticks and track are one shape: the track shows between the ticks, which are darker. A filled slider takes
            // the accent over both up to the knob (the kit's "Linear with Ticks - Filled").
            const ImVec2 tick = Px(metrics.tick);
            const float filled_to = options.filled ? center : track.Min.x;
            float start = track.Min.x;
            for (int i = 0; i < ticks; ++i) {
                const float left = TickLeft(bar, TickFraction(options, i));
                visible(ImMax(start, filled_to), left, paint_track);
                visible(start, ImMin(left, filled_to), paint_fill);
                start = ImMax(start, left + tick.x);
            }
            visible(ImMax(start, filled_to), track.Max.x, paint_track);
            visible(start, ImMin(track.Max.x, filled_to), paint_fill);
            visible(bar.Min.x, bar.Max.x, [&] {
                for (int i = 0; i < ticks; ++i) {
                    const float left = TickLeft(bar, TickFraction(options, i));
                    const Rgba color = left + tick.x * 0.5f < filled_to ? colors.controlAccent : colors.trackTick;
                    Draw::FillRoundedRect(draw, ImRect(left, middle - tick.y * 0.5f, left + tick.x, middle + tick.y * 0.5f), CornerRadii(tick.x * 0.5f), color, CornerStyle::Circular);
                }
            });
        }
        const ImRect handle(center - knob.x * 0.5f, middle - knob.y * 0.5f, center + knob.x * 0.5f, middle + knob.y * 0.5f);
        Bezel::Pill(draw, handle, CornerRadii(knob.x * 0.5f), colors.sliderKnob, pressed, CornerStyle::Circular);
        // The keyboard focus rings the knob, as NSSlider's.
        if (focused)
            Draw::FocusRing(draw, handle, CornerRadii(knob.x * 0.5f), colors.accent, CornerStyle::Circular);
    }

    // The first caption starts at the track's start and the last one ends at its end; the others are centered at
    // their ticks, or at even steps between.
    static void DrawCaptions(ImDrawList* draw, const ImRect& bar, const SliderOptions& options, Rgba color) {
        const Metrics::SliderMetrics metrics = Metrics::Slider();
        const Font font = Font::System(metrics.captionSize);
        const float top = bar.Max.y + Px(metrics.captionBaseline) - Typography::Baseline(font);
        const int count = int(options.captions.size());
        const bool at_ticks = count == TickCount(options);
        const float tick_width = Px(metrics.tick.x);
        int index = 0;
        for (const char* caption : options.captions) {
            const float width = Typography::Width(font, caption);
            const float anchor = at_ticks ? TickLeft(bar, TickFraction(options, index)) + tick_width * 0.5f : bar.Min.x + bar.GetWidth() * (count > 1 ? float(index) / float(count - 1) : 0.0f);
            const float left = index == 0 ? bar.Min.x : index == count - 1 ? bar.Max.x - width : anchor - width * 0.5f;
            Typography::Draw(draw, font, ImRect(left, top, left + width, top + Px(font.lineHeight)), color, caption);
            ++index;
        }
    }

    struct SliderState {
        // Pointer offset from the knob's center when a drag starts on the knob; a press on the track jumps there.
        float grab = 0.0f;
    };

    static bool SliderBehavior(ImGuiID id, const ImRect& bar, float* value, float min, float max, const SliderOptions& options, bool* pressed, bool* focused) {
        const Interaction::Response response = Interaction::Button(id, bar, ImGuiButtonFlags_PressedOnClick);
        *pressed = response.held;
        *focused = response.focused;
        if (response.focused && max > min && Environment().enabled) {
            // With the keyboard focus the arrows step the value, as NSSlider's: to the next tick, or by a twentieth.
            for (const ImGuiKey key : {ImGuiKey_LeftArrow, ImGuiKey_RightArrow, ImGuiKey_DownArrow, ImGuiKey_UpArrow})
                ImGui::SetKeyOwner(key, id);
            const bool up = ImGui::IsKeyPressed(ImGuiKey_RightArrow, ImGuiInputFlags_Repeat, id) || ImGui::IsKeyPressed(ImGuiKey_UpArrow, ImGuiInputFlags_Repeat, id);
            const bool down = ImGui::IsKeyPressed(ImGuiKey_LeftArrow, ImGuiInputFlags_Repeat, id) || ImGui::IsKeyPressed(ImGuiKey_DownArrow, ImGuiInputFlags_Repeat, id);
            if (up != down) {
                const float fraction = ImSaturate((*value - min) / (max - min));
                float next = ImSaturate(fraction + (up ? 0.05f : -0.05f));
                if (TickCount(options) > 0) {
                    next = up ? 1.0f : 0.0f;
                    for (int i = 0; i < TickCount(options); ++i) {
                        const float tick = TickFraction(options, i);
                        if (up && tick > fraction + 1e-4f)
                            next = ImMin(next, tick);
                        if (!up && tick < fraction - 1e-4f)
                            next = ImMax(next, tick);
                    }
                }
                const float stepped = ImLerp(min, max, next);
                if (stepped != *value) {
                    *value = stepped;
                    return true;
                }
            }
        }
        if (!response.held || max <= min)
            return false;
        const float knob = Px(KnobSize(options).x);
        const float travel = ImMax(bar.GetWidth() - knob, 1.0f);
        const float pointer = ImGui::GetIO().MousePos.x;
        SliderState& state = State::Get<SliderState>(id);
        if (response.pressed) {
            const float center = bar.Min.x + knob * 0.5f + travel * ImSaturate((*value - min) / (max - min));
            state.grab = ImFabs(pointer - center) <= knob * 0.5f ? pointer - center : 0.0f;
        }
        float fraction = ImSaturate((pointer - state.grab - bar.Min.x - knob * 0.5f) / travel);
        if (options.snapsToTicks && TickCount(options) > 0) {
            float nearest = TickFraction(options, 0);
            for (int i = 1; i < TickCount(options); ++i) {
                if (ImFabs(TickFraction(options, i) - fraction) < ImFabs(nearest - fraction))
                    nearest = TickFraction(options, i);
            }
            fraction = nearest;
        }
        const float next = ImLerp(min, max, fraction);
        if (next == *value)
            return false;
        *value = next;
        return true;
    }

    // The circular slider (kit, 22 pt): the round white bezel, and a 6 pt dot 7 pt from its center at the value's angle,
    // clockwise from the top. Dragging turns the dot toward the pointer.
    static bool CircularSlider(const char* label, float* value, float min, float max) {
        const Metrics::SliderMetrics metrics = Metrics::Slider();
        const ImRect frame = Layout::Place(Px(ImVec2(metrics.circular, metrics.circular)));
        if (Layout::IsMeasuring())
            return false;
        const Interaction::Response response = Interaction::Button(ImGui::GetID(label), frame, ImGuiButtonFlags_PressedOnClick);
        const ImVec2 center = frame.GetCenter();
        bool changed = false;
        if (response.held && max > min) {
            const ImVec2 pointer = ImGui::GetIO().MousePos - center;
            float turn = (std::atan2(pointer.x, -pointer.y)) / (2.0f * IM_PI);
            turn = turn < 0.0f ? turn + 1.0f : turn;
            const float next = ImLerp(min, max, turn);
            changed = next != *value;
            *value = next;
        }
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        {
            const Interaction::DisabledFade fade;
            Bezel::Pill(draw, frame, CornerRadii(frame.GetWidth() * 0.5f), response.held && response.hovered, CornerStyle::Circular);
        }
        const float angle = 2.0f * IM_PI * (max > min ? ImSaturate((*value - min) / (max - min)) : 0.0f);
        const ImVec2 dot = center + Px(metrics.circularKnobDistance) * ImVec2(std::sin(angle), -std::cos(angle));
        Draw::FillCircle(draw, dot, Px(metrics.circularKnob) * 0.5f, Environment().enabled ? colors.secondaryLabel : colors.tertiaryLabel);
        if (response.focused)
            Draw::FocusRing(draw, frame, CornerRadii(frame.GetWidth() * 0.5f), colors.accent, CornerStyle::Circular);
        return changed;
    }

    bool Slider(const char* label, float* value, float min, float max, const SliderOptions& options) {
        if (options.style == SliderStyle::Circular)
            return CircularSlider(label, value, min, max);
        const Metrics::SliderMetrics metrics = Metrics::Slider();
        const Font body = Font::Style(TextStyle::Body);
        const Font symbols = body.ImageScale(options.symbolScale);
        const std::string_view text = Interaction::VisibleLabel(label);
        const float leading = options.minimumSymbol ? Pt(Typography::SymbolWidth(options.minimumSymbol, symbols)) + metrics.symbolSpacing : 0.0f;
        const float trailing = options.maximumSymbol ? Pt(Typography::SymbolWidth(options.maximumSymbol, symbols)) + metrics.symbolSpacing : 0.0f;
        const bool captioned = options.captions.size() > 0;
        const bool large_symbols = options.symbolScale == SymbolScale::Large && (options.minimumSymbol || options.maximumSymbol);
        const float line = large_symbols ? metrics.largeSymbolHeight : metrics.height;
        const float height = line + (captioned ? metrics.captionHeight : 0.0f);

        ImRect accessory;
        if (Layout::ParentRole() == Layout::Role::FormSection) {
            // Without a width the track starts in the middle of the row, as System Settings' Brightness does.
            const Metrics::FormMetrics& form = Metrics::Form();
            const float track = options.width > 0.0f ? options.width : ImMax(metrics.minWidth, Pt(Layout::FullProposal().x) * 0.5f - form.valueTrailing - trailing);
            const bool ticked = TickCount(options) > 0 && !captioned;
            const FormRowSpec spec = {.label = text, .accessorySize = ImVec2(leading + track + trailing, height + (ticked ? metrics.tickRoom : 0.0f)), .trailingInset = form.valueTrailing, .accessoryTop = ticked ? metrics.tickedFormTop : metrics.formTop};
            const FormRowFrame row = PlaceFormRow(spec);
            if (Layout::IsMeasuring())
                return false;
            DrawFormRowLabel(row, spec);
            accessory = row.accessory;
        } else {
            const float label_width = text.empty() ? 0.0f : Pt(Typography::Width(body, text)) + metrics.labelSpacing;
            const float track = options.width > 0.0f ? options.width : ImMax(metrics.minWidth, Pt(Layout::Proposal().x) - label_width - leading - trailing);
            const ImRect rect = Layout::Place(Layout::Placement{.size = Px(ImVec2(label_width + leading + track + trailing, height)), .baseline = text.empty() ? -1.0f : Typography::CenteredBaseline(body, Px(line))});
            if (Layout::IsMeasuring())
                return false;
            if (!text.empty())
                Typography::Draw(ImGui::GetWindowDrawList(), body, ImRect(rect.Min, ImVec2(rect.Min.x + Px(label_width), rect.Min.y + Px(line))), Theme::Colors().LabelColor(Environment().enabled), text);
            accessory = ImRect(rect.Min.x + Px(label_width), rect.Min.y, rect.Max.x, rect.Max.y);
        }

        const float bar_top = accessory.Min.y + Px(line - metrics.height) * 0.5f;
        const ImRect bar(accessory.Min.x + Px(leading), bar_top, accessory.Max.x - Px(trailing), bar_top + Px(metrics.height));
        bool pressed = false;
        bool focused = false;
        const bool changed = SliderBehavior(ImGui::GetID(label), bar, value, min, max, options, &pressed, &focused);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        DrawSlider(draw, bar, max > min ? ImSaturate((*value - min) / (max - min)) : 0.0f, options, pressed, focused);

        const Palette& colors = Theme::Colors();
        const bool enabled = Environment().enabled;
        const Rgba symbol_color = enabled ? colors.secondaryLabel : colors.tertiaryLabel;
        if (options.minimumSymbol)
            Typography::DrawSymbol(draw, options.minimumSymbol, symbols, ImRect(accessory.Min.x, accessory.Min.y, bar.Min.x - Px(metrics.symbolSpacing), accessory.Min.y + Px(line)), symbol_color);
        if (options.maximumSymbol)
            Typography::DrawSymbol(draw, options.maximumSymbol, symbols, ImRect(bar.Max.x + Px(metrics.symbolSpacing), accessory.Min.y, accessory.Max.x, accessory.Min.y + Px(line)), symbol_color);
        if (captioned)
            DrawCaptions(draw, bar, options, colors.LabelColor(enabled));
        return changed;
    }
} // namespace Cupertino
