#pragma once

#include "Color.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Cupertino {
    enum class FontWeight {
        Regular,
        Medium,
        Semibold,
        Bold,
        Heavy,
    };

    // SwiftUI text styles; sizes and leading differ per platform (macOS Body is 13/16, iOS Body is 17/22).
    enum class TextStyle {
        LargeTitle,
        Title,
        Title2,
        Title3,
        Headline,
        Subheadline,
        Body,
        Callout,
        Footnote,
        Caption,
        Caption2,
    };

    enum class TextAlignment {
        Leading,
        Center,
        Trailing,
    };

    // Where text too long for its line gives up characters to an ellipsis (Text.TruncationMode): at its end, or in its
    // middle, as Finder shortens file names (the copy window @2x: "Pippin - Apple's F…e Console.mov").
    enum class TruncationMode {
        Tail,
        Middle,
    };

    // SF Symbols image scale (.imageScale): the same symbol drawn smaller or larger at one point size.
    enum class SymbolScale {
        Small,
        Medium,
        Large,
    };

    // How the layers of a symbol take its color (.symbolRenderingMode): all alike, or the secondary layers at half its
    // opacity (the outline of a battery around its level).
    enum class SymbolRendering {
        Monochrome,
        Hierarchical,
    };

    // A system font: point size, weight and the height of one line in points.
    struct Font {
        float size = 13.0f;
        FontWeight weight = FontWeight::Regular;
        float lineHeight = 16.0f;
        SymbolScale symbolScale = SymbolScale::Medium;
        // Every digit as wide as a zero, like .monospacedDigit(): chart axes and counters keep their columns.
        bool monospacedDigits = false;
        // SF Mono, like .monospaced(): every character as wide as the others, for code and columns of text.
        bool monospaced = false;
        // Drawn the way a picture is (sidebar icons ship as images), without the heavier strokes of live text.
        bool picture = false;
        SymbolRendering symbolRendering = SymbolRendering::Monochrome;
        // Image(systemName:variableValue:): a symbol with variable layers lights those the value (0...1) reaches and dims
        // the rest (the arcs of a weak Wi-Fi); negative draws it whole.
        float variableValue = -1.0f;

        static Font System(float size, FontWeight weight = FontWeight::Regular);
        static Font Style(TextStyle style);

        Font Weight(FontWeight new_weight) const;
        Font WithLineHeight(float height) const;
        Font MonospacedDigit() const;
        Font Monospaced() const;
        Font ImageScale(SymbolScale scale) const;
        Font AsPicture() const;
        Font SymbolRenderingMode(SymbolRendering rendering) const;
        Font VariableValue(float value) const;
    };

    // Text output with SF Pro's trak tracking and GPOS pair kerning, which ImGui's AddText does not apply.
    // Geometry is in pixels at the current environment scale.
    namespace Typography {
        // Loads SF Pro Text and Display, and SF Mono, the small and large symbol fonts and the symbol layers from
        // directory/derived when present.
        bool LoadFonts(const std::string& directory);

        // CoreText tracking of SF Pro at a point size, in points.
        float Tracking(float size);

        float Width(const Font& font, std::string_view text);
        // A null text is empty.
        float Width(const Font& font, const char* text);

        // UTF-8: whether a byte continues a codepoint, how many codepoints text has, and the byte where codepoint index
        // starts (text's size past its end).
        bool IsContinuationByte(char c);
        int CodepointCount(std::string_view text);
        size_t CodepointOffset(std::string_view text, int index);
        // The widest of texts, in pixels.
        float WidestWidth(const Font& font, std::span<const char* const> texts);

        // Pen positions of a line in pixels from its start: one per codepoint (where its glyph starts, after kerning)
        // and a last one at the line's width. Text editing places carets and selections on them.
        void Offsets(const Font& font, std::string_view text, std::vector<float>& offsets);
        // Wrapped text may start its first line first_indent pixels in, after an inline symbol.
        ImVec2 Measure(const Font& font, std::string_view text, float wrap_width = 0.0f, float first_indent = 0.0f);

        // Pixels from the top of a line box to the baseline.
        float Baseline(const Font& font);
        // Height of SF Pro's capitals in pixels, 1443 of 2048 units: marks centered on a line of text center on it.
        float CapHeight(const Font& font);
        // Pixels from the top of a box of the given height (pixels) to the baseline of a line centered in it, as Draw
        // places it; controls report it so that rows can align them on their text.
        float CenteredBaseline(const Font& font, float height);

        // Draws one line vertically centered in rect, the way AppKit centers a label in its cell.
        void Draw(ImDrawList* draw, const Font& font, const ImRect& rect, Rgba color, std::string_view text, TextAlignment alignment = TextAlignment::Leading);

        // Draws text wrapped to the rect width, starting at its top. Both drawing functions show Markdown links
        // ("[title](target)") as their title in the link color, as SwiftUI's Text does.
        void DrawWrapped(ImDrawList* draw, const Font& font, const ImRect& rect, Rgba color, std::string_view text, TextAlignment alignment = TextAlignment::Leading, float first_indent = 0.0f);

        // A range of a text (bytes) drawn in its own color, as search matches stand out in System Settings' sidebar.
        struct ColorSpan {
            size_t begin = 0;
            size_t end = 0;
            Rgba color;
        };

        // Draws text wrapped to the rect width from its top, its spans (in order, apart) in their colors.
        void DrawWrapped(ImDrawList* draw, const Font& font, const ImRect& rect, Rgba color, std::string_view text, std::span<const ColorSpan> spans);

        // Cuts text with an ellipsis at its end, or in its middle, so that it fits max_width pixels.
        std::string Truncate(const Font& font, std::string_view text, float max_width, TruncationMode mode = TruncationMode::Tail);

        // Codepoints drawn so far that no loaded font has, drawn as ImGui's fallback glyph instead: text or a symbol the
        // baked subset leaves out (tools/bake.py keeps the symbols the sources name).
        std::span<const unsigned> MissingGlyphs();

        // The ImGui font and pixel size behind a font, to set plain ImGui widgets in it (ImGui::PushFont).
        ImFont* ImGuiFont(const Font& font);
        float PixelSize(const Font& font);

        // SF Symbols are glyphs of SF Pro, drawn with the weight and size of the given font.
        std::string SymbolText(unsigned symbol);
        float SymbolWidth(unsigned symbol, const Font& font);
        void DrawSymbol(ImDrawList* draw, unsigned symbol, const Font& font, const ImRect& rect, Rgba color, TextAlignment alignment = TextAlignment::Center);
    } // namespace Typography
} // namespace Cupertino
