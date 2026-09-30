#pragma once

#include "Layout.h"
#include "core/Color.h"

#include <functional>
#include <limits>
#include <string_view>

namespace Cupertino {
    inline constexpr float Infinity = std::numeric_limits<float>::infinity();

    struct VStackOptions {
        HorizontalAlignment alignment = HorizontalAlignment::Center;
        float spacing = 8.0f;
    };

    struct HStackOptions {
        VerticalAlignment alignment = VerticalAlignment::Center;
        float spacing = 8.0f;
    };

    struct ZStackOptions {
        Alignment alignment;
    };

    // .frame(width:height:), .frame(minHeight:) and .frame(maxWidth: Infinity) in points.
    struct FrameOptions {
        float width = 0.0f;
        float height = 0.0f;
        float minWidth = 0.0f;
        float minHeight = 0.0f;
        float maxWidth = 0.0f;
        float maxHeight = 0.0f;
        Alignment alignment;
    };

    void VStack(const VStackOptions& options, const std::function<void()>& content);
    void VStack(const std::function<void()>& content);
    void HStack(const HStackOptions& options, const std::function<void()>& content);
    void HStack(const std::function<void()>& content);
    void ZStack(const ZStackOptions& options, const std::function<void()>& content);
    void ZStack(const std::function<void()>& content);

    // Flexible space along the stack's axis.
    void Spacer(float min_length = 0.0f);

    // A separator line across the stack: horizontal in a VStack, vertical in an HStack, a separator row in a menu. A color
    // replaces the separator color, like Divider().overlay(color).
    void Divider();
    void Divider(Rgba color);

    void Frame(const FrameOptions& options, const std::function<void()>& content);
    void Padding(const EdgeInsets& insets, const std::function<void()>& content);
    void Padding(float length, const std::function<void()>& content);

    // A fixed-size view (points) drawn by a callback with the draw list and its rectangle in pixels. Its first text
    // baseline is its bottom, as with any view without text, or baseline points from its top.
    void Canvas(ImVec2 size, const std::function<void(ImDrawList* draw, const ImRect& rect)>& paint, float baseline = -1.0f);

    // .background { } and .overlay { }: paint fills the content's frame behind or over the content.
    void Background(const std::function<void(ImDrawList* draw, const ImRect& rect)>& paint, const std::function<void()>& content);
    void Overlay(const std::function<void(ImDrawList* draw, const ImRect& rect)>& paint, const std::function<void()>& content);

    // .id(_:): content takes its identity from value, so what it shows for each value keeps its own layout and state (all
    // ImGui keeps by id). When the value changes the content is a new view and its scroll views start at the top, as a
    // detail column does for each pane its sidebar selects.
    void Id(int value, const std::function<void()>& content);
    void Id(std::string_view value, const std::function<void()>& content);
} // namespace Cupertino
