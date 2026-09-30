#pragma once

#include "core/Typography.h"

#include <initializer_list>

namespace Cupertino {
    enum class SliderStyle {
        Linear,
        // AppKit's circular slider: a round bezel with a dot that goes around clockwise from the top.
        Circular,
    };

    struct SliderOptions {
        SliderStyle style = SliderStyle::Linear;
        // Width of the track in points; 0 takes the offered width, in a form row the trailing half of the row.
        float width = 0.0f;
        // Tick marks at evenly spaced values. A slider with ticks gets a bar knob and no fill, as in AppKit, unless filled.
        int ticks = 0;
        bool filled = false;
        // Tick marks at these fractions of the range instead, for sliders whose first step is special (Magnification).
        std::initializer_list<float> tickValues;
        // Only tick values can be picked (AppKit's allowsTickMarkValuesOnly).
        bool snapsToTicks = false;
        // SF Symbols on both sides of the track, like SwiftUI's minimumValueLabel and maximumValueLabel.
        unsigned minimumSymbol = 0;
        unsigned maximumSymbol = 0;
        // Their scale, like .imageScale(): Brightness in Displays shows its suns large.
        SymbolScale symbolScale = SymbolScale::Medium;
        // Captions under the track from its start to its end (System Settings' "Small ... Large"); with as many
        // captions as ticks, each one sits at its tick.
        std::initializer_list<const char*> captions;
    };

    // A linear slider over [min, max] with its label on the leading side; in a form row the label is the row's.
    // Returns true while the value changes.
    bool Slider(const char* label, float* value, float min, float max, const SliderOptions& options = {});
} // namespace Cupertino
