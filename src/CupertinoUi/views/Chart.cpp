#include "Chart.h"

#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/State.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace Cupertino {
    static void DashedVertical(ImDrawList* draw, float x, float top, float bottom, float width, float dash, Rgba color) {
        for (float y = top; y < bottom; y += 2.0f * dash)
            Draw::FillRect(draw, ImRect(x - width * 0.5f, y, x + width * 0.5f, ImMin(y + dash, bottom)), color);
    }

    // The widest label of the marks, at least minimum (pixels).
    static float MarkLabelWidth(std::span<const AxisMark> marks, const Font& font, float minimum) {
        float width = minimum;
        for (const AxisMark& mark : marks)
            width = ImMax(width, Typography::Width(font, mark.label));
        return width;
    }

    // Gridlines across the plot centered on the marks' values.
    static void Gridlines(ImDrawList* draw, const ImRect& plot, std::span<const AxisMark> marks, const std::function<float(float)>& y_at) {
        const float half = Px(Metrics::Chart().gridline * 0.5f);
        for (const AxisMark& mark : marks)
            Draw::FillRect(draw, ImRect(plot.Min.x, y_at(mark.value) - half, plot.Max.x, y_at(mark.value) + half), Theme::Colors().chartGrid);
    }

    // The marks' labels from left to right, each centered on its gridline.
    static void TrailingLabels(ImDrawList* draw, const Font& font, float left, float right, std::span<const AxisMark> marks, const std::function<float(float)>& y_at) {
        const float half = Px(font.lineHeight) * 0.5f;
        for (const AxisMark& mark : marks) {
            if (mark.label)
                Typography::Draw(draw, font, ImRect(left, y_at(mark.value) - half, right, y_at(mark.value) + half), Theme::Colors().secondaryLabel, mark.label);
        }
    }

    void BarChart(std::span<const float> values, const BarChartOptions& options) {
        const Metrics::ChartMetrics& metrics = Metrics::Chart();
        const Font label_font = Font::Style(TextStyle::Callout);
        const float label_width = MarkLabelWidth(options.yMarks, label_font, Px(options.labelWidth));
        // The top gridline straddles the top of the plot, so the chart starts half a line above it; under the plot come
        // the rows of the band, the hour labels and the dates.
        const bool dated = std::any_of(options.dayMarks.begin(), options.dayMarks.end(), [](const AxisMark& mark) { return mark.label != nullptr; });
        const int rows = int(!options.band.empty()) + int(options.xMarks.size() > 0) + int(dated);
        const ImRect rect = Layout::Place(ImVec2(Layout::Proposal().x, Px(metrics.gridline * 0.5f + options.height + metrics.axisRow * float(rows) + metrics.axisBottom)));
        if (Layout::IsMeasuring())
            return;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const Rgba color = options.color.a > 0.0f ? options.color : Theme::SystemGreen();
        const float label_column = label_width > 0.0f ? Px(metrics.labelSpacing) + label_width : 0.0f;
        const float plot_top = rect.Min.y + Px(metrics.gridline * 0.5f);
        const ImRect plot(rect.Min.x, plot_top, rect.Max.x - label_column, plot_top + Px(options.height));
        const int slots = options.slots > 0 ? options.slots : int(values.size());
        const float pitch = slots > 0 ? plot.GetWidth() / float(slots) : 0.0f;
        const auto slot_x = [&](float slot) { return plot.Min.x + pitch * slot; };
        const auto value_y = [&](float value) { return plot.Max.y - ImSaturate(value / options.maximum) * plot.GetHeight(); };

        // Gridlines are centered on their values; charging runs are tinted over them down to their capsule.
        Gridlines(draw, plot, options.yMarks, value_y);
        const float capsule_top = plot.Max.y + Px(metrics.bandGap);
        for (int i = 0; i < int(options.band.size()) && i < slots; ++i) {
            if (!options.band[size_t(i)])
                continue;
            int end = i;
            while (end + 1 < int(options.band.size()) && end + 1 < slots && options.band[size_t(end + 1)])
                ++end;
            Draw::FillRect(draw, ImRect(slot_x(float(i)), plot.Min.y, slot_x(float(end + 1)), capsule_top), color.Opacity(metrics.bandOpacity));
            Draw::FillRoundedRect(draw, ImRect(slot_x(float(i)), capsule_top, slot_x(float(end + 1)), capsule_top + Px(metrics.bandCapsule)), CornerRadii(Px(metrics.bandCapsule * 0.5f)), color, CornerStyle::Circular);
            i = end;
        }

        // Verticals run through the plot and on through the hour labels, dashed afresh from the plot's bottom; the edges
        // and the solid lines where days begin run through all the rows.
        const float hours_top = plot.Max.y + (options.band.empty() ? 0.0f : Px(metrics.axisRow));
        const float rows_bottom = plot.Max.y + Px(metrics.axisRow) * float(rows);
        const float half_line = Px(metrics.dashWidth) * 0.5f;
        const auto begins_day = [&](float slot) { return std::any_of(options.dayMarks.begin(), options.dayMarks.end(), [&](const AxisMark& mark) { return mark.value == slot; }); };
        for (const AxisMark& mark : options.dayMarks)
            Draw::FillRect(draw, ImRect(slot_x(mark.value) - half_line, rect.Min.y, slot_x(mark.value) + half_line, rows_bottom), colors.chartGrid);
        if (options.gridEvery > 0) {
            for (int i = 0; i <= slots; i += options.gridEvery) {
                if (begins_day(float(i)))
                    continue;
                const float bottom = i == 0 || i == slots ? rows_bottom : hours_top + Px(metrics.axisRow);
                DashedVertical(draw, slot_x(float(i)), plot.Min.y, plot.Max.y, Px(metrics.dashWidth), Px(metrics.dash), colors.chartGrid);
                DashedVertical(draw, slot_x(float(i)), plot.Max.y, bottom, Px(metrics.dashWidth), Px(metrics.dash), colors.chartGrid);
            }
        }

        // Bars stand centered in their slots on fractional edges, their top corners rounded.
        const float bar_width = ImMin(Px(options.barWidth), pitch);
        for (int i = 0; i < int(values.size()) && i < slots; ++i) {
            const float top = value_y(values[size_t(i)]);
            if (top >= plot.Max.y)
                continue;
            const float left = slot_x(float(i)) + (pitch - bar_width) * 0.5f;
            const float radius = ImMin(ImMin(Px(metrics.barRadius), bar_width * 0.5f), plot.Max.y - top);
            Draw::FillRoundedRect(draw, ImRect(left, top, left + bar_width, plot.Max.y), CornerRadii(radius, radius, 0.0f, 0.0f), color, CornerStyle::Circular);
        }

        // Labels on the trailing side are centered on their gridlines; the ones below start after their slot's line, dates
        // a line lower.
        TrailingLabels(draw, label_font, plot.Max.x + Px(metrics.labelSpacing), rect.Max.x, options.yMarks, value_y);
        const float hour_baseline = hours_top + Px(metrics.xLabelBaseline);
        const auto draw_label = [&](const AxisMark& mark, float baseline) {
            const float top = baseline - Typography::Baseline(label_font);
            Typography::Draw(draw, label_font, ImRect(slot_x(mark.value) + Px(metrics.xLabelInset), top, rect.Max.x, top + Px(label_font.lineHeight)), colors.secondaryLabel, mark.label);
        };
        for (const AxisMark& mark : options.xMarks) {
            if (mark.label)
                draw_label(mark, hour_baseline);
        }
        for (const AxisMark& mark : options.dayMarks) {
            if (mark.label)
                draw_label(mark, hour_baseline + Px(metrics.dateSpacing));
        }
    }

    void AreaChart(std::span<const float> values, const AreaChartOptions& options) {
        Layout::Placement placement;
        placement.size = ImVec2(Layout::Proposal().x, Px(options.height));
        placement.flexibleWidth = true;
        const ImRect rect = Layout::Place(placement);
        if (Layout::IsMeasuring() || values.size() < 2)
            return;

        const float maximum = options.maximum > 0.0f ? options.maximum : ImMax(*std::max_element(values.begin(), values.end()), FLT_MIN);
        const float half = Px(options.lineWidth) * 0.5f;
        const ImRect plot(rect.Min + ImVec2(half, half), rect.Max - ImVec2(half, half));
        const auto value_y = [&](float value) { return plot.Max.y - ImSaturate(value / maximum) * plot.GetHeight(); };
        const float step = options.spacing > 0.0f ? Px(options.spacing) : plot.GetWidth() / float(values.size() - 1);
        const float first_x = options.spacing > 0.0f ? plot.Max.x - step * float(values.size() - 1) : plot.Min.x;
        // The line's points between the two floor corners of the area; a value before the leading edge only aims the
        // line coming in.
        std::vector<ImVec2> points;
        points.reserve(values.size() + 2);
        points.push_back(ImVec2());
        for (size_t i = 0; i < values.size(); ++i) {
            const ImVec2 point(first_x + step * float(i), value_y(values[i]));
            if (point.x < plot.Min.x - 0.01f)
                continue;
            if (points.size() == 1 && i > 0) {
                const float previous = value_y(values[i - 1]);
                points.push_back(ImVec2(plot.Min.x, ImLerp(previous, point.y, (plot.Min.x - (point.x - step)) / step)));
            }
            points.push_back(point);
        }
        if (points.size() < 3)
            return;
        points.front() = ImVec2(points[1].x, plot.Max.y);
        points.push_back(ImVec2(points.back().x, plot.Max.y));

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Rgba color = options.color.a > 0.0f ? options.color : Theme::SystemBlue();
        if (options.areaOpacity > 0.0f) {
            // The line covers the area's edges; ImGui's fringe would fold at the spikes.
            const ImDrawListFlags flags = draw->Flags;
            draw->Flags &= ~ImDrawListFlags_AntiAliasedFill;
            draw->AddConcavePolyFilled(points.data(), int(points.size()), color.Opacity(options.areaOpacity).Packed());
            draw->Flags = flags;
        }
        const std::span<const ImVec2> all(points);
        Draw::Polyline(draw, options.outlinesArea ? all : all.subspan(1, points.size() - 2), color, Px(options.lineWidth), options.outlinesArea);
    }

    Rgba ChartColor(int index) {
        const Rgba palette[] = {Theme::SystemBlue(), Theme::SystemGreen(), Theme::SystemOrange(), Theme::SystemPurple(), Theme::SystemRed(), Theme::SystemCyan(), Theme::SystemYellow()};
        return palette[index % int(std::size(palette))];
    }

    struct LegendEntry {
        const char* name = nullptr;
        Rgba color;
    };

    // Lays the legend out in rows across the width from origin, lined up by the alignment: a dot centered on each name's
    // capitals before it. Draws when given a draw list; returns the height either way.
    static float Legend(ImDrawList* draw, std::span<const LegendEntry> entries, const Font& font, ImVec2 origin, float width, HorizontalAlignment alignment) {
        if (entries.empty())
            return 0.0f;
        const Metrics::ChartMetrics& metrics = Metrics::Chart();
        const float row_height = Px(font.lineHeight);
        const float dot = Px(metrics.legendDot);
        const float spacing = Px(metrics.legendSpacing);
        if (alignment == HorizontalAlignment::Leading) {
            origin.x += Px(metrics.legendInset);
            width -= Px(metrics.legendInset);
        }
        // Rows first, so that each can be lined up by its own width.
        std::vector<float> widths(entries.size());
        std::vector<int> row_of(entries.size());
        std::vector<float> row_widths(1, 0.0f);
        for (size_t i = 0; i < entries.size(); ++i) {
            widths[i] = dot + Px(metrics.legendDotSpacing) + Typography::Width(font, entries[i].name);
            float& row = row_widths.back();
            if (row > 0.0f && row + spacing + widths[i] > width)
                row_widths.push_back(0.0f);
            row_widths.back() += (row_widths.back() > 0.0f ? spacing : 0.0f) + widths[i];
            row_of[i] = int(row_widths.size()) - 1;
        }
        if (draw) {
            const Rgba color = Theme::Colors().secondaryLabel;
            float x = 0.0f;
            for (size_t i = 0; i < entries.size(); ++i) {
                const float free = width - row_widths[size_t(row_of[i])];
                if (i == 0 || row_of[i] != row_of[i - 1])
                    x = origin.x + (alignment == HorizontalAlignment::Center ? free * 0.5f : alignment == HorizontalAlignment::Trailing ? free : 0.0f);
                const float top = origin.y + row_height * float(row_of[i]);
                const float center_y = top + Typography::Baseline(font) - Typography::CapHeight(font) * 0.5f;
                Draw::FillCircle(draw, ImVec2(x + dot * 0.5f, center_y), dot * 0.5f, entries[i].color);
                Typography::Draw(draw, font, ImRect(x + dot + Px(metrics.legendDotSpacing), top, x + widths[i], top + row_height), color, entries[i].name);
                x += widths[i] + spacing;
            }
        }
        return row_height * float(row_widths.size());
    }

    // Whether the pointer is over an area this frame, reporting when it has just left: selections bound to hovering
    // clear once as the pointer leaves and otherwise keep what the app sets.
    struct ChartHover {
        bool hovered = false;
    };

    static bool TrackHover(ImGuiID id, const ImRect& area, bool* left) {
        const bool hovered = Interaction::Button(id, area, 0, ImGuiItemFlags_NoNav).hovered;
        bool& was_hovered = State::Get<ChartHover>(id).hovered;
        *left = was_hovered && !hovered;
        was_hovered = hovered;
        return hovered;
    }

    void LineChart(std::span<const ChartSeries> series, const LineChartOptions& options) {
        const Metrics::ChartMetrics& metrics = Metrics::Chart();
        const Font& font = options.labelFont;
        const float label_width = MarkLabelWidth(options.yMarks, font, Px(options.labelWidth));
        std::vector<LegendEntry> legend;
        for (size_t i = 0; i < series.size() && options.showsLegend; ++i) {
            if (series[i].name)
                legend.push_back({series[i].name, series[i].color.a > 0.0f ? series[i].color : ChartColor(int(i))});
        }
        const bool labeled = std::any_of(options.xMarks.begin(), options.xMarks.end(), [](const AxisMark& mark) { return mark.label != nullptr; });
        const float axis_row = labeled ? Px(metrics.markAxisRow) : 0.0f;
        const float legend_gap = options.legendSpacing > 0.0f ? Px(options.legendSpacing) : labeled ? 0.0f : Px(metrics.legendGap);
        const float width = Layout::Proposal().x;
        const float legend_height = Legend(nullptr, legend, font, ImVec2(), width, options.legendAlignment);
        const float height = Px(metrics.gridline * 0.5f + options.height) + axis_row + (legend.empty() ? 0.0f : legend_gap + legend_height);
        const ImRect rect = Layout::Place(ImVec2(width, height));
        if (Layout::IsMeasuring())
            return;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const float label_column = label_width > 0.0f ? Px(metrics.markLabelSpacing) + label_width : 0.0f;
        const float plot_top = rect.Min.y + Px(metrics.gridline * 0.5f);
        const ImRect plot(rect.Min.x, plot_top, rect.Max.x - label_column, plot_top + Px(options.height));

        // The domains: the values from the minimum to the maximum or the largest value, the places from first to last or
        // across the positions.
        float maximum = options.maximum;
        float first = options.first;
        float last = options.last;
        const bool fit_x = first == last;
        if (fit_x) {
            first = FLT_MAX;
            last = -FLT_MAX;
        }
        for (const ChartSeries& one : series) {
            for (size_t i = 0; i < one.values.size(); ++i) {
                if (options.maximum <= 0.0f)
                    maximum = ImMax(maximum, one.values[i]);
                if (fit_x) {
                    const float place = i < one.positions.size() ? one.positions[i] : float(i);
                    first = ImMin(first, place);
                    last = ImMax(last, place);
                }
            }
        }
        if (last <= first)
            last = first + 1.0f;
        if (maximum <= options.minimum)
            maximum = options.minimum + 1.0f;
        const auto x_at = [&](float place) { return plot.Min.x + (place - first) / (last - first) * plot.GetWidth(); };
        const auto y_at = [&](float value) { return plot.Max.y - (value - options.minimum) / (maximum - options.minimum) * plot.GetHeight(); };
        const auto point_at = [&](const ChartSeries& one, size_t i) { return ImVec2(x_at(i < one.positions.size() ? one.positions[i] : float(i)), y_at(one.values[i])); };

        // Gridlines centered on their values, then verticals dashed from the top of the plot down through the label row.
        Gridlines(draw, plot, options.yMarks, y_at);
        for (const AxisMark& mark : options.xMarks)
            DashedVertical(draw, x_at(mark.value), plot.Min.y, plot.Max.y + axis_row, Px(metrics.dashWidth), Px(metrics.dash), colors.chartGrid);

        // chartXSelection: the index of the value nearest the pointer along the first series.
        const ImGuiID id = Layout::NextViewId();
        int selected = options.selection ? *options.selection : -1;
        if (options.selection && !series.empty()) {
            bool left = false;
            if (TrackHover(id, plot, &left)) {
                const ChartSeries& one = series.front();
                float nearest = FLT_MAX;
                for (size_t i = 0; i < one.values.size(); ++i) {
                    const float distance = ImFabs(point_at(one, i).x - ImGui::GetIO().MousePos.x);
                    if (distance < nearest) {
                        nearest = distance;
                        selected = int(i);
                    }
                }
            } else if (left) {
                selected = -1;
            }
            *options.selection = selected;
        }
        if (selected >= 0) {
            const float x = series.empty() || size_t(selected) >= series.front().values.size() ? x_at(float(selected)) : point_at(series.front(), size_t(selected)).x;
            Draw::FillRect(draw, ImRect(x - Px(metrics.ruleWidth * 0.5f), plot.Min.y, x + Px(metrics.ruleWidth * 0.5f), plot.Max.y), Theme::SystemGray().Opacity(metrics.selectionRuleOpacity));
        }

        // Marks in order: each series' line, then its points; rules over them.
        std::vector<ImVec2> points;
        for (size_t s = 0; s < series.size(); ++s) {
            const ChartSeries& one = series[s];
            const Rgba color = one.color.a > 0.0f ? one.color : ChartColor(int(s));
            points.clear();
            for (size_t i = 0; i < one.values.size(); ++i)
                points.push_back(point_at(one, i));
            if (options.line && points.size() > 1)
                Draw::Polyline(draw, points, color, Px(metrics.lineWidth));
            for (size_t i = 0; i < points.size(); ++i) {
                if (options.points || int(i) == selected)
                    Draw::FillCircle(draw, points[i], Px(metrics.pointSize * 0.5f), color);
            }
        }
        for (const ChartRule& rule : options.rules) {
            const float y = y_at(rule.value);
            const Rgba color = rule.color.a > 0.0f ? rule.color : colors.accent;
            Draw::FillRect(draw, ImRect(plot.Min.x, y - Px(metrics.ruleWidth * 0.5f), plot.Max.x, y + Px(metrics.ruleWidth * 0.5f)), color);
            if (rule.label) {
                const float bottom = y - Px(metrics.ruleWidth * 0.5f);
                Typography::Draw(draw, font, ImRect(plot.Min.x, bottom - Px(font.lineHeight), plot.Max.x, bottom), color, rule.label);
            }
        }

        // The annotation over the plot at the chosen value, kept inside the chart.
        if (selected >= 0 && options.annotation) {
            const std::string text = options.annotation(selected);
            const ImVec2 size = Typography::Measure(font, text) + ImVec2(Px(metrics.annotationPadding), Px(metrics.annotationPadding)) * 2.0f;
            const float x = series.empty() || size_t(selected) >= series.front().values.size() ? x_at(float(selected)) : point_at(series.front(), size_t(selected)).x;
            const float left = ImClamp(x - size.x * 0.5f, plot.Min.x, ImMax(plot.Min.x, plot.Max.x - size.x));
            const ImRect box(left, plot.Min.y, left + size.x, plot.Min.y + size.y);
            Draw::FillRoundedRect(draw, box, CornerRadii(Px(metrics.annotationRadius)), Theme::SystemGray().Opacity(metrics.annotationOpacity));
            Typography::DrawWrapped(draw, font, ImRect(box.Min + ImVec2(Px(metrics.annotationPadding), Px(metrics.annotationPadding)), box.Max), colors.label, text);
        }

        // Labels: trailing ones centered on their gridlines, lower ones after their verticals on one baseline.
        TrailingLabels(draw, font, plot.Max.x + Px(metrics.markLabelSpacing), rect.Max.x, options.yMarks, y_at);
        // A label that would run past the plot's trailing edge is left out, as the render drops "80" at its last line.
        for (const AxisMark& mark : options.xMarks) {
            const float left = x_at(mark.value) + Px(metrics.markLabelInset);
            if (!mark.label || left + Typography::Width(font, mark.label) > plot.Max.x)
                continue;
            const float top = plot.Max.y + Px(metrics.markLabelBaseline) - Typography::Baseline(font);
            Typography::Draw(draw, font, ImRect(left, top, rect.Max.x, top + Px(font.lineHeight)), colors.secondaryLabel, mark.label);
        }
        Legend(draw, legend, font, ImVec2(rect.Min.x, plot.Max.y + axis_row + (legend.empty() ? 0.0f : legend_gap)), width, options.legendAlignment);
    }

    // A sector's outline: its outer arc, then its inner arc back or the tip of a pie; each side moved in by the inset
    // and each corner rounded by the circle that touches the side and the arc.
    static void SectorOutline(ImVector<ImVec2>& out, ImVec2 center, float outer, float inner, float start, float end, float inset, float corner) {
        const auto direction = [](float angle) { return ImVec2(std::cos(angle), std::sin(angle)); };
        const auto angle_of = [](ImVec2 v) { return std::atan2(v.y, v.x); };
        const auto arc = [&](ImVec2 at, float radius, float from, float to) {
            const int steps = Draw::ArcSegments(to - from, 2);
            for (int k = 0; k <= steps; ++k)
                out.push_back(at + direction(ImLerp(from, to, float(k) / float(steps))) * radius);
        };
        // The corner circle at a side (sign +1 at the start, -1 at the end) against a circle of the given radius:
        // grows outward (+1) or inward (-1).
        struct Corner {
            ImVec2 circle;
            ImVec2 onSide;
            ImVec2 onArc;
        };
        const auto corner_at = [&](float side, float sign, float radius, float grow) {
            const ImVec2 along = direction(side);
            const ImVec2 normal = ImVec2(-along.y, along.x) * sign;
            const float reach = radius - grow * corner;
            const float t = std::sqrt(ImMax(reach * reach - (inset + corner) * (inset + corner), 0.0f));
            Corner result;
            result.circle = center + along * t + normal * (inset + corner);
            result.onSide = center + along * t + normal * inset;
            result.onArc = center + Draw::Normalized(result.circle - center) * radius;
            return result;
        };
        const auto corner_arc = [&](const Corner& c, ImVec2 from, ImVec2 to) {
            if (corner <= 0.0f)
                return;
            const float a = angle_of(from - c.circle);
            float b = angle_of(to - c.circle);
            while (b - a > IM_PI)
                b -= 2.0f * IM_PI;
            while (b - a < -IM_PI)
                b += 2.0f * IM_PI;
            arc(c.circle, corner, a, b);
        };

        const Corner outer_start = corner_at(start, 1.0f, outer, 1.0f);
        const Corner outer_end = corner_at(end, -1.0f, outer, 1.0f);
        corner_arc(outer_start, outer_start.onSide, outer_start.onArc);
        float from = angle_of(outer_start.onArc - center);
        float to = angle_of(outer_end.onArc - center);
        while (to < from)
            to += 2.0f * IM_PI;
        arc(center, outer, from, to);
        corner_arc(outer_end, outer_end.onArc, outer_end.onSide);
        if (inner > inset + corner) {
            const Corner inner_end = corner_at(end, -1.0f, inner, -1.0f);
            const Corner inner_start = corner_at(start, 1.0f, inner, -1.0f);
            corner_arc(inner_end, inner_end.onSide, inner_end.onArc);
            from = angle_of(inner_end.onArc - center);
            to = angle_of(inner_start.onArc - center);
            while (to > from)
                to -= 2.0f * IM_PI;
            arc(center, inner, from, to);
            corner_arc(inner_start, inner_start.onArc, inner_start.onSide);
        } else {
            // A pie's sides meet where the inset lines cross, on the bisector.
            const float half = (end - start) * 0.5f;
            out.push_back(center + direction(start + half) * (inset / ImMax(std::sin(half), 0.05f)));
        }
        // Where a corner meets an arc both give the point: repeated points stop the triangulation.
        Draw::RemoveDuplicates(out);
    }

    void SectorChart(std::span<const ChartSector> sectors, const SectorChartOptions& options) {
        const Metrics::ChartMetrics& metrics = Metrics::Chart();
        const Font& font = options.labelFont;
        std::vector<LegendEntry> legend;
        float total = 0.0f;
        for (size_t i = 0; i < sectors.size(); ++i) {
            total += ImMax(sectors[i].value, 0.0f);
            if (options.showsLegend && sectors[i].name)
                legend.push_back({sectors[i].name, sectors[i].color.a > 0.0f ? sectors[i].color : ChartColor(int(i))});
        }
        const float width = Layout::Proposal().x;
        const float diameter = options.size > 0.0f ? ImMin(Px(options.size), width) : width;
        const float legend_gap = Px(options.legendSpacing > 0.0f ? options.legendSpacing : metrics.legendGap);
        const float legend_height = Legend(nullptr, legend, font, ImVec2(), width, options.legendAlignment);
        const ImRect rect = Layout::Place(ImVec2(width, diameter + (legend.empty() ? 0.0f : legend_gap + legend_height)));
        if (Layout::IsMeasuring())
            return;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const ImVec2 center(rect.GetCenter().x, rect.Min.y + diameter * 0.5f);
        const float outer = diameter * 0.5f;
        const float inner = outer * ImSaturate(options.innerRadius);
        const float inset = Px(options.angularInset);
        const float corner = ImMin(Px(options.cornerRadius), (outer - inner) * 0.5f);
        const float top = -IM_PI * 0.5f;

        // chartAngleSelection: the sector under the pointer, within the ring.
        const ImGuiID id = Layout::NextViewId();
        int selected = options.selection ? *options.selection : -1;
        if (options.selection && total > 0.0f) {
            bool left = false;
            const ImRect square(center - ImVec2(outer, outer), center + ImVec2(outer, outer));
            if (TrackHover(id, square, &left)) {
                const ImVec2 offset = ImGui::GetIO().MousePos - center;
                const float distance = ImSqrt(ImLengthSqr(offset));
                selected = -1;
                if (distance <= outer && distance >= inner) {
                    float angle = std::atan2(offset.y, offset.x) - top;
                    while (angle < 0.0f)
                        angle += 2.0f * IM_PI;
                    float sum = 0.0f;
                    for (size_t i = 0; i < sectors.size() && selected < 0; ++i) {
                        sum += ImMax(sectors[i].value, 0.0f);
                        if (angle <= sum / total * 2.0f * IM_PI)
                            selected = int(i);
                    }
                }
            } else if (left) {
                selected = -1;
            }
            *options.selection = selected;
        }

        ImVector<ImVec2> outline;
        float sum = 0.0f;
        for (size_t i = 0; i < sectors.size() && total > 0.0f; ++i) {
            const float value = ImMax(sectors[i].value, 0.0f);
            const float start = top + sum / total * 2.0f * IM_PI;
            sum += value;
            const float end = top + sum / total * 2.0f * IM_PI;
            Rgba color = sectors[i].color.a > 0.0f ? sectors[i].color : ChartColor(int(i));
            if (selected >= 0 && int(i) != selected)
                color = color.Opacity(metrics.fadedOpacity);
            if (value >= total) {
                if (inner > 0.0f)
                    Draw::Arc(draw, center, (outer + inner) * 0.5f, outer - inner, 0.0f, 2.0f * IM_PI, color);
                else
                    Draw::FillCircle(draw, center, outer, color);
                continue;
            }
            // Sides moved in past each other leave nothing to draw.
            if (end - start <= 2.0f * std::asin(ImMin(inset / outer, 1.0f)))
                continue;
            outline.resize(0);
            SectorOutline(outline, center, outer, inner, start, end, inset, corner);
            draw->AddConcavePolyFilled(outline.Data, outline.Size, color.Packed());
        }

        if (options.center && inner > 0.0f) {
            const float half = inner * 0.70710678f;
            Layout::ContainerSpec spec;
            spec.arrangement = Layout::Arrangement::Overlay;
            spec.alignment = Alignment{HorizontalAlignment::Center, VerticalAlignment::Center};
            spec.fillWidth = true;
            spec.fillHeight = true;
            Layout::Region(id + 1, ImRect(center - ImVec2(half, half), center + ImVec2(half, half)), spec, options.center);
        }
        Legend(draw, legend, font, ImVec2(rect.Min.x, rect.Min.y + diameter + legend_gap), width, options.legendAlignment);
    }
} // namespace Cupertino
