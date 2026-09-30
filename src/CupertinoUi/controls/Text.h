#pragma once

#include "IconPlate.h"

#include "core/Bitmap.h"
#include "core/Color.h"
#include "core/Typography.h"

#include <string_view>

namespace Cupertino {
    // Semantic foreground colors, like .foregroundStyle(.secondary).
    enum class Foreground {
        Primary,
        Secondary,
        Tertiary,
        Quaternary,
        Accent,
        White,
        Custom,
    };

    struct TextOptions {
        Font font = Font::Style(TextStyle::Body);
        Foreground foreground = Foreground::Primary;
        Rgba color;
        TextAlignment alignment = TextAlignment::Leading;
        // Wraps to the offered width; one line otherwise, truncated with an ellipsis when it does not fit.
        bool wraps = false;
        TruncationMode truncation = TruncationMode::Tail;
    };

    struct ImageOptions {
        Font font = Font::Style(TextStyle::Body);
        Foreground foreground = Foreground::Primary;
        Rgba color;
    };

    // Resolves a semantic foreground against the current palette.
    Rgba ForegroundColor(Foreground foreground, Rgba custom = Rgba());

    void Text(std::string_view text, const TextOptions& options = {});

    // An SF Symbol sized and weighted like text of the given font (Image(systemName:)).
    void Image(unsigned symbol, const ImageOptions& options = {});

    // A bitmap at its own size, or stretched to size in points (Image(nsImage:) made resizable in a frame).
    void Image(const Bitmap& bitmap, ImVec2 size = ImVec2(0.0f, 0.0f));
    // An icon (a symbol on its plate, a painted glyph or a picture) in a frame of size points.
    void Image(const Icon& icon, ImVec2 size);
} // namespace Cupertino
