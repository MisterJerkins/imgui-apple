#pragma once

namespace Cupertino::Kerning {
    // One pair-kerning table per static SF Pro face, generated from GPOS by tools/fonts/kerning.py.
    enum class Table {
        None,
        TextRegular,
        TextMedium,
        TextSemibold,
        TextBold,
        TextHeavy,
        DisplayRegular,
        DisplayMedium,
        DisplaySemibold,
        DisplayBold,
    };

    inline constexpr float UnitsPerEm = 2048.0f;

    // Adjustment of the left glyph's advance in font units; 0 when the pair is not kerned.
    int Adjustment(Table table, unsigned left, unsigned right);
} // namespace Cupertino::Kerning
