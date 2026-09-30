#pragma once

#include "imgui.h"
#include "imgui_internal.h"

namespace Cupertino {
    // Up and down arrows that change value by step within [min, max], with the label on the leading side (in a form
    // row, at the row's leading edge). A printf format shows the value next to the arrows, like SwiftUI's
    // Stepper(value:in:step:format:). Returns true when the value changes.
    bool Stepper(const char* label, int* value, int min, int max, int step = 1, const char* format = nullptr);
    bool Stepper(const char* label, float* value, float min, float max, float step, const char* format = nullptr);

    // A stepper's arrows at rect (pixels), each half a button that repeats while held, as steppers and date pickers draw
    // them. Returns 1 or -1 on the frames the upper or the lower half steps, 0 otherwise.
    int StepperArrows(ImGuiID id, const ImRect& rect);

    // The outline around a stepper's value and a date picker's field: a point of the separator color inside the frame
    // (Screen Time's Downtime; the kit draws half a point of 3 % outside).
    void DrawValueField(ImDrawList* draw, const ImRect& rect);
} // namespace Cupertino
