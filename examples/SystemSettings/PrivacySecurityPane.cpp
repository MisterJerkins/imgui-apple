#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    struct PrivacyEntry {
        const char* title;
        Icon icon;
        // How many apps asked for the permission.
        const char* value;
    };

    // The permissions in macOS 15's alphabetical order; the pane opens scrolled past most of them.
    static const PrivacyEntry Permissions[] = {
        {"Accessibility", {Symbols::Accessibility, IconPlate::Blue}, "3"},
        {"App Management", {Symbols::AppBadgeCheckmarkFill, IconPlate::Blue}, "1"},
        {"Automation", {Symbols::Gearshape2Fill, IconPlate::Gray}, "2"},
        {"Bluetooth", {.plate = IconPlate::Blue, .paint = PaintBluetoothRune}, "2"},
        {"Calendars", {Symbols::Calendar, IconPlate::Red}, "1"},
        {"Camera", {Symbols::CameraFill, IconPlate::Green}, "0"},
        {"Contacts", {Symbols::PersonCropCircleFill, IconPlate::Gray}, "1"},
        {"Developer Tools", {Symbols::HammerFill, IconPlate::Gray}, "0"},
        {"Files & Folders", {Symbols::FolderFill, IconPlate::Blue}, "4"},
        {"Focus", {Symbols::MoonFill, IconPlate::Purple}, "0"},
        {"Full Disk Access", {Symbols::ExternaldriveFill, IconPlate::Gray}, "2"},
        {"HomeKit", {Symbols::HouseFill, IconPlate::Orange}, "0"},
        {"Input Monitoring", {Symbols::KeyboardFill, IconPlate::Gray}, "1"},
        {"Local Network", {Symbols::Network, IconPlate::Blue}, "3"},
        {"Location Services", {Symbols::LocationFill, IconPlate::Blue}, "On"},
        {"Media & Apple Music", {Symbols::MusicNote, IconPlate::Pink}, "0"},
        {"Microphone", {Symbols::MicFill, IconPlate::Orange}, "2"},
        {"Passkeys Access for Web Browsers", {Symbols::KeyFill, IconPlate::Gray}, "0"},
        {"Photos", {Symbols::PhotoFill, IconPlate::Orange}, "1"},
        {"Reminders", {Symbols::ListBullet, IconPlate::Blue}, "0"},
        {"Remote Desktop", {Symbols::Display, IconPlate::Blue}, "0"},
        {"Screen & System Audio Recording", {Symbols::RecordCircle, IconPlate::Red}, "10"},
        {"Speech Recognition", {Symbols::Waveform, IconPlate::Gray, SymbolScale::Large, 0.92f}, "0"},
    };

    // A kind of data apps ask for in 15.0: the picture of the app that keeps it in the assets' sidebar folder, the icon
    // without it, and what apps have of it.
    struct DataKind {
        const char* title;
        const char* picture;
        Icon icon;
        const char* access;
    };

    static const DataKind DataKinds[] = {
        {"Calendars", "privacy-calendars", {.symbol = Symbols::Calendar, .plate = IconPlate::White, .color = Rgba::Hex(0xF2433A)}, "None"},
        {"Contacts", "privacy-contacts", {Symbols::PersonCropCircleFill, IconPlate::Gray}, "None"},
        {"Files & Folders", "privacy-files-folders", {Symbols::FolderFill, IconPlate::Blue}, "1 app"},
        {"Full Disk Access", "privacy-full-disk-access", {Symbols::InternaldriveFill, IconPlate::Gray}, "1 full access"},
        {"HomeKit", "privacy-homekit", {Symbols::HouseFill, IconPlate::Orange}, "None"},
        {"Media & Apple Music", "privacy-media-music", {Symbols::MusicNote, IconPlate::Pink}, "None"},
        {"Passkeys Access for Web Browsers", "privacy-passkeys", {Symbols::PersonBadgeKeyFill, IconPlate::Gray}, "None"},
        {"Photos", "privacy-photos", {.symbol = Symbols::PhotoFill, .plate = IconPlate::White, .color = Rgba::Hex(0xF2A33A)}, "None"},
        {"Reminders", "privacy-reminders", {.symbol = Symbols::ListBullet, .plate = IconPlate::White, .color = Rgba::Hex(0x3A8EF6)}, "None"},
    };

    struct PrivacySecurityModel {
        int allowedApps = 1;
    };

    // Privacy as 512 Pixels captured it on 15.0: the pane's header, Location Services, and the kinds of data, each row
    // 52 pt with the picture at the row inset and the title 10 pt after it, 1 pt over the access in Callout.
    static void PrivacyByData() {
        Form([] {
            Section([] {
                Label("Privacy", {.description = "Control which apps can access your data, location, camera, and microphone, and manage safety protections. [Learn more\xE2\x80\xA6](help:privacy)", .icon = PaneIcon(Pane::PrivacyAndSecurity)});
            });
            Section([] { NavigationLink("Location Services", {.icon = {Symbols::LocationFill, IconPlate::Blue}, .value = "3"}); });
            Section([] {
                for (const DataKind& kind : DataKinds) {
                    NavigationLink(kind.title, [&] {
                        HStack({.spacing = 10.0f}, [&] {
                            Image(PictureIcon(std::string("sidebar/") + kind.picture, kind.icon), ImVec2(20.0f, 20.0f));
                            VStack({.alignment = HorizontalAlignment::Leading, .spacing = 1.0f}, [&] {
                                Text(kind.title);
                                Text(kind.access, {.font = Font::Style(TextStyle::Callout), .foreground = Foreground::Secondary});
                            });
                        });
                    });
                }
            });
        });
    }

    // Privacy & Security with an app Gatekeeper blocked, as in Apple's article on opening it anyway (Privacy &
    // Security @2x, scrolled to the Security section).
    void PrivacySecurityPane(const Mac& mac) {
        if (mac.privacyByData) {
            PrivacyByData();
            return;
        }
        static PrivacySecurityModel model;
        Form([] {
            Section([] {
                for (const PrivacyEntry& entry : Permissions)
                    NavigationLink(entry.title, {.icon = entry.icon, .value = entry.value});
            });
            Section([] { NavigationLink("Sensitive Content Warning", {.icon = {Symbols::EyeTrianglebadgeExclamationmarkFill, IconPlate::Blue, SymbolScale::Large, 0.7f}, .value = "On"}); });
            Section([] {
                NavigationLink("Analytics & Improvements", {.icon = {Symbols::ChartBarXaxis, IconPlate::Blue, SymbolScale::Large, 0.93f}});
                NavigationLink("Apple Advertising", {.icon = {Symbols::MegaphoneFill, IconPlate::Blue, SymbolScale::Large, 0.85f}});
            });
            Section({.header = "Security"}, [] { Picker("Allow applications from", &model.allowedApps, {"App Store", "App Store & Known Developers"}); });
            Section([] {
                LabeledContent("“Example App” was blocked to protect your Mac.", [] { Button("Open Anyway"); });
                Text("Apple could not verify “Example App” is free of malware that may harm your Mac or compromise your privacy.", {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary, .wraps = true});
            });
            Section([] {
                NavigationLink("FileVault", {.icon = {.plate = IconPlate::Gray, .paint = PaintFileVault}, .value = "On"});
                NavigationLink("Lockdown Mode", {.icon = PaneIcon(Pane::PrivacyAndSecurity), .value = "Off"});
            });
            // The buttons keep 20 pt from the last section, the help button 10 pt after Advanced….
            TrailingButtons({.top = 10.0f, .spacing = 10.0f}, [] {
                Button("Advanced…");
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
