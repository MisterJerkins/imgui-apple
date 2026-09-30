#pragma once

#include "Bitmap.h"
#include "Color.h"
#include "Shape.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <functional>
#include <span>

// Drawing primitives shared by every view. Geometry is in pixels; shadows are Sketch values in points.
namespace Cupertino::Draw {
    // The unit vector along v; zero stays zero.
    ImVec2 Normalized(ImVec2 v);
    // How many straight pieces an arc of the given sweep in radians takes: 96 to a full turn keeps the chords within a
    // tenth of a pixel of the circle; at least minimum.
    int ArcSegments(float sweep, int minimum);
    // Drops consecutive points closer than a hundredth of a pixel, and a last one on the first: ImGui's anti-aliasing
    // needs edges of some length.
    void RemoveDuplicates(ImVector<ImVec2>& points);
    // The triangle whose sides lie distance outside those of corners (inside when negative), each corner moved along its
    // bisector (the rim of the caution sign).
    void OffsetTriangle(const ImVec2 (&corners)[3], float distance, ImVec2 (&out)[3]);

    // Outline of a rounded rectangle, clockwise from the top-left corner, with exactly corner_points vertices per corner.
    void RoundedRectContour(ImVector<ImVec2>& out, const ImRect& rect, const CornerRadii& radii, CornerStyle style, int corner_points);
    // How far along each edge a corner of this radius curves: about 1.53 radius when continuous. A rectangle takes it only
    // when at least twice this on each side; a smaller one gets a circular corner instead.
    float CornerReach(float radius, CornerStyle style = CornerStyle::Continuous);

    void FillRect(ImDrawList* draw, const ImRect& rect, Rgba color);
    void FillRoundedRect(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, Rgba color, CornerStyle style = CornerStyle::Continuous);
    void StrokeRoundedRect(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, Rgba color, float thickness, StrokeAlignment alignment = StrokeAlignment::Inside, CornerStyle style = CornerStyle::Continuous);

    // Linear gradient from the top edge of rect to its bottom edge, alpha included.
    void FillVerticalGradient(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, Rgba top, Rgba bottom, CornerStyle style = CornerStyle::Continuous);
    // A triangle with each corner rounded by the circle that touches both of its sides, shaded from its top to its
    // bottom (the caution sign of critical alerts, the arrows of the gesture videos).
    void FillRoundedTriangle(ImDrawList* draw, const ImVec2 (&corners)[3], const float (&radii)[3], Rgba top, Rgba bottom);

    void FillCircle(ImDrawList* draw, ImVec2 center, float radius, Rgba color);
    // A capsule over rect's height from its left edge, at least as long as it is tall so that its ends stay round.
    void FillCapsule(ImDrawList* draw, const ImRect& rect, Rgba color);

    // A bitmap stretched over rect with circular corners, at the given opacity.
    void Image(ImDrawList* draw, const ImRect& rect, const Bitmap& bitmap, float radius = 0.0f, float opacity = 1.0f);
    void StrokeCircle(ImDrawList* draw, ImVec2 center, float radius, Rgba color, float thickness, StrokeAlignment alignment = StrokeAlignment::Inside);

    // Gaussian shadows built as a mesh of contours with per-vertex alpha, so they stay exact at any size and corner.
    // opacity scales every shadow of a stack, for cross-fading between control states. cut_out leaves the area under the
    // shape empty, like a layer group faded as a whole: a half-transparent control does not show its shadow through.
    void DropShadow(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, const Shadow& shadow, CornerStyle style = CornerStyle::Continuous, bool cut_out = false);
    void DropShadows(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, std::span<const Shadow> shadows, CornerStyle style = CornerStyle::Continuous, float opacity = 1.0f, bool cut_out = false);
    void InnerShadow(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, const Shadow& shadow, CornerStyle style = CornerStyle::Continuous);
    void InnerShadows(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, std::span<const Shadow> shadows, CornerStyle style = CornerStyle::Continuous, float opacity = 1.0f);

    // Axis-aligned lines snapped to device pixels; thinner than a pixel means a one-pixel line with reduced alpha.
    void HorizontalLine(ImDrawList* draw, float x0, float x1, float y, float thickness, Rgba color);
    void VerticalLine(ImDrawList* draw, float x, float y0, float y1, float thickness, Rgba color);

    // A convex shape filled as a fan from center, each contour point in its own color blended across the triangles
    // (angular and diagonal gradients). No antialiasing: it goes over a solid fill of the same shape.
    void FillShaded(ImDrawList* draw, ImVec2 center, Rgba center_color, std::span<const ImVec2> contour, const std::function<Rgba(ImVec2)>& color);

    // Joined line segments with miter joins, bevelled where CoreGraphics bevels (past a miter limit of 10): a spike ends
    // at its point instead of running out in a long miter.
    void Polyline(ImDrawList* draw, std::span<const ImVec2> points, Rgba color, float thickness, bool closed = false);

    // A band along a circle of the given radius, from start to end in radians clockwise from twelve o'clock, each end
    // with a round cap or square; a full turn draws a ring.
    void Arc(ImDrawList* draw, ImVec2 center, float radius, float thickness, float start, float end, Rgba color, bool round_start = true, bool round_end = true);

    // Turns everything drawn since first_vertex (draw->VtxBuffer.Size before drawing) around center, clockwise.
    void RotateVertices(ImDrawList* draw, int first_vertex, ImVec2 center, float radians);

    // Mixes the colors of everything drawn since first_vertex toward color by amount, keeping their alpha: the same as
    // covering it with color at that opacity.
    void TintVertices(ImDrawList* draw, int first_vertex, Rgba color, float amount);

    // Scales the alpha of everything drawn since first_vertex by opacity at each vertex's position (a fading tail).
    void FadeVertices(ImDrawList* draw, int first_vertex, const std::function<float(ImVec2)>& opacity);

    // Recolors everything drawn since first_vertex by position, keeping each vertex's alpha: a gradient across a glyph
    // (one quad) or a shape drawn in a single color.
    void ShadeVertices(ImDrawList* draw, int first_vertex, const std::function<Rgba(ImVec2)>& color);

    // Draws content as one layer at the given opacity over a solid backdrop, like a faded layer group: its shapes do not
    // show through each other (an icon plate under its symbol). Content draws opaque, then blends toward the backdrop.
    void FadedGroup(ImDrawList* draw, Rgba backdrop, float opacity, const std::function<void()>& content);

    // Lowers the opacity of everything drawn until the scope ends, for cross-fading two looks of a control.
    class Opacity {
    public:
        explicit Opacity(float opacity);
        ~Opacity();
    };

    // Keyboard focus ring: 3.5 pt wide in the accent color at half opacity, covering the control's outer half point
    // (Connect to Server and Calendar alert @2x); it fades out with the window's key state.
    void FocusRing(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, Rgba accent, CornerStyle style = CornerStyle::Continuous);

    // Rounds a pixel coordinate to the device pixel grid.
    float Snap(float value);
    ImVec2 Snap(ImVec2 point);
    ImRect Snap(const ImRect& rect);
} // namespace Cupertino::Draw
