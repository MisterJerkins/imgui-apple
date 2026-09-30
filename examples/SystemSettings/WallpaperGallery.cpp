#include "Settings.h"

#include <functional>
#include <string>
#include <string_view>

namespace Examples::Settings {
    using namespace Cupertino;

    // Thumbnails are 71 pt tall, 117.5 pt apart with 3.5 pt corners and a hairline around them; the selected one shrinks
    // 2 pt into a gap of the pane's color inside a 2 pt gray ring (Wallpaper and Screen Saver @2x).
    static constexpr float PhotoHeight = 71.0f;
    static constexpr float Pitch = 117.5f;
    static constexpr float Radius = 3.5f;

    void DrawWallpaper(ImDrawList* draw, const ImRect& rect, const char* pane, const char* picture, float radius) {
        const Bitmap photo = Picture(std::string("wallpapers/") + pane + "/" + picture);
        if (!photo.IsEmpty()) {
            Draw::Image(draw, rect, photo, radius);
            return;
        }
        // A sky of its own: dusk colors picked by the picture's name.
        static const std::pair<unsigned, unsigned> Skies[] = {{0x1D4E89, 0xF4A259}, {0x2B2D6E, 0xE56B6F}, {0x0B6E4F, 0xA7D7C5}, {0x3A506B, 0xC4D7F2}, {0x6D3B8C, 0xF7B267}, {0x14213D, 0x4F9DDE}};
        const std::pair<unsigned, unsigned>& sky = Skies[std::hash<std::string_view>{}(picture) % std::size(Skies)];
        Draw::FillVerticalGradient(draw, rect, CornerRadii(radius), Rgba::Hex(sky.first), Rgba::Hex(sky.second));
    }

    static void PaintThumbnail(ImDrawList* draw, const ImRect& rect, const char* pane, const char* picture, bool selected) {
        const bool dark = Environment().IsDark();
        const Rgba ring = dark ? Rgba::White(0.15f) : Rgba::Black(0.07f);
        if (!selected) {
            Draw::StrokeRoundedRect(draw, rect, CornerRadii(Px(Radius)), ring, Px(0.5f), StrokeAlignment::Outside);
            DrawWallpaper(draw, rect, pane, picture, Px(Radius));
            return;
        }
        const auto inset = [&](float points) { return ImRect(rect.Min + Px(ImVec2(points, points)), rect.Max - Px(ImVec2(points, points))); };
        Draw::FillRoundedRect(draw, inset(-1.0f), CornerRadii(Px(Radius + 1.0f)), ring);
        Draw::FillRoundedRect(draw, inset(1.0f), CornerRadii(Px(Radius - 1.0f)), dark ? Theme::Colors().windowBackground : Rgba::White(1.0f));
        DrawWallpaper(draw, inset(2.0f), pane, picture, Px(Radius - 2.0f));
    }

    std::span<const WallpaperItem> Landscapes() {
        static const WallpaperItem items[] = {{"Sequoia Sunrise", "sequoia-sunrise"}, {"Sonoma Horizon", "sonoma-horizon", true}, {"California\xE2\x80\x99s Temblor Range", "californias-temblor-range", true}, {"Redwoods from Above", "redwoods-from-above", true}, {"Tahoe Day", "landscape-5", true}};
        return items;
    }

    std::span<const WallpaperItem> Cityscapes() {
        static const WallpaperItem items[] = {{"Dubai", "cityscape-1", true}, {"Los Angeles", "cityscape-2", true}, {"London", "cityscape-3", true}, {"Hong Kong", "cityscape-4", true}, {"New York", "cityscape-5", true}};
        return items;
    }

    // The name under a thumbnail, Caption centered on two lines at most; one still to download ends in the text's own
    // arrow (U+2193) joined by a no-break space, so the arrow never stands alone.
    static std::string ThumbnailName(const WallpaperItem& item) {
        return item.download ? std::string(item.name) + "\xC2\xA0\xE2\x86\x93" : std::string(item.name);
    }

    void WallpaperCategory(const WallpaperGallery& gallery, const char* title, int count, std::span<const WallpaperItem> items, int selected) {
        const Font caption = Font::Style(TextStyle::Caption);
        const std::string show_all = "Show All (" + std::to_string(count) + ")";
        VStack({.alignment = HorizontalAlignment::Leading, .spacing = 0.0f}, [&] {
            HStack({.alignment = VerticalAlignment::FirstTextBaseline}, [&] {
                // Medium, where a form's section headers are Semibold (@2x: the same width and ink).
                Text(title, {.font = Font::Style(TextStyle::Body).Weight(FontWeight::Medium)});
                Spacer();
                Text(show_all, {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary});
            });
            // The row runs on past the window's edge, which cuts the fifth thumbnail off; with the form's spacing the
            // categories stand 144 pt apart.
            Padding(EdgeInsets{10.0f, gallery.leading, 7.5f, 0.0f}, [&] {
                HStack({.alignment = VerticalAlignment::Top, .spacing = Pitch - gallery.photoWidth}, [&] {
                    for (size_t i = 0; i < items.size(); ++i) {
                        VStack({.spacing = 3.5f}, [&] {
                            Canvas(ImVec2(gallery.photoWidth, PhotoHeight), [&](ImDrawList* draw, const ImRect& rect) { PaintThumbnail(draw, rect, gallery.pane, items[i].picture, int(i) == selected); });
                            Frame({.width = gallery.photoWidth, .height = 2.0f * caption.lineHeight, .alignment = {HorizontalAlignment::Center, VerticalAlignment::Top}}, [&] {
                                Text(ThumbnailName(items[i]), {.font = caption, .alignment = TextAlignment::Center, .wraps = true});
                            });
                        });
                    }
                });
            });
        });
    }
} // namespace Examples::Settings
