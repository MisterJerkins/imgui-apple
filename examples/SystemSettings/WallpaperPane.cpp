#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    struct WallpaperModel {
        bool screenSaver = true;
        bool allSpaces = true;
    };

    // Wallpaper as 512 Pixels captured it: the current picture in its well beside its name and switches, the buttons that
    // add pictures, and the galleries under a line across the pane.
    void WallpaperPane() {
        static WallpaperModel model;
        static const WallpaperGallery Gallery = {.pane = "wallpaper", .photoWidth = 106.5f, .leading = 0.5f};
        static const WallpaperItem Dynamic[] = {{"Sequoia", "sequoia"}, {"Macintosh", "macintosh"}, {"Sonoma", "sonoma"}, {"Ventura", "ventura", true}, {"Monterey", "monterey", true}};
        Form([] {
            VStack({.alignment = HorizontalAlignment::Trailing, .spacing = 14.0f}, [] {
                HStack({.alignment = VerticalAlignment::Top, .spacing = 0.0f}, [] {
                    // The well reaches 2 pt past the 143 pt slot the pictures of both panes share.
                    Frame({.width = 153.0f, .alignment = {HorizontalAlignment::Leading, VerticalAlignment::Top}}, [] {
                        Padding(EdgeInsets{0.0f, -2.0f, 0.0f, 0.0f}, [] { ImageWell("##current", PictureIcon("wallpapers/wallpaper/preview", {.symbol = Symbols::Photo, .color = Theme::Colors().tertiaryLabel}), {.size = ImVec2(147.0f, 82.0f)}); });
                    });
                    VStack({.spacing = 10.0f}, [] {
                        Section([] { Text("Sequoia Sunrise"); });
                        Section([] {
                            Toggle("Show as screen saver", &model.screenSaver);
                            Toggle("Show on all Spaces", &model.allSpaces);
                        });
                    });
                });
                TrailingButtons([] {
                    Menu("Add Photo", {"Choose\xE2\x80\xA6"}, {.plainIndicator = true});
                    Menu("Add Folder or Album", {"Choose Folder\xE2\x80\xA6", "Add Photos Album\xE2\x80\xA6"}, {.plainIndicator = true});
                });
            });
            Padding(EdgeInsets{4.0f, -20.0f, 16.0f, -20.0f}, [] { Divider(); });
            WallpaperCategory(Gallery, "Dynamic Wallpapers", 33, Dynamic);
            WallpaperCategory(Gallery, "Landscape", 64, Landscapes(), 0);
            WallpaperCategory(Gallery, "Cityscape", 30, Cityscapes());
        });
    }
} // namespace Examples::Settings
