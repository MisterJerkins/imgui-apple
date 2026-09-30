#include "Progress.h"

#include "Bezel.h"
#include "Text.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/Theme.h"
#include "layout/Layout.h"
#include "layout/Stacks.h"

#include <cmath>
#include <functional>
#include <optional>

namespace Cupertino {
    static bool IsSmall() {
        const ControlSize size = Environment().controlSize;
        return size == ControlSize::Small || size == ControlSize::Mini;
    }

    // The kit's spinner: eight 2 x 5 pt spokes of a 16 pt circle, fading behind the darkest one as it steps clockwise
    // once a second.
    static void DrawSpinner(ImDrawList* draw, const ImRect& rect) {
        const Metrics::ProgressMetrics& metrics = Metrics::Progress();
        const float unit = rect.GetWidth() / metrics.smallSpinner;
        const ImVec2 center = rect.GetCenter();
        const ImVec2 spoke = metrics.spoke * unit;
        const int head = int(ImGui::GetTime() * double(metrics.spokes)) % metrics.spokes;
        const Rgba color = Theme::Colors().label;
        for (int i = 0; i < metrics.spokes; ++i) {
            const int behind = (i - head - 1 + metrics.spokes) % metrics.spokes;
            const float alpha = 0.1f * float(behind + 1);
            const int first = draw->VtxBuffer.Size;
            const ImRect bar(center.x - spoke.x * 0.5f, rect.Min.y, center.x + spoke.x * 0.5f, rect.Min.y + spoke.y);
            Draw::FillRoundedRect(draw, bar, CornerRadii(spoke.x * 0.5f), color.WithAlpha(alpha), CornerStyle::Circular);
            Draw::RotateVertices(draw, first, center, 2.0f * IM_PI * float(i) / float(metrics.spokes));
        }
    }

    // A bar's capsule track with the slider's inner shadows, placed at the options' width or the offered one.
    static std::optional<ImRect> PlaceTrack(const ProgressViewOptions& options) {
        const Metrics::ProgressMetrics& metrics = Metrics::Progress();
        const float height = IsSmall() ? metrics.smallBarHeight : metrics.barHeight;
        const float width = options.width > 0.0f ? options.width : ImMax(metrics.minWidth, Pt(Layout::Proposal().x));
        const ImRect bar = Layout::Place(Px(ImVec2(width, height)));
        if (Layout::IsMeasuring())
            return std::nullopt;
        Bezel::Track(ImGui::GetWindowDrawList(), bar);
        return bar;
    }

    // The indicator with its labels: the title over a bar or under a spinner or ring, then the current value.
    static void Labeled(const ProgressViewOptions& options, bool bar, const std::function<void()>& indicator) {
        if (!options.label && !options.currentValueLabel) {
            indicator();
            return;
        }
        VStack({.alignment = bar ? HorizontalAlignment::Leading : HorizontalAlignment::Center, .spacing = Metrics::Progress().labelSpacing}, [&] {
            if (options.label && bar)
                Text(options.label);
            indicator();
            if (options.label && !bar)
                Text(options.label);
            if (options.currentValueLabel)
                Text(options.currentValueLabel, {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary});
        });
    }

    static void Indeterminate(const ProgressViewOptions& options) {
        const Metrics::ProgressMetrics& metrics = Metrics::Progress();
        const Interaction::DisabledFade fade;
        if (options.style == ProgressViewStyle::Linear) {
            const std::optional<ImRect> bar = PlaceTrack(options);
            if (!bar)
                return;
            // The segment enters on the leading side and leaves on the trailing one, its tail fading out behind the head.
            ImDrawList* draw = ImGui::GetWindowDrawList();
            const float length = ImMax(bar->GetWidth() * metrics.sweepLength, bar->GetHeight());
            const float phase = std::fmod(float(ImGui::GetTime()), metrics.sweepPeriod) / metrics.sweepPeriod;
            const float left = bar->Min.x - length + (bar->GetWidth() + length) * phase;
            draw->PushClipRect(bar->Min, bar->Max, true);
            const int first = draw->VtxBuffer.Size;
            Draw::FillRoundedRect(draw, ImRect(left, bar->Min.y, left + length, bar->Max.y), CornerRadii(bar->GetHeight() * 0.5f), Theme::Colors().controlAccent, CornerStyle::Circular);
            Draw::FadeVertices(draw, first, [&](ImVec2 point) { return ImSaturate((point.x - left) / length); });
            draw->PopClipRect();
            return;
        }
        const float size = IsSmall() ? metrics.smallSpinner : metrics.spinner;
        const ImRect rect = Layout::Place(Px(ImVec2(size, size)));
        if (Layout::IsMeasuring())
            return;
        DrawSpinner(ImGui::GetWindowDrawList(), rect);
    }

    static void Determinate(float value, const ProgressViewOptions& options) {
        const Metrics::ProgressMetrics& metrics = Metrics::Progress();
        const Palette& colors = Theme::Colors();
        const float fraction = ImSaturate(value);
        ImDrawList* draw = ImGui::GetWindowDrawList();

        if (options.style == ProgressViewStyle::Circular) {
            // A 5 pt band: the track, and the accent from twelve o'clock clockwise with round ends.
            const float size = IsSmall() ? metrics.smallRing : metrics.ring;
            const ImRect rect = Layout::Place(Px(ImVec2(size, size)));
            if (Layout::IsMeasuring())
                return;
            const Interaction::DisabledFade fade;
            const float band = Px(metrics.ringBand);
            const float radius = rect.GetWidth() * 0.5f - band * 0.5f;
            Draw::Arc(draw, rect.GetCenter(), radius, band, 0.0f, 2.0f * IM_PI, colors.track);
            if (fraction > 0.0f)
                Draw::Arc(draw, rect.GetCenter(), radius, band, 0.0f, 2.0f * IM_PI * fraction, colors.controlAccent);
            return;
        }

        // The track filled with the accent from the leading end.
        const Interaction::DisabledFade fade;
        const std::optional<ImRect> bar = PlaceTrack(options);
        if (bar && fraction > 0.0f)
            Draw::FillCapsule(draw, ImRect(bar->Min, ImVec2(bar->Min.x + bar->GetWidth() * fraction, bar->Max.y)), colors.controlAccent);
    }

    void ProgressView() {
        ProgressView(ProgressViewOptions{});
    }

    void ProgressView(const ProgressViewOptions& options) {
        Labeled(options, options.style == ProgressViewStyle::Linear, [&] { Indeterminate(options); });
    }

    void ProgressView(float value, const ProgressViewOptions& options) {
        Labeled(options, options.style != ProgressViewStyle::Circular, [&] { Determinate(value, options); });
    }
} // namespace Cupertino
