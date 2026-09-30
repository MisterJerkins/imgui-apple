#include "Settings.h"

#include "core/Metrics.h"

namespace Examples::Settings {
    using namespace Cupertino;

    struct DisplaysModel {
        int resolution = 2;
        float brightness = 0.5f;
        bool autoBrightness = true;
        bool trueTone = true;
        int colorProfile = 0;
    };

    // Sequoia's redwoods on the MacBook's screen: colors sampled on a 20 x 12 grid from the 512 Pixels capture and
    // blended across each cell.
    static void PaintRedwoods(ImDrawList* draw, const ImRect& screen) {
        static const unsigned grid[13][21] = {
            0x222715, 0x615439, 0x3F2F22, 0x84979B, 0x5A6760, 0x535945, 0x556668, 0x434430, 0x272F23, 0x817F68, 0x563F2B, 0x354137, 0x735B42, 0x25251F, 0x1B1A11, 0x616F66, 0x3B4F51, 0x46554D, 0x455D61, 0x4B423B, 0x151613,
            0x11190C, 0x604E33, 0x473320, 0x828C7A, 0x5C654D, 0x575A3F, 0x65746E, 0x67674B, 0x3C4331, 0x998C6D, 0x61452D, 0x414A30, 0x846441, 0x24251E, 0x242016, 0x59685F, 0x3C4C3E, 0x49513A, 0x5C7269, 0x413C2F, 0x1C1D19,
            0x1E200E, 0x62492D, 0x4C3A27, 0x7A7659, 0x4B5138, 0x686643, 0x585636, 0x58593F, 0x393C29, 0xAF9A7C, 0x61462B, 0x43492D, 0x80603E, 0x272720, 0x2D271B, 0x49574D, 0x2F4036, 0x2A3928, 0x80937C, 0x53452A, 0x1B1A14,
            0x292910, 0x644428, 0x45311D, 0x4C4F3C, 0x343C29, 0x4F5034, 0x43422A, 0x4B4932, 0x3A3A26, 0xBFA583, 0x5E4627, 0x444830, 0x563F26, 0x292721, 0x2F2619, 0x5B5C42, 0x2D3A29, 0x2A3825, 0x677267, 0x4D3C1F, 0x211C11,
            0x392B10, 0x765232, 0x382814, 0x383A2C, 0x3B3E28, 0x3C3C27, 0x3A3724, 0x393A29, 0x373421, 0xC19D7A, 0x44331D, 0x222C1D, 0x372B19, 0x2E271E, 0x302517, 0x4B4D2C, 0x364125, 0x323824, 0x565535, 0x402D16, 0x1E170D,
            0x554220, 0x5C3B20, 0x3A2C16, 0x1D281A, 0x212A1A, 0x313620, 0x3A3723, 0x3A3928, 0x3A3722, 0xCBA17D, 0x49371D, 0x262D1F, 0x32271A, 0x30271E, 0x2F2315, 0x2B2F1E, 0x343D22, 0x3C3D26, 0x46452B, 0x3E2A11, 0x25180B,
            0x775B2C, 0x3D260F, 0x2A2613, 0x0F1A0F, 0x1F2519, 0x323520, 0x262519, 0x3F3F2A, 0x473C21, 0x966B4B, 0x553F27, 0x363323, 0x543C27, 0x33271E, 0x2B1F13, 0x2B3118, 0x1A261B, 0x313220, 0x25271C, 0x2B1C0C, 0x20170B,
            0x5E3F22, 0x2F1E09, 0x2B2416, 0x182016, 0x1F271A, 0x273022, 0x212420, 0x233021, 0x3E3824, 0x422C1C, 0x403222, 0x4F4937, 0x674E34, 0x302419, 0x281F13, 0x3F3921, 0x273128, 0x2E372D, 0x252D2C, 0x442C18, 0x3B240F,
            0x2A2411, 0x231607, 0x303417, 0x404C2D, 0x555E37, 0x6B7045, 0x41493D, 0x444D3B, 0x554E3D, 0x352418, 0x4D4529, 0x5A5637, 0x332619, 0x2A2117, 0x1C160D, 0x5A4D3A, 0x5A574B, 0x484E4B, 0x4C534D, 0x613F1F, 0x382713,
            0x1D2F14, 0x2A3F16, 0x32471C, 0x3B4A31, 0x49553D, 0x566539, 0x646A47, 0x6B6D4D, 0x505538, 0x363F22, 0x575634, 0x7C732B, 0x2A2015, 0x231C13, 0x21170D, 0x715C42, 0x8C765B, 0x54574C, 0x5C5946, 0x55371B, 0x1C180C,
            0x152E0C, 0x273E10, 0x203A10, 0x1D361F, 0x223A1E, 0x28411C, 0x334823, 0x3B4E2A, 0x384C28, 0x3B502A, 0x4B5C30, 0x464428, 0x231C13, 0x1F170E, 0x23180F, 0x3A3328, 0x3E413B, 0x42443F, 0x4C4738, 0x6E4F31, 0x1E1A14,
            0x32401B, 0x2E3F14, 0x18310C, 0x1E3618, 0x152D1C, 0x1A2F21, 0x283E23, 0x243D21, 0x2E4220, 0x3A4E25, 0x495331, 0x322F1D, 0x201910, 0x17120B, 0x1E160E, 0x312A1C, 0x293428, 0x2E3D30, 0x5A4E39, 0x2E2A23, 0x141A1C,
            0x1C3110, 0x273A16, 0x1F3217, 0x1E341A, 0x1C3320, 0x172A22, 0x253A27, 0x203724, 0x314229, 0x374B28, 0x314027, 0x312D1C, 0x170D06, 0x160F09, 0x1D150C, 0x222217, 0x1F2C23, 0x333E31, 0x5A4B34, 0x1D2323, 0x131D20,
        };
        const ImVec2 cell(screen.GetWidth() / 20.0f, screen.GetHeight() / 12.0f);
        const auto color = [](int row, int column) { return Rgba::Hex(grid[row][column]).Packed(); };
        for (int row = 0; row < 12; ++row) {
            for (int column = 0; column < 20; ++column) {
                const ImVec2 min = screen.Min + ImVec2(cell.x * float(column), cell.y * float(row));
                draw->AddRectFilledMultiColor(min, min + cell, color(row, column), color(row, column + 1), color(row + 1, column + 1), color(row + 1, column));
            }
        }
    }

    // The arrangement over the settings (Displays @2x): a band from edge to edge of the pane that darkens toward its
    // bottom line, the display with its name, and the button that adds a display.
    static void Arrangement(const Mac& mac) {
        const auto band = [](ImDrawList* draw, const ImRect& rect) {
            const float margin = Px(Metrics::Form().margin);
            const ImRect area(rect.Min.x - margin, rect.Min.y, rect.Max.x + margin, rect.Max.y);
            Draw::FillVerticalGradient(draw, area, CornerRadii(0.0f), Rgba::Black(0.0f), Rgba::Black(0.05f));
            Draw::FillRect(draw, ImRect(area.Min.x, area.Max.y - Px(1.0f), area.Max.x, area.Max.y), Rgba::Black(0.05f));
        };
        Padding(EdgeInsets{0.0f, 0.0f, 10.0f, 0.0f}, [&] {
            Background(band, [&] {
                ZStack({.alignment = Alignment{HorizontalAlignment::Trailing, VerticalAlignment::Top}}, [&] {
                    Frame({.height = 159.0f, .maxWidth = Infinity, .alignment = Alignment{HorizontalAlignment::Center, VerticalAlignment::Top}}, [] {
                        Padding(EdgeInsets{39.5f, 0.0f, 0.0f, 0.0f}, [] {
                            VStack({.spacing = 7.5f}, [] {
                                Canvas(ImVec2(126.0f, 78.0f), [](ImDrawList* draw, const ImRect& rect) { PaintMacBookAir(draw, rect, PaintRedwoods); });
                                Text("Built-in Display", {.font = Font::System(13.0f, FontWeight::Semibold)});
                            });
                        });
                    });
                    Padding(EdgeInsets{123.0f, 0.0f, 0.0f, 0.0f}, [&] {
                        Menu("Add Display", {.symbol = Symbols::Plus}, [&] {
                            Section({.header = "Mirror or extend to"}, [&] {
                                Button("Living Room");
                                Button(mac.ipadName);
                            });
                        });
                    });
                });
            });
        });
    }

    // One resolution as a thumbnail of a window at that scale (Displays @2x): a 21 pt title bar with its lights and a
    // paragraph set in 12/15 pt that runs past the thumbnail's edge; the window is a dark one in dark mode.
    static void PaintResolution(ImDrawList* draw, const ImRect& rect, float scale) {
        static const char* const lines[] = {
            "Here\xE2\x80\x99s to the crazy ones. The misfits. The rebels. The",
            "troublemakers. The round pegs in the square holes. The",
            "ones who see things differently. They\xE2\x80\x99re not fond of",
            "rules. And they have no respect for the status quo. You",
            "can quote them, disagree with them, glorify or vilify",
            "them. About the only thing you can\xE2\x80\x99t do is ignore them.",
            "Because they change things.",
        };
        const Palette& colors = Theme::Colors();
        const bool dark = Environment().IsDark();
        const CornerRadii radii(Px(5.0f));
        Draw::DropShadow(draw, rect, radii, Shadow{Rgba::Black(0.2f), ImVec2(0.0f, 0.5f), 1.5f});
        Draw::FillRoundedRect(draw, rect, radii, dark ? Rgba::Hex(0x2D2D2D) : Rgba::Hex(0xFAFAFA));
        const float bar = Px(21.0f * scale);
        Draw::FillRoundedRect(draw, ImRect(rect.Min, ImVec2(rect.Max.x, rect.Min.y + bar)), CornerRadii(radii.topLeft, radii.topRight, 0.0f, 0.0f), dark ? Rgba::Hex(0x3C3C3C) : Rgba::White(1.0f));
        Draw::FillRect(draw, ImRect(rect.Min.x, rect.Min.y + bar, rect.Max.x, rect.Min.y + bar + Px(0.5f)), dark ? Rgba::Black(0.5f) : Rgba::Hex(0xE1E1E1));
        draw->PushClipRect(rect.Min, rect.Max, true);
        const Rgba lights[] = {colors.trafficClose, colors.trafficMinimize, colors.trafficZoom};
        for (int i = 0; i < 3; ++i)
            Draw::FillCircle(draw, rect.Min + Px(ImVec2(13.5f + 20.0f * float(i), 10.5f) * scale), Px(6.0f * scale), lights[i]);
        const Font font = Font::System(12.0f * scale).WithLineHeight(15.0f * scale);
        for (int i = 0; i < IM_ARRAYSIZE(lines); ++i) {
            const float baseline = rect.Min.y + Px((37.1f + 15.0f * float(i)) * scale);
            const ImVec2 min(rect.Min.x + Px(7.0f * scale), baseline - Typography::Baseline(font));
            Typography::Draw(draw, font, ImRect(min, ImVec2(min.x + Px(400.0f), min.y + Px(font.lineHeight))), colors.label, lines[i]);
        }
        draw->PopClipRect();
        Draw::StrokeRoundedRect(draw, rect, radii, dark ? Rgba::White(0.15f) : Rgba::Black(0.12f), Px(0.5f));
    }

    // The scaled resolutions of the built-in display: four thumbnails a third of the section apart, the first, the
    // default and the last one named under them; the chosen one gets an accent ring and a bold name.
    static void ResolutionPicker(int* selection) {
        static const float scales[] = {5.0f / 6.0f, 2.0f / 3.0f, 0.5f, 1.0f / 3.0f};
        static const char* const names[] = {"Larger Text", nullptr, "Default", "More Space"};
        const ImRect area = Layout::Place(Layout::Placement{.size = ImVec2(Layout::FullProposal().x, Px(108.0f)), .ignoresChildInsets = true});
        if (Layout::IsMeasuring())
            return;
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const float center = area.GetCenter().x;
        for (int i = 0; i < 4; ++i) {
            const float x = center + Px(111.67f * (float(i) - 1.5f));
            const ImRect tile(x - Px(21.875f), area.Min.y + Px(22.0f), x + Px(21.875f), area.Min.y + Px(65.75f));
            const ImRect hit(x - Px(50.0f), area.Min.y, x + Px(50.0f), area.Max.y);
            ImGui::PushID(i);
            if (Interaction::Button(ImGui::GetID("resolution"), hit).pressed)
                *selection = i;
            ImGui::PopID();
            if (*selection == i)
                Draw::StrokeRoundedRect(draw, ImRect(tile.Min - Px(ImVec2(2.5f, 2.5f)), tile.Max + Px(ImVec2(2.5f, 2.5f))), CornerRadii(Px(7.5f)), Rgba::Hex(0x4797E8), Px(2.5f));
            PaintResolution(draw, tile, scales[i]);
            if (!names[i])
                continue;
            const Font font = Font::Style(TextStyle::Subheadline).Weight(*selection == i ? FontWeight::Bold : FontWeight::Regular);
            const float top = area.Min.y + Px(85.0f) - Typography::Baseline(font);
            Typography::Draw(draw, font, ImRect(x - Px(55.0f), top, x + Px(55.0f), top + Px(font.lineHeight)), colors.label, names[i], TextAlignment::Center);
        }
    }

    // Displays as 512 Pixels captured it (a MacBook Air on its own): the arrangement, the resolutions, brightness and
    // True Tone, the color profile and the buttons for advanced settings and Night Shift.
    void DisplaysPane(const Mac& mac) {
        static DisplaysModel model;
        Form([&] {
            Arrangement(mac);
            Section([] { ResolutionPicker(&model.resolution); });
            Section([] {
                Slider("Brightness", &model.brightness, 0.0f, 1.0f, {.minimumSymbol = Symbols::SunMinFill, .maximumSymbol = Symbols::SunMaxFill, .symbolScale = SymbolScale::Large});
                Toggle("Automatically adjust brightness", &model.autoBrightness);
                Toggle("True Tone", &model.trueTone, {.description = "Automatically adapt display to make colors appear consistent in different ambient lighting conditions."});
            });
            Section([] { Picker("Color profile", &model.colorProfile, {"Color LCD", "Display P3", "sRGB IEC61966-2.1", "Generic RGB Profile"}); });
            // The buttons keep 30 pt from the last section.
            TrailingButtons({.top = 20.0f}, [] {
                Button("Advanced\xE2\x80\xA6");
                Button("Night Shift\xE2\x80\xA6");
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
