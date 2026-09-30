#include "Typography.h"

#include "BakedFiles.h"
#include "Draw.h"
#include "Environment.h"
#include "Kerning.h"
#include "Theme.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <memory>
#include <vector>

namespace Cupertino {
    // SF Pro hhea metrics in em: ascender 1950 and descender 494 of 2048 units.
    static constexpr float Ascender = 1950.0f / 2048.0f;
    static constexpr float Descender = 494.0f / 2048.0f;

    Font Font::System(float size, FontWeight weight) {
        return Font{.size = size, .weight = weight, .lineHeight = ImCeil(size * (Ascender + Descender))};
    }

    static Font StyleFont(float size, FontWeight weight, float line_height) {
        return Font{.size = size, .weight = weight, .lineHeight = line_height};
    }

    Font Font::Style(TextStyle style) {
        if (Environment().platform == Platform::IOS) {
            switch (style) {
                case TextStyle::LargeTitle:
                    return StyleFont(34.0f, FontWeight::Regular, 41.0f);
                case TextStyle::Title:
                    return StyleFont(28.0f, FontWeight::Regular, 34.0f);
                case TextStyle::Title2:
                    return StyleFont(22.0f, FontWeight::Regular, 28.0f);
                case TextStyle::Title3:
                    return StyleFont(20.0f, FontWeight::Regular, 25.0f);
                case TextStyle::Headline:
                    return StyleFont(17.0f, FontWeight::Semibold, 22.0f);
                case TextStyle::Subheadline:
                    return StyleFont(15.0f, FontWeight::Regular, 20.0f);
                case TextStyle::Body:
                    return StyleFont(17.0f, FontWeight::Regular, 22.0f);
                case TextStyle::Callout:
                    return StyleFont(16.0f, FontWeight::Regular, 21.0f);
                case TextStyle::Footnote:
                    return StyleFont(13.0f, FontWeight::Regular, 18.0f);
                case TextStyle::Caption:
                    return StyleFont(12.0f, FontWeight::Regular, 16.0f);
                case TextStyle::Caption2:
                    return StyleFont(11.0f, FontWeight::Regular, 13.0f);
            }
        }
        switch (style) {
            case TextStyle::LargeTitle:
                return StyleFont(26.0f, FontWeight::Regular, 32.0f);
            case TextStyle::Title:
                return StyleFont(22.0f, FontWeight::Regular, 26.0f);
            case TextStyle::Title2:
                return StyleFont(17.0f, FontWeight::Regular, 22.0f);
            case TextStyle::Title3:
                return StyleFont(15.0f, FontWeight::Regular, 20.0f);
            case TextStyle::Headline:
                return StyleFont(13.0f, FontWeight::Bold, 16.0f);
            case TextStyle::Subheadline:
                return StyleFont(11.0f, FontWeight::Regular, 14.0f);
            case TextStyle::Body:
                return StyleFont(13.0f, FontWeight::Regular, 16.0f);
            case TextStyle::Callout:
                return StyleFont(12.0f, FontWeight::Regular, 15.0f);
            case TextStyle::Footnote:
                return StyleFont(10.0f, FontWeight::Regular, 13.0f);
            case TextStyle::Caption:
                return StyleFont(10.0f, FontWeight::Regular, 13.0f);
            case TextStyle::Caption2:
                return StyleFont(10.0f, FontWeight::Medium, 13.0f);
        }
        return StyleFont(13.0f, FontWeight::Regular, 16.0f);
    }

    Font Font::Weight(FontWeight new_weight) const {
        Font font = *this;
        font.weight = new_weight;
        return font;
    }

    Font Font::WithLineHeight(float height) const {
        Font font = *this;
        font.lineHeight = height;
        return font;
    }

    Font Font::MonospacedDigit() const {
        Font font = *this;
        font.monospacedDigits = true;
        return font;
    }

    Font Font::Monospaced() const {
        Font font = *this;
        font.monospaced = true;
        return font;
    }

    Font Font::ImageScale(SymbolScale scale) const {
        Font font = *this;
        font.symbolScale = scale;
        return font;
    }

    Font Font::AsPicture() const {
        Font font = *this;
        font.picture = true;
        return font;
    }

    Font Font::SymbolRenderingMode(SymbolRendering rendering) const {
        Font font = *this;
        font.symbolRendering = rendering;
        return font;
    }

    Font Font::VariableValue(float value) const {
        Font font = *this;
        font.variableValue = value;
        return font;
    }

    namespace Typography {
        struct Face {
            ImFont* font = nullptr;
            Kerning::Table kerning = Kerning::Table::None;
        };

        // How strongly glyph edges are filled in. CoreText draws text heavier than its outlines, light text on a dark
        // ground heavier still (dark Appearance @2x: 12-19% more ink than dark text); pictures such as the sidebar icons
        // keep the weight of their outlines.
        enum class Rendering {
            Text,
            LightText,
            Picture,
        };
        static constexpr int RenderingCount = 3;
        static constexpr float RasterizerMultiplies[RenderingCount] = {1.45f, 2.6f, 1.25f};

        // SF Pro Text below 20 pt and SF Pro Display from 20 pt, like the system font's optical sizes.
        static Face TextFaces[RenderingCount][5];
        static Face DisplayFaces[RenderingCount][5];
        // SF Mono, without kerning or tracking; text in it falls back to SF Pro while it is not loaded.
        static Face MonoFaces[RenderingCount][5];
        // Symbol-only fonts for the small and large image scales (tools/fonts/symbol_scales.py).
        static Face SmallSymbolFaces[RenderingCount][5];
        static Face LargeSymbolFaces[RenderingCount][5];
        // The layers of the symbols macOS draws in layers (tools/fonts/symbol_layers.py): each symbol's layers at every
        // scale have codepoints of their own, from its first one on.
        static Face LayerFaces[RenderingCount][5];

        struct LayeredSymbol {
            unsigned symbol;
            unsigned first;
            int count;
            int levels[4];
        };

        static const LayeredSymbol LayeredSymbols[] = {
#include "generated/SymbolLayers.inc"
        };

        // Secondary layers keep half the color's opacity, layers a variable value does not reach 30% (Bluetooth and Wi-Fi
        // @2x).
        static constexpr float SecondaryLayerOpacity = 0.5f;
        static constexpr float DimmedLayerOpacity = 0.3f;

        // Glyph run parameters for one font at the current scale.
        struct Run {
            ImFontBaked* baked = nullptr;
            float scale = 1.0f;
            float em = 0.0f;
            float tracking = 0.0f;
            Kerning::Table kerning = Kerning::Table::None;
            // With monospaced digits every digit advances by the zero's width, its glyph centered in it.
            float digitAdvance = 0.0f;
        };

        // A font file by its path in the fonts folder: baked into the library when no folder is given, or read from the
        // folder once and kept for the atlas, which renders it once per rendering. Empty when there is none.
        static std::span<const unsigned char> FontFile(const std::string& directory, const std::string& name) {
            if (directory.empty())
                return FindBakedFile(BakedFonts(), name);
            static std::vector<std::unique_ptr<std::vector<unsigned char>>> files;
            // In one read: the fonts are megabytes each.
            std::ifstream stream(directory + "/" + name, std::ios::binary | std::ios::ate);
            if (!stream)
                return {};
            const std::streamsize size = stream.tellg();
            stream.seekg(0);
            auto data = std::make_unique<std::vector<unsigned char>>(size_t(size));
            if (!stream.read(reinterpret_cast<char*>(data->data()), size))
                return {};
            files.push_back(std::move(data));
            return *files.back();
        }

        bool LoadFonts(const std::string& directory) {
            static const char* const WeightNames[] = {"Regular", "Medium", "Semibold", "Bold", "Heavy"};
            static const Kerning::Table TextTables[] = {Kerning::Table::TextRegular, Kerning::Table::TextMedium, Kerning::Table::TextSemibold, Kerning::Table::TextBold, Kerning::Table::TextHeavy};
            static const Kerning::Table DisplayTables[] = {Kerning::Table::DisplayRegular, Kerning::Table::DisplayMedium, Kerning::Table::DisplaySemibold, Kerning::Table::DisplayBold, Kerning::Table::None};

            // Apple sizes are em sizes while ImGui sizes span ascender + descender, so glyphs are scaled up to match.
            // CoreText draws stems a little heavier than their outlines (16% more ink on System Settings @2x);
            // strengthening partial coverage brings the weight and the share of dark pixels within 2% of it.
            ImFontConfig config;
            config.ExtraSizeScale = Ascender + Descender;
            config.FontDataOwnedByAtlas = false;
            ImFontAtlas* atlas = ImGui::GetIO().Fonts;
            // The atlas only reads the files, baked ones included.
            const auto add = [&](std::span<const unsigned char> file, int rendering) {
                config.RasterizerMultiply = RasterizerMultiplies[rendering];
                return file.empty() ? nullptr : atlas->AddFontFromMemoryTTF(const_cast<unsigned char*>(file.data()), int(file.size()), 0.0f, &config);
            };
            for (int weight = 0; weight < 5; ++weight) {
                const std::string suffix = std::string("-") + WeightNames[weight] + ".otf";
                const std::span<const unsigned char> text = FontFile(directory, "SF-Pro-Text" + suffix);
                const std::span<const unsigned char> display = FontFile(directory, "SF-Pro-Display" + suffix);
                if (text.empty() || display.empty())
                    return false;
                const std::span<const unsigned char> mono = FontFile(directory, "SF-Mono" + suffix);
                const std::span<const unsigned char> small = FontFile(directory, "derived/SF-Symbols-Small" + suffix);
                const std::span<const unsigned char> large = FontFile(directory, "derived/SF-Symbols-Large" + suffix);
                const std::span<const unsigned char> layers = FontFile(directory, "derived/SF-Symbols-Layers" + suffix);
                for (int rendering = 0; rendering < RenderingCount; ++rendering) {
                    TextFaces[rendering][weight] = Face{add(text, rendering), TextTables[weight]};
                    DisplayFaces[rendering][weight] = Face{add(display, rendering), DisplayTables[weight]};
                    MonoFaces[rendering][weight].font = add(mono, rendering);
                    SmallSymbolFaces[rendering][weight].font = add(small, rendering);
                    LargeSymbolFaces[rendering][weight].font = add(large, rendering);
                    LayerFaces[rendering][weight].font = add(layers, rendering);
                }
            }
            return true;
        }

        float Tracking(float size) {
            // trak table of the variable SF-Pro.ttf, track 0: point size and tracking in 1/1000 em.
            static const ImVec2 Table[] = {{6.0f, 40.039f}, {9.0f, 18.555f}, {10.0f, 11.719f}, {11.0f, 5.859f}, {12.0f, 0.0f}, {13.0f, -5.859f}, {14.0f, -10.742f}, {15.0f, -15.625f}, {16.0f, -19.531f}, {17.0f, -25.391f}, {20.0f, -22.461f}, {22.0f, -11.719f}, {24.0f, 2.93f}, {28.0f, 13.672f}, {32.0f, 12.695f}, {36.0f, 10.254f}, {50.0f, 6.836f}, {64.0f, 3.418f}, {80.0f, 0.0f}};
            const int count = IM_ARRAYSIZE(Table);
            float milli_em = Table[count - 1].y;
            if (size <= Table[0].x) {
                milli_em = Table[0].y;
            } else {
                for (int i = 1; i < count; ++i) {
                    if (size <= Table[i].x) {
                        const float t = (size - Table[i - 1].x) / (Table[i].x - Table[i - 1].x);
                        milli_em = ImLerp(Table[i - 1].y, Table[i].y, t);
                        break;
                    }
                }
            }
            return milli_em * size / 1000.0f;
        }

        // Light text gets the heavier rendering; a font drawn as a picture keeps its outlines.
        static Rendering RenderingFor(const Font& font, Rgba color) {
            if (font.picture)
                return Rendering::Picture;
            return 0.2126f * color.r + 0.7152f * color.g + 0.0722f * color.b > 0.5f ? Rendering::LightText : Rendering::Text;
        }

        static const Face& FaceFor(const Font& font, Rendering rendering = Rendering::Text) {
            const int weight = ImClamp(int(font.weight), 0, 4);
            if (font.monospaced && MonoFaces[int(rendering)][weight].font)
                return MonoFaces[int(rendering)][weight];
            return font.size >= 20.0f ? DisplayFaces[int(rendering)][weight] : TextFaces[int(rendering)][weight];
        }

        // Medium symbols come from the text faces; the other scales from the derived fonts when they are loaded.
        static const Face& SymbolFaceFor(const Font& font, Rendering rendering = Rendering::Text) {
            const int weight = ImClamp(int(font.weight), 0, 4);
            if (font.symbolScale == SymbolScale::Small && SmallSymbolFaces[int(rendering)][weight].font)
                return SmallSymbolFaces[int(rendering)][weight];
            if (font.symbolScale == SymbolScale::Large && LargeSymbolFaces[int(rendering)][weight].font)
                return LargeSymbolFaces[int(rendering)][weight];
            // SF Mono has no symbols.
            Font text = font;
            text.monospaced = false;
            return FaceFor(text, rendering);
        }

        static Run RunFor(const Font& font, const Face& face, bool tracked) {
            IM_ASSERT(face.font && "Typography::LoadFonts() must be called first");
            const float scale = Environment().Scale();
            Run run;
            run.em = font.size * scale;
            run.baked = face.font->GetFontBaked(run.em);
            run.scale = run.em / run.baked->Size;
            run.tracking = tracked && !font.monospaced ? Tracking(font.size) * scale : 0.0f;
            run.kerning = face.kerning;
            if (font.monospacedDigits)
                run.digitAdvance = run.baked->GetCharAdvance(ImWchar('0')) * run.scale;
            return run;
        }

        static Run RunFor(const Font& font) {
            return RunFor(font, FaceFor(font), true);
        }

        static unsigned NextCodepoint(const char*& cursor, const char* end) {
            unsigned codepoint = static_cast<unsigned char>(*cursor);
            if (codepoint < 0x80)
                cursor += 1;
            else
                cursor += ImTextCharFromUtf8(&codepoint, cursor, end);
            return codepoint;
        }

        static bool IsTabularDigit(const Run& run, unsigned codepoint) {
            return run.digitAdvance > 0.0f && codepoint >= '0' && codepoint <= '9';
        }

        // Tabular digits do not kern with each other.
        static float KerningOf(const Run& run, unsigned left, unsigned right) {
            if (IsTabularDigit(run, left) && IsTabularDigit(run, right))
                return 0.0f;
            return float(Kerning::Adjustment(run.kerning, left, right)) * run.em / Kerning::UnitsPerEm;
        }

        static float AdvanceOf(const Run& run, unsigned codepoint) {
            return IsTabularDigit(run, codepoint) ? run.digitAdvance : run.baked->GetCharAdvance(ImWchar(codepoint)) * run.scale;
        }

        // Width of a line; offsets, when given, receive the pen position of every codepoint and the final width.
        static float Advance(const Run& run, std::string_view text, std::vector<float>* offsets = nullptr) {
            const char* cursor = text.data();
            const char* end = cursor + text.size();
            float width = 0.0f;
            unsigned previous = 0;
            while (cursor < end) {
                const unsigned codepoint = NextCodepoint(cursor, end);
                if (previous)
                    width += KerningOf(run, previous, codepoint);
                if (offsets)
                    offsets->push_back(width);
                width += AdvanceOf(run, codepoint) + run.tracking;
                previous = codepoint;
            }
            if (offsets)
                offsets->push_back(width);
            return width;
        }

        // Pen position: x of the first glyph origin and the baseline y, in pixels.
        static std::vector<unsigned>& Missing() {
            static std::vector<unsigned> missing;
            return missing;
        }

        std::span<const unsigned> MissingGlyphs() {
            return Missing();
        }

        static void EmitGlyphs(ImDrawList* draw, const Run& run, ImVec2 pen, ImU32 color, std::string_view text) {
            if ((color & IM_COL32_A_MASK) == 0 || text.empty())
                return;
            const char* begin = text.data();
            const char* end = begin + text.size();

            // Loading every glyph first keeps the atlas stable while vertices are written.
            int glyph_count = 0;
            for (const char* cursor = begin; cursor < end; ++glyph_count)
                run.baked->FindGlyph(ImWchar(NextCodepoint(cursor, end)));

            const ImVec4 clip = draw->_CmdHeader.ClipRect;
            const float ascent = IM_ROUND(run.baked->Ascent);
            draw->PrimReserve(glyph_count * 6, glyph_count * 4);
            int written = 0;
            float x = pen.x;
            unsigned previous = 0;
            for (const char* cursor = begin; cursor < end;) {
                const unsigned codepoint = NextCodepoint(cursor, end);
                if (previous)
                    x += KerningOf(run, previous, codepoint);
                const ImFontGlyph* glyph = run.baked->FindGlyph(ImWchar(codepoint));
                // ImGui hands back its fallback glyph for one the font lacks.
                if (glyph->Codepoint != codepoint && codepoint > ' ' && std::find(Missing().begin(), Missing().end(), codepoint) == Missing().end())
                    Missing().push_back(codepoint);
                const float advance = AdvanceOf(run, codepoint);
                if (glyph->Visible) {
                    const float left = x + (advance - glyph->AdvanceX * run.scale) * 0.5f * float(IsTabularDigit(run, codepoint));
                    const ImVec2 min(left + glyph->X0 * run.scale, pen.y + (glyph->Y0 - ascent) * run.scale);
                    const ImVec2 max(left + glyph->X1 * run.scale, pen.y + (glyph->Y1 - ascent) * run.scale);
                    if (max.x >= clip.x && min.x <= clip.z && max.y >= clip.y && min.y <= clip.w) {
                        draw->PrimRectUV(min, max, ImVec2(glyph->U0, glyph->V0), ImVec2(glyph->U1, glyph->V1), color);
                        ++written;
                    }
                }
                x += advance + run.tracking;
                previous = codepoint;
            }
            draw->PrimUnreserve((glyph_count - written) * 6, (glyph_count - written) * 4);
        }

        // A Markdown link, which SwiftUI's Text styles in string literals: "[Learn more…](target)" shows its title in the
        // link color; an autolink "<https://…>" shows the address underlined, as AppKit shows a detected link (Software
        // Update @2x). The range is in the text without markup.
        struct LinkSpan {
            size_t begin = 0;
            size_t end = 0;
            bool underlined = false;
            Rgba color;
        };

        // Copies text without link markup into plain; false, with plain left empty, when there is no link.
        static bool StripLinks(std::string_view text, std::string& plain, std::vector<LinkSpan>& links) {
            constexpr size_t none = std::string_view::npos;
            size_t cursor = 0;
            while (cursor < text.size()) {
                const size_t open = ImMin(text.find('[', cursor), text.find('<', cursor));
                if (open == none)
                    break;
                const bool autolink = text[open] == '<';
                const size_t middle = autolink ? open : text.find("](", open);
                const size_t close = middle == none ? none : text.find(autolink ? '>' : ')', autolink ? open : middle + 2);
                const std::string_view title = close == none ? std::string_view() : autolink ? text.substr(open + 1, close - open - 1) : text.substr(open + 1, middle - open - 1);
                if (close == none || (autolink && (title.find("://") == none || title.find(' ') != none))) {
                    // Not a link: the bracket stays in the text.
                    plain.append(text.substr(cursor, open + 1 - cursor));
                    cursor = open + 1;
                    continue;
                }
                plain.append(text.substr(cursor, open - cursor));
                const size_t begin = plain.size();
                plain.append(title);
                links.push_back({begin, plain.size(), autolink, Theme::Colors().link});
                cursor = close + 1;
            }
            if (links.empty()) {
                plain.clear();
                return false;
            }
            plain.append(text.substr(cursor));
            return true;
        }

        // Emits one line of plain text that starts at line_begin, with the spans in their colors. An underline is 0.045 em
        // thick, 0.136 em under the baseline (11 pt @2x: 0.5 pt, 1.5 pt down).
        static void EmitLine(ImDrawList* draw, const Run& run, ImVec2 pen, ImU32 color, std::string_view line, size_t line_begin, const std::vector<LinkSpan>& links) {
            size_t at = 0;
            for (const LinkSpan& link : links) {
                const size_t begin = ImClamp(link.begin, line_begin, line_begin + line.size()) - line_begin;
                const size_t end = ImClamp(link.end, line_begin, line_begin + line.size()) - line_begin;
                if (begin >= end)
                    continue;
                EmitGlyphs(draw, run, pen, color, line.substr(at, begin - at));
                pen.x += Advance(run, line.substr(at, begin - at));
                const std::string_view title = line.substr(begin, end - begin);
                EmitGlyphs(draw, run, pen, link.color.Packed(), title);
                const float width = Advance(run, title);
                if (link.underlined) {
                    const float top = pen.y + ImFloor(run.em * 0.136f + 0.5f);
                    Draw::FillRect(draw, ImRect(pen.x, top, pen.x + width, top + ImMax(1.0f, ImFloor(run.em * 0.045f + 0.5f))), link.color);
                }
                pen.x += width;
                at = end;
            }
            EmitGlyphs(draw, run, pen, color, line.substr(at));
        }

        float Width(const Font& font, std::string_view text) {
            return Advance(RunFor(font), text);
        }

        float Width(const Font& font, const char* text) {
            return text ? Width(font, std::string_view(text)) : 0.0f;
        }

        bool IsContinuationByte(char c) {
            return (static_cast<unsigned char>(c) & 0xC0) == 0x80;
        }

        int CodepointCount(std::string_view text) {
            int count = 0;
            for (const char c : text)
                count += IsContinuationByte(c) ? 0 : 1;
            return count;
        }

        size_t CodepointOffset(std::string_view text, int index) {
            size_t offset = 0;
            for (int i = 0; i < index && offset < text.size(); ++i) {
                ++offset;
                while (offset < text.size() && IsContinuationByte(text[offset]))
                    ++offset;
            }
            return offset;
        }

        float WidestWidth(const Font& font, std::span<const char* const> texts) {
            float widest = 0.0f;
            for (const char* text : texts)
                widest = ImMax(widest, Width(font, text));
            return widest;
        }

        void Offsets(const Font& font, std::string_view text, std::vector<float>& offsets) {
            offsets.clear();
            Advance(RunFor(font), text, &offsets);
        }

        float Baseline(const Font& font) {
            return CenteredBaseline(font, font.lineHeight * Environment().Scale());
        }

        float CapHeight(const Font& font) {
            return font.size * 1443.0f / 2048.0f * Environment().Scale();
        }

        float CenteredBaseline(const Font& font, float height) {
            return height * 0.5f + (Ascender - Descender) * 0.5f * font.size * Environment().Scale();
        }

        // CoreText's standard line breaking (pushOut) keeps a paragraph of two lines from ending on a lone word: the first
        // line hands it its last word (Bluetooth, Trackpad, Game Center, Wallpaper @2x: "…displayed in / your inbox.").
        // Longer paragraphs keep their orphan (Wallet, the Wi-Fi details sheet @2x).
        static void AvoidOrphan(const Run& run, std::string_view paragraph, float wrap_width, std::vector<std::string_view>& lines, size_t first) {
            if (lines.size() != first + 2 || lines.back().find(' ') != std::string_view::npos)
                return;
            std::string_view& previous = lines[lines.size() - 2];
            const size_t space = previous.rfind(' ');
            if (space == std::string_view::npos)
                return;
            const char* start = previous.data() + space + 1;
            const std::string_view last(start, size_t(paragraph.data() + paragraph.size() - start));
            if (Advance(run, last) > wrap_width)
                return;
            previous = previous.substr(0, space);
            lines.back() = last;
        }

        // Greedy line breaking at spaces; words longer than the width stay on their own line. The first line of the text
        // is first_indent shorter.
        static void BreakLines(const Run& run, std::string_view text, float wrap_width, std::vector<std::string_view>& lines, float first_indent = 0.0f) {
            size_t paragraph_start = 0;
            while (paragraph_start <= text.size()) {
                size_t paragraph_end = text.find('\n', paragraph_start);
                if (paragraph_end == std::string_view::npos)
                    paragraph_end = text.size();
                std::string_view paragraph = text.substr(paragraph_start, paragraph_end - paragraph_start);

                if (wrap_width <= 0.0f) {
                    lines.push_back(paragraph);
                } else {
                    const size_t first = lines.size();
                    size_t line_start = 0;
                    while (true) {
                        size_t fit_end = std::string_view::npos;
                        size_t search = line_start;
                        while (true) {
                            size_t word_end = paragraph.find(' ', search);
                            if (word_end == std::string_view::npos)
                                word_end = paragraph.size();
                            const float width = lines.empty() ? wrap_width - first_indent : wrap_width;
                            if (Advance(run, paragraph.substr(line_start, word_end - line_start)) > width && fit_end != std::string_view::npos)
                                break;
                            fit_end = word_end;
                            if (word_end >= paragraph.size())
                                break;
                            search = word_end + 1;
                        }
                        lines.push_back(paragraph.substr(line_start, fit_end - line_start));
                        if (fit_end >= paragraph.size())
                            break;
                        line_start = fit_end + 1;
                    }
                    AvoidOrphan(run, paragraph, wrap_width, lines, first);
                }
                if (paragraph_end >= text.size())
                    break;
                paragraph_start = paragraph_end + 1;
            }
        }

        ImVec2 Measure(const Font& font, std::string_view text, float wrap_width, float first_indent) {
            const Run run = RunFor(font);
            std::string plain;
            std::vector<LinkSpan> links;
            if (StripLinks(text, plain, links))
                text = plain;
            std::vector<std::string_view> lines;
            BreakLines(run, text, wrap_width, lines, first_indent);
            float width = 0.0f;
            for (size_t i = 0; i < lines.size(); ++i)
                width = ImMax(width, Advance(run, lines[i]) + (i == 0 ? first_indent : 0.0f));
            return ImVec2(width, float(lines.size()) * font.lineHeight * Environment().Scale());
        }

        static float AlignedX(const ImRect& rect, float width, TextAlignment alignment) {
            switch (alignment) {
                case TextAlignment::Center:
                    return rect.Min.x + (rect.GetWidth() - width) * 0.5f;
                case TextAlignment::Trailing:
                    return rect.Max.x - width;
                case TextAlignment::Leading:
                    break;
            }
            return rect.Min.x;
        }

        // CoreGraphics floors glyph origins in its bottom-up space, so a text baseline between pixels moves down to the
        // next one (System Settings, menus and sheets @2x: Body text a pixel below the nearest). A hair less keeps float
        // noise on a whole pixel from moving it.
        static float SnapBaseline(float y) {
            return ImCeil(y - 0.001f);
        }

        // Centering the line box puts the baseline (ascender - descender) / 2 below the middle, whatever the leading.
        // Symbols are images to AppKit and snap to the nearest pixel instead.
        static void DrawCentered(ImDrawList* draw, const Run& run, const ImRect& rect, Rgba color, std::string_view text, TextAlignment alignment, bool image) {
            std::string plain;
            std::vector<LinkSpan> links;
            const bool linked = !image && StripLinks(text, plain, links);
            const std::string_view shown = linked ? std::string_view(plain) : text;
            const float x = AlignedX(rect, Advance(run, shown), alignment);
            const float baseline = rect.GetCenter().y + (Ascender - Descender) * 0.5f * run.em;
            const ImVec2 pen(x, image ? ImFloor(baseline + 0.5f) : SnapBaseline(baseline));
            if (linked)
                EmitLine(draw, run, pen, color.Packed(), shown, 0, links);
            else
                EmitGlyphs(draw, run, pen, color.Packed(), text);
        }

        void Draw(ImDrawList* draw, const Font& font, const ImRect& rect, Rgba color, std::string_view text, TextAlignment alignment) {
            DrawCentered(draw, RunFor(font, FaceFor(font, RenderingFor(font, color)), true), rect, color, text, alignment, false);
        }

        // Lines broken to the rect width from its top, the spans (ranges of text) in their colors.
        static void DrawLines(ImDrawList* draw, const Font& font, const ImRect& rect, Rgba color, std::string_view text, const std::vector<LinkSpan>& links, TextAlignment alignment, float first_indent) {
            const Run run = RunFor(font, FaceFor(font, RenderingFor(font, color)), true);
            std::vector<std::string_view> lines;
            BreakLines(run, text, rect.GetWidth(), lines, first_indent);
            const float line_height = font.lineHeight * Environment().Scale();
            const float baseline = Baseline(font);
            const ImU32 color_packed = color.Packed();
            for (size_t i = 0; i < lines.size(); ++i) {
                const float indent = i == 0 ? first_indent : 0.0f;
                const float x = AlignedX(ImRect(rect.Min.x + indent, rect.Min.y, rect.Max.x, rect.Max.y), Advance(run, lines[i]), alignment);
                const float y = SnapBaseline(rect.Min.y + float(i) * line_height + baseline);
                EmitLine(draw, run, ImVec2(x, y), color_packed, lines[i], size_t(lines[i].data() - text.data()), links);
            }
        }

        void DrawWrapped(ImDrawList* draw, const Font& font, const ImRect& rect, Rgba color, std::string_view text, TextAlignment alignment, float first_indent) {
            std::string plain;
            std::vector<LinkSpan> links;
            DrawLines(draw, font, rect, color, StripLinks(text, plain, links) ? std::string_view(plain) : text, links, alignment, first_indent);
        }

        void DrawWrapped(ImDrawList* draw, const Font& font, const ImRect& rect, Rgba color, std::string_view text, std::span<const ColorSpan> spans) {
            std::vector<LinkSpan> links;
            for (const ColorSpan& span : spans)
                links.push_back({span.begin, span.end, false, span.color});
            DrawLines(draw, font, rect, color, text, links, TextAlignment::Leading, 0.0f);
        }

        std::string Truncate(const Font& font, std::string_view text, float max_width, TruncationMode mode) {
            const Run run = RunFor(font);
            if (Advance(run, text) <= max_width)
                return std::string(text);

            // Whole codepoints at either end, so prefixes and suffixes never split a UTF-8 sequence.
            const size_t codepoints = size_t(CodepointCount(text));
            const auto prefix = [&](size_t count) { return text.substr(0, CodepointOffset(text, int(count))); };
            const auto suffix = [&](size_t count) { return count == 0 ? std::string_view() : text.substr(CodepointOffset(text, int(codepoints - count))); };
            const std::string_view ellipsis = "\xE2\x80\xA6";
            // The most characters of one end that fit width.
            const auto longest = [&](const auto& end, float width) {
                size_t low = 0;
                size_t high = codepoints;
                while (low < high) {
                    const size_t middle = (low + high + 1) / 2;
                    if (Advance(run, end(middle)) <= width)
                        low = middle;
                    else
                        high = middle - 1;
                }
                return end(low);
            };
            // In the middle mode the end keeps what fits half the room and the start fills the rest, as CoreText does
            // (the copy window @2x: "Pippin - Apple's F…e Console.mov").
            const float room = max_width - Advance(run, ellipsis);
            const std::string_view tail = mode == TruncationMode::Middle ? longest(suffix, room * 0.5f) : std::string_view();
            std::string result(longest(prefix, room - Advance(run, tail)));
            while (!result.empty() && result.back() == ' ')
                result.pop_back();
            result.append(ellipsis);
            result.append(tail);
            return result;
        }

        ImFont* ImGuiFont(const Font& font) {
            return FaceFor(font).font;
        }

        float PixelSize(const Font& font) {
            return font.size * Environment().Scale();
        }

        std::string SymbolText(unsigned symbol) {
            char bytes[5] = {};
            const int length = ImTextCharToUtf8(bytes, symbol);
            return std::string(bytes, size_t(length));
        }

        float SymbolWidth(unsigned symbol, const Font& font) {
            return Advance(RunFor(font, SymbolFaceFor(font), false), SymbolText(symbol));
        }

        // The layers of a symbol drawn in layers, when its rendering asks for them.
        static const LayeredSymbol* LayersOf(unsigned symbol, const Font& font) {
            if (font.symbolRendering == SymbolRendering::Monochrome && font.variableValue < 0.0f)
                return nullptr;
            const auto layered = std::find_if(std::begin(LayeredSymbols), std::end(LayeredSymbols), [&](const LayeredSymbol& entry) { return entry.symbol == symbol; });
            return layered != std::end(LayeredSymbols) ? layered : nullptr;
        }

        void DrawSymbol(ImDrawList* draw, unsigned symbol, const Font& font, const ImRect& rect, Rgba color, TextAlignment alignment) {
            const Rendering rendering = RenderingFor(font, color);
            const LayeredSymbol* layered = LayersOf(symbol, font);
            const Face& layer_face = LayerFaces[int(rendering)][ImClamp(int(font.weight), 0, 4)];
            if (!layered || !layer_face.font) {
                DrawCentered(draw, RunFor(font, SymbolFaceFor(font, rendering), false), rect, color, SymbolText(symbol), alignment, true);
                return;
            }
            // Every layer keeps the symbol's advance, so it lands where the whole symbol would.
            const Run run = RunFor(font, layer_face, false);
            const unsigned first = layered->first + unsigned(int(font.symbolScale) * layered->count);
            for (int i = 0; i < layered->count; ++i) {
                float opacity = 1.0f;
                if (font.symbolRendering == SymbolRendering::Hierarchical && layered->levels[i] > 0)
                    opacity *= SecondaryLayerOpacity;
                if (font.variableValue >= 0.0f && font.variableValue <= float(i) / float(layered->count))
                    opacity *= DimmedLayerOpacity;
                DrawCentered(draw, run, rect, color.Opacity(opacity), SymbolText(first + unsigned(i)), alignment, true);
            }
        }
    } // namespace Typography
} // namespace Cupertino
