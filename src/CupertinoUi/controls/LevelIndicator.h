#pragma once

namespace Cupertino {
    enum class LevelIndicatorStyle {
        // A bar filled up to the value (NSLevelIndicator's continuous capacity).
        ContinuousCapacity,
        // A row of cells lit up to the value, one per unit between minimum and maximum.
        DiscreteCapacity,
        // Stars up to the value, one per unit.
        Rating,
    };

    struct LevelIndicatorOptions {
        LevelIndicatorStyle style = LevelIndicatorStyle::ContinuousCapacity;
        float minimum = 0.0f;
        float maximum = 1.0f;
        // Values from warning on draw yellow and from critical red; with critical under warning, low values are the bad
        // ones. Equal values (0 and 0) keep the green.
        float warning = 0.0f;
        float critical = 0.0f;
        // Each part of the bar in its own tier's color instead of the whole bar in the value's.
        bool tiered = false;
        // Length of a bar in points; 0 takes the offered width.
        float width = 0.0f;
    };

    // NSLevelIndicator: a capacity bar, cells or a rating, with the label on the leading side (in a form row, at the row's
    // leading edge).
    void LevelIndicator(const char* label, float value, const LevelIndicatorOptions& options = {});
    // An editable one, as AppKit's isEditable: a click or a drag sets the value to the cell or star under the pointer.
    bool LevelIndicator(const char* label, float* value, const LevelIndicatorOptions& options = {});
} // namespace Cupertino
