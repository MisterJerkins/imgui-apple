#include "Settings.h"

#include <cmath>
#include <string>

namespace Examples::Settings {
    using namespace Cupertino;

    struct AppearanceModel {
        int highlight = 0;
        // The accent the highlight color last followed: choosing an accent picks its highlight too.
        AccentColor highlightAccent = AccentColor::Multicolor;
        int iconSize = 1;
        int scrollBars = 0;
        int scrollClick = 0;
    };

    static AppearanceModel& Model() {
        static AppearanceModel model;
        return model;
    }

    // A scene painted where the assets have no picture: a menu at the upper left over the wallpaper and a window with its
    // lights at the lower right.
    static void PaintThemeScene(ImDrawList* draw, const ImRect& rect, bool dark, bool graphite) {
        const DrawingSpace space = PointSpace(rect);
        Draw::FillVerticalGradient(draw, rect, CornerRadii(Px(5.0f)), Rgba::Hex(dark ? 0x1D3557 : 0x5AA0E8), Rgba::Hex(dark ? 0x0B1A30 : 0x2E6CC9));
        const ImRect window(space.At(24.0f, 15.0f), space.At(64.0f, 41.0f));
        Draw::FillRoundedRect(draw, window, CornerRadii(Px(2.5f)), dark ? Rgba::Hex(0x2B2B2B) : Rgba::White(1.0f));
        Draw::FillRoundedRect(draw, ImRect(window.Min, ImVec2(window.Max.x, window.Min.y + Px(5.0f))), CornerRadii(Px(2.5f), Px(2.5f), 0.0f, 0.0f), dark ? Rgba::Hex(0x3A3A3A) : Rgba::Hex(0xECECEC));
        const unsigned lights[] = {0xFF5F57, 0xFEBC2E, 0x28C840};
        for (int i = 0; i < 3; ++i)
            Draw::FillCircle(draw, space.At(26.5f + 2.5f * float(i), 17.5f), Px(0.9f), graphite ? Rgba::Hex(0x9A9A9A) : Rgba::Hex(lights[i]));
        Draw::FillRoundedRect(draw, ImRect(space.At(2.5f, 4.0f), space.At(36.0f, 26.0f)), CornerRadii(Px(2.0f)), dark ? Rgba::Hex(0x3A3A3C) : Rgba::White(1.0f));
        for (const float y : {17.0f, 21.0f})
            Draw::FillRect(draw, ImRect(space.At(6.0f, y), space.At(26.0f, y + 1.5f)), dark ? Rgba::White(0.3f) : Rgba::Black(0.2f));
    }

    // The thumbnail pictures as System Settings shows them under the accent bar, from the assets' appearance folder
    // (512 Pixels @2x), or painted without them; Graphite has its own, with gray traffic lights. Auto is the light
    // scene's left half and the dark one's right half.
    static void DrawThemePicture(ImDrawList* draw, const ImRect& rect, int theme, bool graphite) {
        static const char* const Names[] = {"light", "dark", "auto"};
        const Bitmap picture = Picture(std::string("appearance/") + Names[theme] + (graphite ? "-graphite" : ""));
        if (!picture.IsEmpty()) {
            Draw::Image(draw, rect, picture);
            return;
        }
        if (theme != 2) {
            PaintThemeScene(draw, rect, theme == 1, graphite);
            return;
        }
        const float middle = rect.GetCenter().x;
        draw->PushClipRect(rect.Min, ImVec2(middle, rect.Max.y), true);
        PaintThemeScene(draw, rect, false, graphite);
        draw->PopClipRect();
        draw->PushClipRect(ImVec2(middle, rect.Min.y), rect.Max, true);
        PaintThemeScene(draw, ImRect(rect.Min + Px(ImVec2(34.0f, 0.0f)), rect.Max + Px(ImVec2(34.0f, 0.0f))), true, graphite);
        draw->PopClipRect();
    }

    // A thumbnail: the picture, and its menu's highlighted item in the accent as the window's appearance draws it. Auto
    // shows the light picture's left half and the dark one's right half, each with its own item.
    static void DrawThemeThumbnail(ImDrawList* draw, const ImRect& rect, int theme, AccentColor accent) {
        DrawThemePicture(draw, rect, theme, accent == AccentColor::Graphite);
        const Rgba color = Theme::Accent(accent, Environment().appearance);
        const auto item = [&](float left) {
            const ImRect bar(rect.Min + Px(ImVec2(left, 8.0f)), rect.Min + Px(ImVec2(left + 29.0f, 13.5f)));
            Draw::FillRoundedRect(draw, bar, CornerRadii(Px(1.5f)), color, CornerStyle::Circular);
        };
        draw->PushClipRect(rect.Min, rect.Max, true);
        item(4.5f);
        if (theme == 2)
            item(38.5f);
        draw->PopClipRect();
    }

    // Thumbnails of 67 x 44 pt in 74 x 51 pt cells 8 pt apart, leaving room for the selection ring: 3 pt wide, half a
    // point out, in the accent even when the window is not key (512 Pixels @2x).
    static void ThemeChooser(AppearancePreferences& preferences) {
        static const char* const Names[] = {"Light", "Dark", "Auto"};
        const ImVec2 thumbnail(67.0f, 44.0f);
        const float ring_room = 3.5f;
        HStack({.alignment = VerticalAlignment::Top, .spacing = 8.0f}, [&] {
            for (int theme = 0; theme < 3; ++theme) {
                const bool selected = int(preferences.theme) == theme;
                VStack({.alignment = HorizontalAlignment::Center, .spacing = 5.0f}, [&] {
                    Canvas(thumbnail + ImVec2(2.0f * ring_room, 2.0f * ring_room), [&](ImDrawList* draw, const ImRect& cell) {
                        const ImRect rect(cell.Min + Px(ImVec2(ring_room, ring_room)), cell.Max - Px(ImVec2(ring_room, ring_room)));
                        ImGui::PushID(theme);
                        if (Interaction::Button(ImGui::GetID("##theme"), rect).pressed)
                            preferences.theme = ThemeChoice(theme);
                        ImGui::PopID();
                        DrawThemeThumbnail(draw, rect, theme, preferences.accent);
                        if (selected) {
                            const ImRect ring(rect.Min - Px(ImVec2(0.5f, 0.5f)), rect.Max + Px(ImVec2(0.5f, 0.5f)));
                            Draw::StrokeRoundedRect(draw, ring, CornerRadii(Px(5.5f)), Theme::Accent(preferences.accent, Environment().appearance), Px(3.0f), StrokeAlignment::Outside);
                        }
                    });
                    Text(Names[theme], {.font = Font::Style(TextStyle::Subheadline).Weight(selected ? FontWeight::Semibold : FontWeight::Regular), .foreground = selected ? Foreground::Primary : Foreground::Secondary});
                });
            }
        });
    }

    // The Multicolor swatch: a conic gradient clockwise from the top through vivid purple, pink, red, orange, yellow,
    // green, cyan and blue, the same in both appearances (512 Pixels @2x).
    static void DrawMulticolor(ImDrawList* draw, ImVec2 center, float radius) {
        struct Stop {
            float turn;
            unsigned color;
        };
        static const Stop Stops[] = {{0.0f, 0x9A36DB}, {0.167f, 0xFC3591}, {0.264f, 0xFD1737}, {0.361f, 0xFD8401}, {0.444f, 0xE5B609}, {0.556f, 0x6DB951}, {0.653f, 0x02BBD1}, {0.75f, 0x1F8CE0}, {0.875f, 0x5155E7}, {1.0f, 0x9A36DB}};
        const auto color_at = [&](float turn) {
            int i = 0;
            while (i + 2 < IM_ARRAYSIZE(Stops) && Stops[i + 1].turn < turn)
                ++i;
            const float t = (turn - Stops[i].turn) / (Stops[i + 1].turn - Stops[i].turn);
            return Blend::Mix(Rgba::Hex(Stops[i].color), Rgba::Hex(Stops[i + 1].color), ImSaturate(t));
        };
        // Each wedge has its own center vertex in its middle color, so the colors meet at the center without mixing.
        const int segments = 72;
        const ImVec2 uv = draw->_Data->TexUvWhitePixel;
        draw->PrimReserve(segments * 3, segments * 3);
        for (int i = 0; i < segments; ++i) {
            const float from = float(i) / float(segments);
            const float to = float(i + 1) / float(segments);
            const ImVec2 start(std::sin(2.0f * IM_PI * from), -std::cos(2.0f * IM_PI * from));
            const ImVec2 end(std::sin(2.0f * IM_PI * to), -std::cos(2.0f * IM_PI * to));
            const unsigned first = draw->_VtxCurrentIdx;
            draw->PrimWriteVtx(center, uv, color_at((from + to) * 0.5f).Packed());
            draw->PrimWriteVtx(center + start * radius, uv, color_at(from).Packed());
            draw->PrimWriteVtx(center + end * radius, uv, color_at(to).Packed());
            draw->PrimWriteIdx(ImDrawIdx(first));
            draw->PrimWriteIdx(ImDrawIdx(first + 1));
            draw->PrimWriteIdx(ImDrawIdx(first + 2));
        }
    }

    static void AccentChooser(const char* multicolor_name, AppearancePreferences& preferences) {
        static const AccentColor Accents[] = {AccentColor::Multicolor, AccentColor::Blue, AccentColor::Purple, AccentColor::Pink, AccentColor::Red, AccentColor::Orange, AccentColor::Yellow, AccentColor::Green, AccentColor::Graphite};
        const char* const names[] = {multicolor_name, "Blue", "Purple", "Pink", "Red", "Orange", "Yellow", "Green", "Graphite"};
        // Swatches of 16 pt, 26 pt apart, centered 10 pt below the top and ending a point before the row's inset; the
        // caption's line centered at 33 pt.
        const float diameter = 16.0f;
        const float pitch = 26.0f;
        Canvas(ImVec2(pitch * 8.0f + diameter + 1.0f, 39.0f), [&](ImDrawList* draw, const ImRect& rect) {
            int selected = 0;
            for (int i = 0; i < 9; ++i) {
                const ImVec2 center = rect.Min + Px(ImVec2(diameter * 0.5f + pitch * float(i), 10.0f));
                const float radius = Px(diameter * 0.5f);
                const ImRect circle(center - ImVec2(radius, radius), center + ImVec2(radius, radius));
                ImGui::PushID(i);
                if (Interaction::Button(ImGui::GetID("##accent"), circle).pressed)
                    preferences.accent = Accents[i];
                ImGui::PopID();
                if (preferences.accent == Accents[i])
                    selected = i;
                if (i == 0)
                    DrawMulticolor(draw, center, radius);
                else
                    Draw::FillCircle(draw, center, radius, Theme::Accent(Accents[i], Environment().appearance));
                // A point-wide edge inside, darker in light and lighter in dark; the selected swatch has a 5.5 pt dot.
                Draw::StrokeCircle(draw, center, radius, Environment().IsDark() ? Rgba::White(0.2f) : Rgba::Black(0.2f), Px(1.0f));
                if (preferences.accent == Accents[i])
                    Draw::FillCircle(draw, center, Px(2.75f), Rgba::White(1.0f));
            }
            const float caption_center = rect.Min.x + Px(diameter * 0.5f + pitch * float(selected));
            const Font font = Font::Style(TextStyle::Subheadline);
            const float width = Typography::Width(font, names[selected]);
            const ImRect caption(caption_center - width * 0.5f, rect.Min.y + Px(26.0f), caption_center + width * 0.5f, rect.Min.y + Px(40.0f));
            Typography::Draw(draw, font, caption, Theme::Colors().secondaryLabel, names[selected], TextAlignment::Center);
        });
    }

    // Highlight color swatches, 24 x 12 with a hairline: "Accent Color" is the Multicolor gradient sampled from
    // Apple's screenshot, the others their highlight color, "Other..." has none.
    static void PaintHighlightSwatch(ImDrawList* draw, const ImRect& frame, int item) {
        static const AccentColor Accents[] = {AccentColor::Blue, AccentColor::Purple, AccentColor::Pink, AccentColor::Red, AccentColor::Orange, AccentColor::Yellow, AccentColor::Green, AccentColor::Graphite};
        if (item == 0) {
            const float middle = ImLerp(frame.Min.x, frame.Max.x, 0.55f);
            const ImU32 purple = Rgba::Hex(0x9690D0).Packed();
            const ImU32 pink = Rgba::Hex(0xFF9A99).Packed();
            const ImU32 yellow = Rgba::Hex(0xFFF599).Packed();
            draw->AddRectFilledMultiColor(frame.Min, ImVec2(middle, frame.Max.y), purple, pink, pink, purple);
            draw->AddRectFilledMultiColor(ImVec2(middle, frame.Min.y), frame.Max, pink, yellow, yellow, pink);
        } else if (item <= IM_ARRAYSIZE(Accents)) {
            Draw::FillRect(draw, frame, Theme::Highlight(Accents[item - 1], Environment().appearance));
        } else {
            return;
        }
        Draw::StrokeRoundedRect(draw, frame, CornerRadii(0.0f), Rgba::Black(0.15f), Px(0.5f), StrokeAlignment::Inside);
    }

    void AppearancePane(const Mac& mac, AppearancePreferences& preferences) {
        AppearanceModel& model = Model();
        // The highlight items follow the accents in order, "Accent Color" first for Multicolor.
        if (model.highlightAccent != preferences.accent) {
            model.highlightAccent = preferences.accent;
            model.highlight = int(preferences.accent);
        }
        // The two choosers keep their label on the row's first line (@2x).
        const auto top_labeled = [](const char* label, const std::function<void()>& content) {
            HStack({.alignment = VerticalAlignment::Top, .spacing = 8.0f}, [&] {
                Text(label);
                Spacer();
                content();
            });
        };
        Form([&] {
            Section([&] {
                top_labeled("Appearance", [&] { ThemeChooser(preferences); });
                top_labeled(mac.accentTitle, [&] { AccentChooser(mac.multicolorName, preferences); });
                Picker(mac.highlightTitle, &model.highlight, {mac.accentItem, "Blue", "Purple", "Pink", "Red", "Orange", "Yellow", "Green", "Graphite", "Other..."}, {.itemImage = PaintHighlightSwatch});
                Picker("Sidebar icon size", &model.iconSize, {"Small", "Medium", "Large"});
                Toggle("Allow wallpaper tinting in windows", &preferences.wallpaperTinting);
            });
            Section([&] {
                Picker("Show scroll bars", &model.scrollBars, {"Automatically based on mouse or trackpad", "When scrolling", "Always"}, {.style = PickerStyle::RadioGroup});
                Picker("Click in the scroll bar to", &model.scrollClick, {"Jump to the next page", "Jump to the spot that\xE2\x80\x99s clicked"}, {.style = PickerStyle::RadioGroup});
            });
            // The help button sits 21 pt under the last section.
            TrailingButtons({.top = 11.0f}, [&] {
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
