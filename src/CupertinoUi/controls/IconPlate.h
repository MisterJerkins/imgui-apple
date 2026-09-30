#pragma once

#include "core/Bitmap.h"
#include "core/Color.h"
#include "core/Typography.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace Cupertino {
    // Rounded-square backgrounds behind white symbols in System Settings (the kit's Icon Background set).
    enum class IconPlate {
        None,
        Black,
        Blue,
        Cyan,
        DarkGray,
        Gray,
        Green,
        Orange,
        Pink,
        Purple,
        Red,
        White,
        Yellow,
    };

    struct Icon {
        unsigned symbol = 0;
        IconPlate plate = IconPlate::None;
        // System Settings draws most plate symbols at the large image scale, some of them smaller than the plate's usual
        // glyph (Sound's speaker at 0.78 of it, @2x sidebars).
        SymbolScale scale = SymbolScale::Large;
        float symbolSize = 1.0f;
        // Draws a glyph SF Symbols does not have (such as the Bluetooth rune) in the symbol frame instead.
        void (*paint)(ImDrawList* draw, const ImRect& frame, Rgba color) = nullptr;
        // A picture in place of the plate and the symbol, as an app's own artwork (Siri, Game Center).
        Bitmap image;
        // The symbol's color when not the plate's white or, without a plate, the accent (Finder's tag dots).
        Rgba color;

        bool IsEmpty() const {
            return !symbol && plate == IconPlate::None && !paint && image.IsEmpty();
        }
    };

    // Draws the plate with its vertical gradient and the symbol centered on it, scaled from the kit's 40 pt master to the
    // square in the middle of frame; a picture fills frame.
    void DrawIcon(ImDrawList* draw, const ImRect& frame, const Icon& icon);

    // A person's picture cut to a circle, or their initials on the blue disc macOS draws without one (the account row of
    // System Settings: 17 pt Medium on 38 pt).
    void DrawAvatar(ImDrawList* draw, const ImRect& rect, const char* initials, const Bitmap& picture = {});

    // The blue Finder folder filling rect (File Sharing @2x, 14 x 13 pt).
    void DrawFolder(ImDrawList* draw, const ImRect& rect);
    // Icon::paint for a folder in a 16 pt icon frame, a point right of its middle (path controls, lists).
    void PaintFolderIcon(ImDrawList* draw, const ImRect& frame, Rgba color);
} // namespace Cupertino
