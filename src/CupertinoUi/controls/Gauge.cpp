#include "Gauge.h"

#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Metrics.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"

#include <string_view>

namespace Cupertino {
    // One line of text with its line box's top at y, lined up in [left, right].
    static void Line(ImDrawList* draw, const Font& font, float left, float right, float y, Rgba color, const char* text, TextAlignment alignment) {
        if (text)
            Typography::Draw(draw, font, ImRect(left, y, right, y + Px(font.lineHeight)), color, text, alignment);
    }

    // The font made smaller until the text fits the width, as minimumScaleFactor lets SwiftUI do.
    static Font Fitted(const Font& font, const char* text, float width) {
        const float natural = Typography::Width(font, text);
        return natural > width ? Font::System(font.size * width / natural, font.weight) : font;
    }

    // Where a linear style puts its parts around the bar, in points.
    struct LinearLayout {
        float thickness = 0.0f;
        float labelBaseline = 0.0f;
        float valueSpacing = 0.0f;
        float boundSpacing = 0.0f;
        float boundLift = 0.0f;
    };

    static LinearLayout LinearLayoutOf(GaugeStyle style) {
        const Metrics::GaugeMetrics& metrics = Metrics::Gauge();
        if (style == GaugeStyle::LinearCapacity)
            return {metrics.capacityThickness, metrics.capacityLabelBaseline, metrics.capacityValueSpacing, metrics.capacityBoundSpacing, metrics.capacityBoundLift};
        if (style == GaugeStyle::AccessoryLinearCapacity)
            return {metrics.accessoryCapacityThickness, metrics.accessoryLabelBaseline, metrics.accessoryValueSpacing, metrics.accessoryCapacityBoundSpacing, metrics.accessoryCapacityBoundLift};
        return {metrics.accessoryThickness, 0.0f, 0.0f, metrics.accessoryBoundSpacing, 0.0f};
    }

    static void LinearGauge(const char* label, float fraction, const GaugeOptions& options) {
        const Metrics::GaugeMetrics& metrics = Metrics::Gauge();
        const Palette& colors = Theme::Colors();
        const LinearLayout layout = LinearLayoutOf(options.style);
        const bool capacity = options.style == GaugeStyle::LinearCapacity;
        const Font label_font = Font::Style(TextStyle::Body);
        const Font bound_font = options.style == GaugeStyle::AccessoryLinear ? label_font.Weight(FontWeight::Semibold) : label_font;
        const Font value_font = capacity ? label_font : Font::Style(TextStyle::Caption);
        // AccessoryLinear shows only its bar and bounds.
        const bool shows_labels = options.style != GaugeStyle::AccessoryLinear;
        const bool has_label = shows_labels && label && *label;
        const bool has_value = shows_labels && options.currentValueLabel;
        const bool has_bounds = options.minimumValueLabel || options.maximumValueLabel;
        const float thickness = Px(layout.thickness);

        // Line boxes by their offsets from the bar's top; the gauge spans them all.
        const float label_top = -Px(layout.labelBaseline) - Typography::Baseline(label_font);
        const float bound_top = thickness * 0.5f - Px(layout.boundLift) + Typography::CapHeight(bound_font) * 0.5f - Typography::Baseline(bound_font);
        const float value_top = thickness + Px(layout.valueSpacing) + Typography::CapHeight(value_font) - Typography::Baseline(value_font);
        float top = 0.0f;
        float bottom = thickness;
        if (has_label)
            top = ImMin(top, label_top);
        if (has_bounds) {
            top = ImMin(top, bound_top);
            bottom = ImMax(bottom, bound_top + Px(bound_font.lineHeight));
        }
        if (has_value)
            bottom = ImMax(bottom, value_top + Px(value_font.lineHeight));
        const float width = options.width > 0.0f ? Px(options.width) : Layout::Proposal().x;
        Layout::Placement placement;
        placement.size = ImVec2(width, bottom - top);
        placement.flexibleWidth = options.width <= 0.0f;
        const ImRect rect = Layout::Place(placement);
        if (Layout::IsMeasuring())
            return;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Rgba tint = options.tint.a > 0.0f ? options.tint : options.style == GaugeStyle::AccessoryLinear ? colors.label : colors.accent;
        const float bound_gap = Px(layout.boundSpacing);
        const float left = rect.Min.x + (options.minimumValueLabel ? Typography::Width(bound_font, options.minimumValueLabel) + bound_gap : 0.0f);
        const float right = rect.Max.x - (options.maximumValueLabel ? Typography::Width(bound_font, options.maximumValueLabel) + bound_gap : 0.0f);
        const float bar_top = rect.Min.y - top;
        // The capacity gauge centers its labels across the whole gauge; the accessory ones start them at the bar.
        const TextAlignment alignment = capacity ? TextAlignment::Center : TextAlignment::Leading;
        const float text_left = capacity ? rect.Min.x : left;
        if (has_label)
            Line(draw, label_font, text_left, rect.Max.x, bar_top + label_top, colors.label, label, alignment);
        Line(draw, bound_font, rect.Min.x, left, bar_top + bound_top, colors.label, options.minimumValueLabel, TextAlignment::Leading);
        Line(draw, bound_font, right, rect.Max.x, bar_top + bound_top, colors.label, options.maximumValueLabel, TextAlignment::Trailing);
        if (has_value)
            Line(draw, value_font, text_left, rect.Max.x, bar_top + value_top, capacity ? colors.label : colors.secondaryLabel, options.currentValueLabel, alignment);

        const float value_x = ImLerp(left, right, fraction);
        if (options.style == GaugeStyle::AccessoryLinear) {
            // The bar stops short of the dot on both sides, square where it is cut.
            const float radius = thickness * 0.5f;
            const float cut = radius + Px(metrics.markerClearance);
            if (value_x - cut > left)
                Draw::FillRoundedRect(draw, ImRect(left, bar_top, value_x - cut, bar_top + thickness), CornerRadii(radius, 0.0f, 0.0f, radius), tint, CornerStyle::Circular);
            if (value_x + cut < right)
                Draw::FillRoundedRect(draw, ImRect(value_x + cut, bar_top, right, bar_top + thickness), CornerRadii(0.0f, radius, radius, 0.0f), tint, CornerStyle::Circular);
            Draw::FillCircle(draw, ImVec2(value_x, bar_top + radius), radius, tint);
        } else {
            Draw::FillCapsule(draw, ImRect(left, bar_top, right, bar_top + thickness), capacity ? colors.tertiaryFill : colors.fill);
            if (fraction > 0.0f)
                Draw::FillCapsule(draw, ImRect(left, bar_top, value_x, bar_top + thickness), tint);
        }
    }

    static void CircularGauge(const char* label, float fraction, const GaugeOptions& options) {
        const Metrics::GaugeMetrics& metrics = Metrics::Gauge();
        const Palette& colors = Theme::Colors();
        const float diameter = Px(metrics.ringDiameter);
        const ImRect rect = Layout::Place(ImVec2(diameter, diameter));
        if (Layout::IsMeasuring())
            return;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const bool capacity = options.style == GaugeStyle::AccessoryCircularCapacity;
        const Rgba tint = options.tint.a > 0.0f ? options.tint : capacity ? colors.accent : colors.label;
        const float stroke = Px(metrics.ringStroke);
        const float radius = (diameter - stroke) * 0.5f;
        const ImVec2 center = rect.GetCenter();
        const Font small_font = Font::System(metrics.ringLabelSize, FontWeight::Medium);
        const bool bottom_labels = !capacity && ((label && *label) || options.minimumValueLabel || options.maximumValueLabel);
        if (capacity) {
            // Clockwise from 12 o'clock over a track of the tint.
            Draw::Arc(draw, center, radius, stroke, 0.0f, 2.0f * IM_PI, tint.Opacity(metrics.ringTrackOpacity));
            if (fraction > 0.0f)
                Draw::Arc(draw, center, radius, stroke, 0.0f, fraction * 2.0f * IM_PI, tint);
        } else {
            // Three quarters of a ring open at the bottom, round at its ends and cut square around the dot at the value.
            const float start = -IM_PI * 0.75f;
            const float end = IM_PI * 0.75f;
            const float at = ImLerp(start, end, fraction);
            const float dot = Px(metrics.ringMarker) * 0.5f;
            const float cut = Px(metrics.ringMarkerCut) * 0.5f / radius;
            if (at - cut > start)
                Draw::Arc(draw, center, radius, stroke, start, at - cut, tint, true, false);
            if (at + cut < end)
                Draw::Arc(draw, center, radius, stroke, at + cut, end, tint, false, true);
            Draw::FillCircle(draw, center + ImVec2(ImSin(at), -ImCos(at)) * radius, dot, tint);
        }
        const float inner = radius - stroke * 0.5f;
        if (options.currentValueLabel) {
            const Font value_font = Fitted(Font::System(metrics.ringValueSize, FontWeight::Medium), options.currentValueLabel, (inner - Px(metrics.ringValueInset)) * 2.0f);
            const float lift = bottom_labels ? Px(metrics.ringValueLift) : 0.0f;
            const float half = Px(value_font.lineHeight) * 0.5f;
            Typography::Draw(draw, value_font, ImRect(rect.Min.x, center.y - lift - half, rect.Max.x, center.y - lift + half), colors.label, options.currentValueLabel, TextAlignment::Center);
        }
        if (!bottom_labels)
            return;
        // In the opening between the ends of the ring: the label in the middle, or the bounds at its sides, smaller when
        // they would not fit.
        const float opening = radius * 0.70710678f - stroke * 0.5f;
        if (options.minimumValueLabel || options.maximumValueLabel) {
            const float widths = Typography::Width(small_font, options.minimumValueLabel) + Typography::Width(small_font, options.maximumValueLabel) + Px(metrics.ringBoundSpacing);
            const Font font = widths > opening * 2.0f ? Font::System(small_font.size * opening * 2.0f / widths, small_font.weight) : small_font;
            const float top = center.y + Px(metrics.ringLabelBaseline) - Typography::Baseline(font);
            Line(draw, font, center.x - opening, center.x, top, colors.label, options.minimumValueLabel, TextAlignment::Leading);
            Line(draw, font, center.x, center.x + opening, top, colors.label, options.maximumValueLabel, TextAlignment::Trailing);
        } else if (label && *label) {
            const Font font = Fitted(small_font, label, opening * 2.0f);
            const float top = center.y + Px(metrics.ringLabelBaseline) - Typography::Baseline(font);
            Line(draw, font, center.x - opening, center.x + opening, top, colors.label, label, TextAlignment::Center);
        }
    }

    void Gauge(const char* label, float value, const GaugeOptions& options) {
        const float span = options.maximum - options.minimum;
        const float fraction = span != 0.0f ? ImSaturate((value - options.minimum) / span) : 0.0f;
        if (options.style == GaugeStyle::AccessoryCircular || options.style == GaugeStyle::AccessoryCircularCapacity)
            CircularGauge(label, fraction, options);
        else
            LinearGauge(label, fraction, options);
    }
} // namespace Cupertino
