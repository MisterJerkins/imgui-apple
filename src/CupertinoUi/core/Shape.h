#pragma once

#include "Color.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace Cupertino {
    // Corner radii clockwise from the top-left corner.
    struct CornerRadii {
        float topLeft = 0.0f;
        float topRight = 0.0f;
        float bottomRight = 0.0f;
        float bottomLeft = 0.0f;

        constexpr CornerRadii() = default;

        constexpr CornerRadii(float all) : topLeft(all), topRight(all), bottomRight(all), bottomLeft(all) {
        }

        constexpr CornerRadii(float top_left, float top_right, float bottom_right, float bottom_left) : topLeft(top_left), topRight(top_right), bottomRight(bottom_right), bottomLeft(bottom_left) {
        }

        // Radii of the same shape grown (or shrunk) by delta; corners never go below zero.
        CornerRadii Offset(float delta) const {
            return CornerRadii(ImMax(topLeft + delta, 0.0f), ImMax(topRight + delta, 0.0f), ImMax(bottomRight + delta, 0.0f), ImMax(bottomLeft + delta, 0.0f));
        }

        float Largest() const {
            return ImMax(ImMax(topLeft, topRight), ImMax(bottomRight, bottomLeft));
        }
    };

    // Apple's continuous (squircle) corners or plain circular arcs; capsules and circles use circular arcs.
    enum class CornerStyle {
        Continuous,
        Circular,
    };

    // Where a stroke sits relative to the outline, as in Sketch.
    enum class StrokeAlignment {
        Inside,
        Center,
        Outside,
    };

    // A drop or inner shadow in Sketch terms, in points: offset, blur radius (sigma is half of it) and spread.
    struct Shadow {
        Rgba color;
        ImVec2 offset;
        float blur = 0.0f;
        float spread = 0.0f;
    };
} // namespace Cupertino
