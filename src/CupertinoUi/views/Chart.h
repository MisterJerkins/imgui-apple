#pragma once

#include "core/Color.h"
#include "core/Typography.h"
#include "layout/Layout.h"

#include <functional>
#include <initializer_list>
#include <span>
#include <string>

namespace Cupertino {
    // A value on an axis with its label, like AxisMarks(values:) in Swift Charts; a mark without a label draws only its
    // gridline.
    struct AxisMark {
        float value = 0.0f;
        const char* label = nullptr;
    };

    struct BarChartOptions {
        // Height of the plot in points; the chart takes the offered width.
        float height = 90.0f;
        // Bar slots across the plot, filled by the values from the leading edge; 0 fits the values.
        int slots = 0;
        float maximum = 1.0f;
        // Width of the bars in points, centered in their slots: 3 in the 15-minute Battery Level chart, 11 for hourly bars.
        float barWidth = 3.0f;
        // The bar color; the system green when left clear.
        Rgba color;
        // Gridlines across the plot with labels on the trailing side, in a column at least labelWidth points wide so
        // that charts stacked in one view end their plots together (Battery Level over Screen On Usage).
        std::initializer_list<AxisMark> yMarks;
        float labelWidth = 0.0f;
        // Dashed verticals every so many slots, both edges included, and labels under the plot at slot positions.
        int gridEvery = 0;
        std::initializer_list<AxisMark> xMarks;
        // Where days begin: solid verticals, dated a line under the other labels (Sep 12 in Screen On Usage).
        std::initializer_list<AxisMark> dayMarks;
        // Charging periods in Battery Level: their slots tinted behind the bars down to a capsule under the plot. A chart
        // with a band keeps its row under the plot, charging or not.
        std::span<const bool> band;
    };

    // A bar chart as System Settings draws its Battery charts with Swift Charts.
    void BarChart(std::span<const float> values, const BarChartOptions& options);

    struct AreaChartOptions {
        // Height of the plot in points; the chart takes the offered width.
        float height = 60.0f;
        // The value at the top of the plot; 0 fits the largest value.
        float maximum = 0.0f;
        Rgba color;
        float lineWidth = 1.0f;
        // The area under the line takes the line color at this opacity; 0 leaves it clear.
        float areaOpacity = 0.28f;
        // Strokes the sides and the floor of the area with the line (Activity Monitor's charts).
        bool outlinesArea = false;
        // Points between neighbouring values, the newest on the trailing edge and older ones cut off at the leading
        // edge, like Activity Monitor's history (3 pt); 0 spreads the values across the width.
        float spacing = 0.0f;
    };

    // Swift Charts' LineMark over an AreaMark: values across the width, the stroke inside the frame.
    void AreaChart(std::span<const float> values, const AreaChartOptions& options);

    // The colors foregroundStyle(by:) gives series and sectors in order: the system blue, green, orange, purple, red,
    // cyan and yellow, then again.
    Rgba ChartColor(int index);

    // A series of LineChart: its values' heights and, when given, their places along the x axis.
    struct ChartSeries {
        // Named series are listed in the legend.
        const char* name = nullptr;
        std::span<const float> values;
        // Places along the x axis; empty spaces the values at 0, 1, 2...
        std::span<const float> positions;
        // The palette color of the series' index when left clear.
        Rgba color;
    };

    // RuleMark(y:): a line across the plot at a value, as a threshold or an average.
    struct ChartRule {
        float value = 0.0f;
        // The accent color when left clear.
        Rgba color;
        // An annotation over the line's leading end.
        const char* label = nullptr;
    };

    struct LineChartOptions {
        // Height of the plot in points; the chart takes the offered width.
        float height = 180.0f;
        // The values at the bottom and the top of the plot; a maximum of 0 fits the largest value.
        float minimum = 0.0f;
        float maximum = 0.0f;
        // The places at the plot's leading and trailing edges; equal ones fit the positions.
        float first = 0.0f;
        float last = 0.0f;
        // Gridlines with labels on the trailing side, in a column at least labelWidth points wide.
        std::initializer_list<AxisMark> yMarks;
        float labelWidth = 0.0f;
        // Dashed verticals through the plot and the label row, labels after them.
        std::initializer_list<AxisMark> xMarks;
        // LineMark through the values, PointMark on each.
        bool line = true;
        bool points = false;
        std::span<const ChartRule> rules;
        // chartLegend(alignment:spacing:): where the legend lines up under the chart and how far under the plot and its
        // labels it starts, 0 for the default.
        bool showsLegend = true;
        HorizontalAlignment legendAlignment = HorizontalAlignment::Leading;
        float legendSpacing = 0.0f;
        // AxisValueLabel's font; the legend uses it too.
        Font labelFont = Font::System(11.0f);
        // chartXSelection: the pointer over the plot chooses the nearest value's index, -1 when it leaves. A gray rule
        // marks it with the series' points, under the annotation's text when there is one.
        int* selection = nullptr;
        std::function<std::string(int index)> annotation;
    };

    // Swift Charts' LineMark and PointMark: the series over gridlines, the y labels trailing, the legend under the plot.
    void LineChart(std::span<const ChartSeries> series, const LineChartOptions& options);

    struct ChartSector {
        const char* name = nullptr;
        float value = 0.0f;
        // The palette color of the sector's index when left clear.
        Rgba color;
    };

    struct SectorChartOptions {
        // Diameter in points; 0 takes the offered width.
        float size = 0.0f;
        // innerRadius as a ratio of the radius: 0 for a pie, 0.6 for a donut.
        float innerRadius = 0.0f;
        // angularInset: each sector's sides move in by it, in points; cornerRadius rounds their corners.
        float angularInset = 0.0f;
        float cornerRadius = 0.0f;
        bool showsLegend = true;
        HorizontalAlignment legendAlignment = HorizontalAlignment::Leading;
        float legendSpacing = 0.0f;
        Font labelFont = Font::System(11.0f);
        // chartAngleSelection: the sector under the pointer, -1 for none; the others fade while one is chosen.
        int* selection = nullptr;
        // Views in the middle of a donut, as chartBackground places them.
        std::function<void()> center;
    };

    // Swift Charts' SectorMark: sectors clockwise from 12 o'clock in proportion to their values, the legend under them.
    void SectorChart(std::span<const ChartSector> sectors, const SectorChartOptions& options);
} // namespace Cupertino
