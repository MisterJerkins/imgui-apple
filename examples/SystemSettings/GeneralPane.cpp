#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    // General: a card with the pane's icon, title and summary, then its pages (General @2x, Ars Technica), glyphs a little
    // smaller than the sidebar's; the rows of rebuilt pages open them.
    void GeneralPane(int* page) {
        Form([&] {
            Section([] {
                Padding(EdgeInsets{16.0f, 20.0f, 13.0f, 20.0f}, [] {
                    Frame({.maxWidth = Infinity}, [] {
                        VStack({.spacing = 8.0f}, [] {
                            Image(PaneIcon(Pane::General), ImVec2(64.0f, 64.0f));
                            VStack({.spacing = 5.0f}, [] {
                                Text("General", {.font = Font::Style(TextStyle::Title).Weight(FontWeight::Heavy), .alignment = TextAlignment::Center});
                                Text("Manage your overall setup and preferences for Mac, such as software updates, device language, AirDrop, and more.", {.font = Font::Style(TextStyle::Callout).WithLineHeight(17.0f), .foreground = Foreground::Secondary, .alignment = TextAlignment::Center, .wraps = true});
                            });
                        });
                    });
                });
            });
            Section([&] {
                if (NavigationLink("About", {.icon = {Symbols::Laptopcomputer, IconPlate::Gray}}))
                    *page = int(Pane::About);
                if (NavigationLink("Software Update", {.icon = {Symbols::GearBadge, IconPlate::Gray, SymbolScale::Large, 0.875f}}))
                    *page = int(Pane::SoftwareUpdate);
                if (NavigationLink("Storage", {.icon = {Symbols::ExternaldriveFill, IconPlate::Gray, SymbolScale::Large, 0.875f}}))
                    *page = int(Pane::Storage);
            });
            Section([] { NavigationLink("AppleCare & Warranty", {.icon = {.plate = IconPlate::White, .paint = PaintAppleCare}}); });
            Section([&] {
                if (NavigationLink("AirDrop & Handoff", {.icon = {.plate = IconPlate::White, .paint = PaintAirDrop}}))
                    *page = int(Pane::AirDropAndHandoff);
            });
            Section([&] {
                NavigationLink("AutoFill & Passwords", {.icon = {Symbols::RectangleAndPencilAndEllipsis, IconPlate::Gray, SymbolScale::Large, 0.88f}});
                if (NavigationLink("Date & Time", {.icon = {Symbols::CalendarBadgeClock, IconPlate::Blue, SymbolScale::Large, 0.84f}}))
                    *page = int(Pane::DateAndTime);
                if (NavigationLink("Language & Region", {.icon = {Symbols::Globe, IconPlate::Blue, SymbolScale::Large, 0.93f}}))
                    *page = int(Pane::LanguageAndRegion);
                if (NavigationLink("Login Items & Extensions", {.icon = {Symbols::ListBullet, IconPlate::Gray}}))
                    *page = int(Pane::LoginItems);
                if (NavigationLink("Sharing", {.icon = {Symbols::Person2BadgeGearshapeFill, IconPlate::Gray}}))
                    *page = int(Pane::Sharing);
                if (NavigationLink("Startup Disk", {.icon = {Symbols::Internaldrive, IconPlate::Gray}}))
                    *page = int(Pane::StartupDisk);
                if (NavigationLink("Time Machine", {.icon = {Symbols::ClockArrowCirclepath, IconPlate::DarkGray}}))
                    *page = int(Pane::TimeMachine);
            });
            Section([&] {
                NavigationLink("Device Management", {.icon = {Symbols::Gear, IconPlate::Gray}});
                if (NavigationLink("Transfer or Reset", {.icon = {Symbols::ArrowCounterclockwise, IconPlate::Gray}}))
                    *page = int(Pane::TransferOrReset);
            });
        });
    }
} // namespace Examples::Settings
