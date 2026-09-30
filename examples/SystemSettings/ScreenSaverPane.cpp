#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    struct ScreenSaverModel {
        bool wallpaper = true;
        bool allSpaces = true;
    };

    // The warning over the galleries: an inset box with the notice on the leading side, its text column 260 pt wide, and
    // the button to Lock Screen centered on the trailing side (Screen Saver @2x).
    static void SleepWarning() {
        Background(DrawInsetBox, [] {
            Padding(EdgeInsets::Symmetric(13.0f, 9.5f), [] {
                HStack([] {
                    // The notice keeps a point under its last line, which centers the button on the box.
                    Frame({.width = 260.0f, .alignment = {HorizontalAlignment::Leading, VerticalAlignment::Top}}, [] {
                        Padding(EdgeInsets{0.0f, 0.0f, 1.0f, 0.0f}, [] {
                            VStack({.alignment = HorizontalAlignment::Leading, .spacing = 0.5f}, [] {
                                HStack({.spacing = 4.0f}, [] {
                                    Image(Symbols::ExclamationmarkTriangleFill, {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Custom, .color = Theme::SystemYellow()});
                                    Text("Display will sleep before screen saver starts.", {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary});
                                });
                                // AppKit's label keeps the lone last word a SwiftUI Text would push out; a line break holds it.
                                Text("You can manage the timing in Lock Screen\nSettings.", {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary, .wraps = true});
                            });
                        });
                    });
                    Spacer();
                    Button("Lock Screen Settings\xE2\x80\xA6");
                });
            });
        });
    }

    // Screen Saver as 512 Pixels captured it: the current picture beside its name and switches, the sleep warning and the
    // galleries under a line across the pane.
    void ScreenSaverPane() {
        static ScreenSaverModel model;
        static const WallpaperGallery Gallery = {.pane = "screen-saver", .photoWidth = 107.5f, .leading = 0.0f};
        static const WallpaperItem MacOS[] = {{"Sequoia", "sequoia"}, {"Macintosh", "macintosh"}, {"Sonoma", "sonoma"}, {"Ventura", "ventura"}, {"Monterey", "monterey"}};
        Form([] {
            HStack({.alignment = VerticalAlignment::Top, .spacing = 0.0f}, [] {
                Frame({.width = 153.0f, .alignment = {HorizontalAlignment::Leading, VerticalAlignment::Top}}, [] {
                    Canvas(ImVec2(143.0f, 80.0f), [](ImDrawList* draw, const ImRect& rect) { DrawWallpaper(draw, rect, "screen-saver", "preview", Px(3.5f)); });
                });
                VStack({.spacing = 10.0f}, [] {
                    Section([] { Text("Sequoia Sunrise"); });
                    Section([] {
                        Toggle("Show as wallpaper", &model.wallpaper);
                        Toggle("Show on all Spaces", &model.allSpaces);
                    });
                });
            });
            Padding(EdgeInsets{5.0f, -20.0f, 8.0f, -20.0f}, [] { Divider(); });
            SleepWarning();
            Padding(EdgeInsets{13.0f, 0.0f, 0.0f, 0.0f}, [] { WallpaperCategory(Gallery, "macOS", 5, MacOS); });
            WallpaperCategory(Gallery, "Landscape", 64, Landscapes(), 0);
            WallpaperCategory(Gallery, "Cityscape", 30, Cityscapes());
        });
    }
} // namespace Examples::Settings
