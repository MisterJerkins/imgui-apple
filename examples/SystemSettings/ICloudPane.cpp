#include "Settings.h"

#include <string>

namespace Examples::Settings {
    using namespace Cupertino;

    // Something kept in iCloud: its app's picture in the assets' sidebar folder, the icon without it, and how much of it
    // is there.
    struct SavedItem {
        const char* title;
        const char* picture;
        Icon icon;
        const char* amount;
    };

    static const SavedItem SavedItems[] = {
        {"Photos", "icloud-photos", {.symbol = Symbols::PhotoFill, .plate = IconPlate::White, .color = Rgba::Hex(0xF2A33A)}, "46 Items"},
        {"Drive", "icloud-drive", {.symbol = Symbols::IcloudFill, .plate = IconPlate::White, .color = Rgba::Hex(0x3A8EF6)}, "On"},
        {"Passwords", "icloud-passwords", {Symbols::KeyFill, IconPlate::Gray}, "On"},
        {"Notes", "icloud-notes", {Symbols::NoteText, IconPlate::Yellow}, "1.9 MB"},
        {"Messages", "icloud-messages", {Symbols::MessageFill, IconPlate::Green}, "Off"},
        {"Mail", "icloud-mail", {Symbols::EnvelopeFill, IconPlate::Blue}, "2.1 KB"},
    };

    // What iCloud+ adds: its picture and what it does; Private Relay breaks its line as the capture does, without pulling
    // "on" down to "Safari".
    struct Feature {
        const char* title;
        const char* picture;
        unsigned symbol;
        const char* detail;
    };

    static const Feature Features[] = {
        {"Up to 12 TB storage", "icloud-storage", Symbols::IcloudFill, "More space for you and your family"},
        {"Share with your family", "icloud-family", Symbols::Person2Fill, "Share an iCloud plan"},
        {"Private Relay", "icloud-private-relay", Symbols::NetworkBadgeShieldHalfFilled, "Protect browsing on\nSafari"},
        {"Hide My Email", "icloud-hide-my-email", Symbols::EnvelopeFill, "Keep your email address private"},
    };

    // A title with a line of secondary text under it, and a button on the title's line.
    static void TitledRow(const std::function<void()>& title, const char* detail, const TextOptions& detail_style, const char* button) {
        HStack({.alignment = VerticalAlignment::FirstTextBaseline}, [&] {
            VStack({.alignment = HorizontalAlignment::Leading, .spacing = 1.5f}, [&] {
                title();
                if (detail)
                    Text(detail, detail_style);
            });
            Spacer();
            Button(button);
        });
    }

    // How much of the storage is used: a capsule 8 pt tall, white or in dark mode the level track, filled in green from
    // its start.
    static void StorageBar(float used) {
        Background([used](ImDrawList* draw, const ImRect& rect) {
            Draw::FillCapsule(draw, rect, Environment().IsDark() ? Theme::Colors().levelIndicatorTrack : Rgba::White(1.0f));
            Draw::FillCapsule(draw, ImRect(rect.Min, ImVec2(rect.Min.x + rect.GetWidth() * used, rect.Max.y)), Theme::SystemGreen());
        }, [] { Frame({.height = 8.0f, .maxWidth = Infinity}, [] {}); });
    }

    // A tile of what is saved: in a tile box 139.5 by 49.5 pt, the picture 10 pt in and the title 6 pt after it, 1.5 pt
    // over the amount in Subheadline.
    static void SavedTile(const SavedItem& item) {
        Background(DrawTileBox, [&] {
            Frame({.width = 139.5f, .height = 49.5f, .alignment = {HorizontalAlignment::Leading, VerticalAlignment::Top}}, [&] {
                Padding(EdgeInsets{10.0f, 10.0f, 0.0f, 0.0f}, [&] {
                    HStack({.alignment = VerticalAlignment::Top, .spacing = 6.0f}, [&] {
                        Image(PictureIcon(std::string("sidebar/") + item.picture, item.icon), ImVec2(20.0f, 20.0f));
                        VStack({.alignment = HorizontalAlignment::Leading, .spacing = 1.5f}, [&] {
                            Text(item.title);
                            Text(item.amount, {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary});
                        });
                    });
                });
            });
        });
    }

    // A feature of iCloud+: the picture, and 7 pt after it the title 1.5 pt over what it does in Subheadline, wrapped at
    // 126 pt ("More space for you and / your family" @2x).
    static void FeatureItem(const Feature& feature) {
        HStack({.alignment = VerticalAlignment::Top, .spacing = 7.0f}, [&] {
            Image(PictureIcon(std::string("sidebar/") + feature.picture, {.symbol = feature.symbol, .color = Theme::SystemBlue()}), ImVec2(20.0f, 20.0f));
            VStack({.alignment = HorizontalAlignment::Leading, .spacing = 1.5f}, [&] {
                Text(feature.title);
                Frame({.width = 126.0f, .alignment = {HorizontalAlignment::Leading, VerticalAlignment::Top}}, [&] {
                    Text(feature.detail, {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary, .wraps = true});
                });
            });
        });
    }

    // Internet Accounts > iCloud as 512 Pixels captured it on 15.0: the account, its storage and the upgrade, what is
    // saved to iCloud in tiles, and what iCloud+ adds.
    void ICloudPane(const Mac& mac) {
        const TextOptions heading = {.font = Font::Style(TextStyle::Body).Weight(FontWeight::Semibold)};
        const TextOptions subheadline = {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary};
        const TextOptions footnote = {.font = Font::Style(TextStyle::Footnote), .foreground = Foreground::Secondary};
        Form([&] {
            // The page's title and the account over the sections, in Title2 Bold at the row inset, 9 pt lower than a first
            // section and 7 pt apart from the next.
            Padding(EdgeInsets{9.0f, 10.0f, 7.0f, 10.0f}, [&] {
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 0.0f}, [&] {
                    const Font title = Font::Style(TextStyle::Title2).Weight(FontWeight::Bold);
                    Text("iCloud", {.font = title});
                    Text(mac.accountName, {.font = title, .foreground = Foreground::Secondary});
                });
            });
            Section([&] {
                HStack({.alignment = VerticalAlignment::FirstTextBaseline}, [&] {
                    // A capture's mosaic over the email address is a little taller than its line.
                    const Bitmap hidden_email = mac.hiddenEmail ? Picture(std::string("pictures/") + mac.hiddenEmail + "-icloud") : Bitmap{};
                    VStack({.alignment = HorizontalAlignment::Leading, .spacing = 0.0f}, [&] {
                        Text(mac.accountName);
                        Frame({.height = 16.0f, .alignment = {HorizontalAlignment::Leading, VerticalAlignment::Top}}, [&] {
                            if (hidden_email.IsEmpty())
                                Text(mac.email, {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary});
                            else
                                Image(hidden_email, ImVec2(113.0f, 19.0f));
                        });
                    });
                    Spacer();
                    Button("Details\xE2\x80\xA6");
                });
            });
            Section([&] {
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 8.0f}, [&] {
                    TitledRow([] { Text("Storage: 540.9 MB of 5 GB Used", {.foreground = Foreground::Secondary}); }, nullptr, {}, "Manage\xE2\x80\xA6");
                    StorageBar(540.9f / 5120.0f);
                });
                TitledRow([] { Text("Upgrade to iCloud+"); }, "Access to premium features and more space", footnote, "Upgrade\xE2\x80\xA6");
            });
            Section([&] {
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 8.5f}, [&] {
                    TitledRow([&] {
                        HStack({.spacing = 6.5f}, [&] {
                            Dot(Theme::SystemGreen(), 10.0f);
                            Text("Saved to iCloud", heading);
                        });
                    }, nullptr, {}, "See All");
                    VStack({.spacing = 10.0f}, [&] {
                        for (int row = 0; row < 2; ++row) {
                            HStack({.spacing = 10.0f}, [&] {
                                for (int column = 0; column < 3; ++column)
                                    SavedTile(SavedItems[row * 3 + column]);
                            });
                        }
                    });
                });
            });
            Section([&] {
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 10.0f}, [&] {
                    TitledRow([&] { Text("Get More with iCloud+", heading); }, "More Storage and Powerful Features", subheadline, "Upgrade to iCloud+");
                    // The features stand in two columns of 224.5 pt, 9.5 pt in and 6.5 pt lower, each centered on its row;
                    // rows 73 pt apart.
                    Padding(EdgeInsets{6.5f, 9.5f, 0.0f, 0.0f}, [&] {
                        VStack({.alignment = HorizontalAlignment::Leading, .spacing = 27.5f}, [&] {
                            for (int row = 0; row < 2; ++row) {
                                HStack({.spacing = 0.0f}, [&] {
                                    for (int column = 0; column < 2; ++column)
                                        Frame({.width = 224.5f, .alignment = {HorizontalAlignment::Leading, VerticalAlignment::Center}}, [&] { FeatureItem(Features[row * 2 + column]); });
                                });
                            }
                        });
                    });
                });
            });
        });
    }
} // namespace Examples::Settings
