#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    struct StorageKind {
        const char* name;
        float gigabytes;
        Rgba color;
    };

    struct StorageCategory {
        const char* name;
        const char* size;
        Icon icon;
    };

    static const float DiskCapacity = 245.11f;

    // The Applications folder's letter A.
    static void PaintApplications(ImDrawList* draw, const ImRect& frame, Rgba color) {
        Typography::Draw(draw, Font::System(frame.GetHeight() / Environment().Scale() * 0.9f, FontWeight::Bold), frame, color, "A", TextAlignment::Center);
    }

    // The TV app's Apple logo and "tv".
    static void PaintTvApp(ImDrawList* draw, const ImRect& frame, Rgba color) {
        const Font font = Font::System(frame.GetHeight() / Environment().Scale() * 0.62f, FontWeight::Semibold);
        const float half = frame.GetWidth() * 0.5f;
        Typography::DrawSymbol(draw, Symbols::AppleLogo, font, ImRect(frame.Min.x, frame.Min.y, frame.Min.x + half, frame.Max.y), color, TextAlignment::Trailing);
        Typography::Draw(draw, font, ImRect(frame.Min.x + half, frame.Min.y, frame.Max.x, frame.Max.y), color, "tv");
    }

    // What fills the disk, in the order the bar stacks it (512 Pixels @2x: 39.22 GB of 245.11 GB used).
    static const StorageKind& Kind(int index) {
        static const StorageKind kinds[] = {
            {"Messages", 3.84f, Rgba::Hex(0xFF3B30)},
            {"Applications", 0.78f, Rgba::Hex(0xFF9500)},
            {"System Data", 14.0f, Rgba::Hex(0x7F7F7F)},
            {"macOS", 20.6f, Rgba::Hex(0x9C9C9C)},
        };
        return kinds[index];
    }

    // The disk as a bar: the used kinds side by side with hairlines between them, the free space in gray with its size
    // in the middle; round only at the bar's ends. Dark mode darkens the free space and the hairlines.
    static void PaintStorageBar(ImDrawList* draw, const ImRect& rect) {
        const bool dark = Environment().IsDark();
        const CornerRadii ends(Px(4.0f));
        Draw::FillRoundedRect(draw, rect, ends, dark ? Rgba::White(0.2f) : Rgba::Hex(0xD0D0D0), CornerStyle::Circular);
        float x = rect.Min.x;
        draw->PushClipRect(rect.Min, rect.Max, true);
        for (int i = 0; i < 4; ++i) {
            const float width = rect.GetWidth() * Kind(i).gigabytes / DiskCapacity;
            const ImRect segment(x, rect.Min.y, x + width, rect.Max.y);
            Draw::FillRoundedRect(draw, segment, i == 0 ? CornerRadii(ends.topLeft, 0.0f, 0.0f, ends.bottomLeft) : CornerRadii(0.0f), Kind(i).color, CornerStyle::Circular);
            x = segment.Max.x;
            Draw::FillRect(draw, ImRect(x, rect.Min.y, x + Px(0.5f), rect.Max.y), dark ? Rgba::Black(0.5f) : Rgba::White(0.8f));
            x += Px(0.5f);
        }
        draw->PopClipRect();
        const ImRect free(x, rect.Min.y, rect.Max.x, rect.Max.y);
        Typography::Draw(draw, Font::Style(TextStyle::Subheadline), free, Theme::Colors().label, "205.89 GB", TextAlignment::Center);
    }

    // The volume: its name and use, the bar and the legend of what it holds; the legend keeps only 5 pt under it.
    static void Volume() {
        Section([] {
            Padding(EdgeInsets{1.5f, 0.0f, -3.5f, 0.0f}, [] {
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 8.5f}, [] {
                    HStack({.alignment = VerticalAlignment::FirstTextBaseline}, [] {
                        Text("Macintosh HD");
                        Spacer();
                        Text("39.22 GB of 245.11 GB used", {.foreground = Foreground::Secondary});
                        Button("All Volumes\xE2\x80\xA6");
                    });
                    Background(PaintStorageBar, [] { Frame({.height = 20.5f, .maxWidth = Infinity}, [] {}); });
                    // The legend stands a point closer to the bar than the bar to the title.
                    Padding(EdgeInsets{-1.0f, 0.0f, 0.0f, 0.0f}, [] {
                        HStack({.spacing = 11.0f}, [] {
                            for (int i = 0; i < 4; ++i) {
                                HStack({.spacing = 4.0f}, [&] {
                                    Dot(Kind(i).color, 8.0f);
                                    Text(Kind(i).name, {.font = Font::Style(TextStyle::Footnote), .foreground = Foreground::Secondary});
                                });
                            }
                        });
                    });
                });
            });
        });
    }

    // A recommendation (512 Pixels @2x): the icon, the title over what it does, and the button that does it on the
    // title's line; rows are at least 70 pt tall with their content centered.
    static void Recommendation(const Icon& icon, const char* title, const char* description, const char* action) {
        Frame({.minHeight = 50.0f, .maxWidth = Infinity, .alignment = Alignment{HorizontalAlignment::Leading, VerticalAlignment::Center}}, [&] {
            HStack({.alignment = VerticalAlignment::Top, .spacing = 11.0f}, [&] {
                Padding(EdgeInsets{2.0f, 1.0f, 0.0f, 0.0f}, [&] { Image(icon, ImVec2(26.0f, 26.0f)); });
                // The button on the title's line reaches a point above it, into the row's inset.
                Padding(EdgeInsets{-1.0f, 0.0f, 0.0f, 0.0f}, [&] {
                    HStack({.alignment = VerticalAlignment::FirstTextBaseline}, [&] {
                        Frame({.maxWidth = Infinity, .alignment = Alignment{HorizontalAlignment::Leading, VerticalAlignment::Top}}, [&] {
                            VStack({.alignment = HorizontalAlignment::Leading, .spacing = 2.0f}, [&] {
                                Text(title);
                                Text(description, {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary, .wraps = true});
                            });
                        });
                        Button(action);
                    });
                });
            });
        });
    }

    // A kind of file and the space it takes, with the button that lists it: rows 42 pt tall.
    static void Category(const StorageCategory& category) {
        Frame({.minHeight = 22.0f, .maxWidth = Infinity}, [&] {
            HStack({.spacing = 18.0f}, [&] {
                Image(category.icon, ImVec2(20.0f, 20.0f));
                Text(category.name);
                Spacer();
                HStack({.spacing = 12.5f}, [&] {
                    Text(category.size, {.foreground = Foreground::Secondary});
                    Image(Symbols::InfoCircle, {.font = Font::Style(TextStyle::Body).ImageScale(SymbolScale::Large), .foreground = Foreground::Secondary});
                });
            });
        });
    }

    // Storage, General's page, as 512 Pixels captured it: the volume, the recommendations and what takes the space.
    void StoragePane() {
        static const StorageCategory categories[] = {
            {"Applications", "778.8 MB", {.plate = IconPlate::Gray, .paint = PaintApplications}},
            {"Documents", "10 MB", {Symbols::DocFill, IconPlate::Gray}},
            {"iCloud Drive", "18.7 MB", PaneIcon(Pane::ICloud)},
            {"Messages", "3.84 GB", {Symbols::MessageFill, IconPlate::Green}},
            {"Photos", "33.1 MB", {Symbols::PhotoFill, IconPlate::White}},
            {"System Data", "14.02 GB", {Symbols::Gearshape2Fill, IconPlate::Gray}},
            {"macOS", "20.64 GB", {Symbols::Laptopcomputer, IconPlate::Gray}},
        };
        Form([] {
            Volume();
            Section({.header = "Recommendations"}, [] {
                Recommendation(PaneIcon(Pane::ICloud), "Store in iCloud", "Store all files in iCloud Drive and save space by keeping only recent files on this Mac when storage space is needed.", "Store in iCloud\xE2\x80\xA6");
                Recommendation({.plate = IconPlate::Black, .paint = PaintTvApp}, "Optimize Storage", "Save space by automatically removing movies and TV shows that you\xE2\x80\x99ve already watched from this Mac.", "Optimize\xE2\x80\xA6");
                Recommendation({Symbols::TrashFill, IconPlate::Gray}, "Empty Trash automatically", "Save space by automatically erasing items that have been in the Trash for more than 30 days.", "Turn On\xE2\x80\xA6");
            });
            Section([] {
                for (const StorageCategory& category : categories)
                    Category(category);
            });
        });
    }
} // namespace Examples::Settings
