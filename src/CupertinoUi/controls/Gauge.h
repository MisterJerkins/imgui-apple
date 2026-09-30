#pragma once

#include "core/Color.h"

namespace Cupertino {
    enum class GaugeStyle {
        // The default: a thick capsule filling from its leading end, the label centered over it, the current value
        // under it and the bounds at its ends.
        LinearCapacity,
        // A thin capsule, the label over its leading end and the current value under it, small and secondary.
        AccessoryLinearCapacity,
        // A bar with a dot at the value cut out of it.
        AccessoryLinear,
        // An open ring with a dot at the value, the current value inside and the label in its opening.
        AccessoryCircular,
        // A closed ring filling clockwise from the top around the current value.
        AccessoryCircularCapacity,
    };

    struct GaugeOptions {
        GaugeStyle style = GaugeStyle::LinearCapacity;
        float minimum = 0.0f;
        float maximum = 1.0f;
        const char* currentValueLabel = nullptr;
        const char* minimumValueLabel = nullptr;
        const char* maximumValueLabel = nullptr;
        // The fill; clear takes the accent in the capacity styles and the label color in the others.
        Rgba tint;
        // Width of the linear styles in points; 0 takes the offered width.
        float width = 0.0f;
    };

    // SwiftUI's Gauge: a value within bounds, drawn by style. Measured on SwiftUI's own renders (iOS 17, 1 px/pt): the
    // shapes keep their sizes in points and the labels take the platform's text styles.
    void Gauge(const char* label, float value, const GaugeOptions& options = {});
} // namespace Cupertino
