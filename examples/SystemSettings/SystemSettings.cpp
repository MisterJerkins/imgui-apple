#include "Settings.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace Examples::Settings {
    using namespace Cupertino;

    // The pane shown and the history behind the toolbar's back and forward buttons; the window opens with General
    // behind it, as the references show the back button enabled.
    struct SettingsModel {
        int pane = int(Pane::Appearance);
        std::vector<int> back = {int(Pane::General)};
        std::vector<int> forward;
        std::string search;
    };

    static SettingsModel& Model() {
        static SettingsModel model;
        return model;
    }

    // Opens a pane, keeping the one it leaves for the back button.
    static void Navigate(int pane) {
        SettingsModel& model = Model();
        if (pane == model.pane)
            return;
        model.back.push_back(model.pane);
        model.forward.clear();
        model.pane = pane;
    }

    static void Step(NavigationStep step) {
        SettingsModel& model = Model();
        std::vector<int>& from = step == NavigationStep::Back ? model.back : model.forward;
        std::vector<int>& to = step == NavigationStep::Back ? model.forward : model.back;
        if (step == NavigationStep::None || from.empty())
            return;
        to.push_back(model.pane);
        model.pane = from.back();
        from.pop_back();
    }

    SidebarEntry Row(const char* title, Pane pane) {
        return {title, pane, PaneIcon(pane)};
    }

    const Mac& MacBookAir() {
        static const Mac mac = {
            .windowSize = ImVec2(715.0f, 625.0f),
            .accountName = "John Appleseed",
            .accountSubtitle = "Apple Account",
            .initials = "JA",
            .accountWeight = FontWeight::Semibold,
            .familyRow = false,
            .groups = {
                {
                    Row("Wi-Fi", Pane::WiFi),
                    Row("Bluetooth", Pane::Bluetooth),
                    Row("Network", Pane::Network),
                    Row("Battery", Pane::Battery),
                    Row("VPN", Pane::Vpn),
                },
                {
                    Row("General", Pane::General),
                    Row("Accessibility", Pane::Accessibility),
                    Row("Appearance", Pane::Appearance),
                    Row("Control Center", Pane::ControlCenter),
                    Row("Desktop & Dock", Pane::DesktopAndDock),
                    Row("Displays", Pane::Displays),
                    Row("Screen Saver", Pane::ScreenSaver),
                    Row("Siri", Pane::Siri),
                    Row("Wallpaper", Pane::Wallpaper),
                },
                {
                    Row("Notifications", Pane::Notifications),
                    Row("Sound", Pane::Sound),
                    Row("Focus", Pane::Focus),
                    Row("Screen Time", Pane::ScreenTime),
                },
                {
                    Row("Lock Screen", Pane::LockScreen),
                    Row("Privacy & Security", Pane::PrivacyAndSecurity),
                    Row("Touch ID & Password", Pane::TouchIdAndPassword),
                    Row("Users & Groups", Pane::UsersAndGroups),
                },
                {
                    Row("Internet Accounts", Pane::InternetAccounts),
                    Row("Game Center", Pane::GameCenter),
                    Row("Wallet & Apple Pay", Pane::WalletAndApplePay),
                },
                {
                    Row("Keyboard", Pane::Keyboard),
                    Row("Mouse", Pane::Mouse),
                    Row("Trackpad", Pane::Trackpad),
                    Row("Printers & Scanners", Pane::PrintersAndScanners),
                },
            },
            .historyButtons = true,
            .accentTitle = "Accent color",
            .highlightTitle = "Highlight color",
            .accentItem = "Accent Color",
            .multicolorName = "Multicolor",
            .british = false,
            .bluetooth ={.paired = {{"John\xE2\x80\x99s Magic Mouse", "magic-mouse", 57}}},
        };
        return mac;
    }

    // Pages under a pane: the toolbar shows their title with their pane selected in the sidebar.
    struct Page {
        const char* title;
        Pane pane;
        Pane parent;
    };

    static const Page Pages[] = {
        {"Display", Pane::AccessibilityDisplay, Pane::Accessibility},
        {"Sharing", Pane::Sharing, Pane::General},
        {"Software Update", Pane::SoftwareUpdate, Pane::General},
        {"Storage", Pane::Storage, Pane::General},
        {"About", Pane::About, Pane::General},
        {"AirDrop & Handoff", Pane::AirDropAndHandoff, Pane::General},
        {"Date & Time", Pane::DateAndTime, Pane::General},
        {"Startup Disk", Pane::StartupDisk, Pane::General},
        {"Time Machine", Pane::TimeMachine, Pane::General},
        {"Login Items & Extensions", Pane::LoginItems, Pane::General},
        {"Language & Region", Pane::LanguageAndRegion, Pane::General},
        {"Transfer or Reset", Pane::TransferOrReset, Pane::General},
        {"iCloud", Pane::InternetAccountsICloud, Pane::InternetAccounts},
    };

    static const Page* PageOf(int pane) {
        const auto page = std::find_if(std::begin(Pages), std::end(Pages), [&](const Page& entry) { return int(entry.pane) == pane; });
        return page != std::end(Pages) ? page : nullptr;
    }

    static void Sidebar(const Mac& mac, bool update_available) {
        SettingsModel& model = Model();
        SidebarSearchField(&model.search);
        // A pane's pages keep the pane selected.
        const Page* page = PageOf(model.pane);
        const int shown = page ? int(page->parent) : model.pane;
        int selection = shown;
        WithBadgeProminence(BadgeProminence::Increased, [&] { List([&] {
            const Bitmap picture = mac.accountPicture ? Picture(std::string("sidebar/") + mac.accountPicture) : Bitmap{};
            if (SidebarAccount(mac.accountName, mac.accountSubtitle, {.initials = mac.initials, .picture = picture, .nameWeight = mac.accountWeight, .selected = selection == int(Pane::AppleAccount)}))
                selection = int(Pane::AppleAccount);
            if (mac.familyRow)
                NavigationLink("Family", &selection, int(Pane::Family), {.icon = PaneIcon(Pane::Family)});
            if (update_available)
                Section([&] { NavigationLink("Software Update Available", &selection, int(Pane::SoftwareUpdate), {.badge = 1}); });
            for (const std::vector<SidebarEntry>& group : mac.groups) {
                Section([&] {
                    for (const SidebarEntry& entry : group)
                        NavigationLink(entry.title, &selection, int(entry.pane), {.icon = entry.icon});
                });
            }
        }); });
        if (selection != shown)
            Navigate(selection);
    }

    // The toolbar shows the title of the selected sidebar row or of a pane's page.
    static const char* PaneTitle(const Mac& mac, int pane) {
        // General's card carries its title.
        if (pane == int(Pane::General))
            return nullptr;
        if (const Page* page = PageOf(pane))
            return page->title;
        if (pane == int(Pane::PrivacyAndSecurity))
            return "Privacy & Security";
        if (pane == int(Pane::AppleAccount))
            return "Apple Account";
        if (pane == int(Pane::AirPods))
            return "AirPods Pro";
        for (const std::vector<SidebarEntry>& group : mac.groups) {
            for (const SidebarEntry& entry : group) {
                if (int(entry.pane) == pane)
                    return entry.title;
            }
        }
        return pane == int(Pane::Family) ? "Family" : "";
    }

    // A pane that opens one of its pages when a row is clicked.
    static void PaneWithPages(int pane, void (*content)(int* page)) {
        int page = pane;
        content(&page);
        if (page != pane)
            Navigate(page);
    }

    // Panes that are not rebuilt yet stay empty, as System Settings looks while a pane loads.
    static void Detail(const Mac& mac, AppearancePreferences& preferences, int pane) {
        switch (Pane(pane)) {
            case Pane::Appearance:
                AppearancePane(mac, preferences);
                break;
            case Pane::DesktopAndDock:
                DesktopAndDockPane(mac);
                break;
            case Pane::Accessibility:
                PaneWithPages(pane, AccessibilityPane);
                break;
            case Pane::AccessibilityDisplay:
                DisplayPane();
                break;
            case Pane::Battery:
                BatteryPane(mac.battery);
                break;
            case Pane::Bluetooth:
                BluetoothPane(mac);
                break;
            case Pane::Sharing:
                SharingPane(mac);
                break;
            case Pane::SoftwareUpdate:
                SoftwareUpdatePane();
                break;
            case Pane::Storage:
                StoragePane();
                break;
            case Pane::About:
                AboutPane(mac);
                break;
            case Pane::PrivacyAndSecurity:
                PrivacySecurityPane(mac);
                break;
            case Pane::WiFi:
                WiFiPane(mac);
                break;
            case Pane::AirPods:
                AirPodsPane();
                break;
            case Pane::Sound:
                SoundPane();
                break;
            case Pane::Keyboard:
                KeyboardPane();
                break;
            case Pane::Trackpad:
                TrackpadPane();
                break;
            case Pane::Mouse:
                MousePane();
                break;
            case Pane::Displays:
                DisplaysPane(mac);
                break;
            case Pane::PrintersAndScanners:
                PrintersPane();
                break;
            case Pane::InternetAccounts:
                PaneWithPages(pane, InternetAccountsPane);
                break;
            case Pane::InternetAccountsICloud:
                ICloudPane(mac);
                break;
            case Pane::ControlCenter:
                ControlCenterPane(mac);
                break;
            case Pane::Spotlight:
                SpotlightPane(mac);
                break;
            case Pane::Network:
                NetworkPane();
                break;
            case Pane::Vpn:
                VpnPane();
                break;
            case Pane::Focus:
                FocusPane();
                break;
            case Pane::ScreenTime:
                ScreenTimePane();
                break;
            case Pane::LockScreen:
                LockScreenPane(mac);
                break;
            case Pane::UsersAndGroups:
                UsersPane(mac);
                break;
            case Pane::Notifications:
                NotificationsPane();
                break;
            case Pane::Siri:
                SiriPane();
                break;
            case Pane::TouchIdAndPassword:
                TouchIdPane();
                break;
            case Pane::AirDropAndHandoff:
                AirDropPane();
                break;
            case Pane::DateAndTime:
                DateTimePane();
                break;
            case Pane::StartupDisk:
                StartupDiskPane();
                break;
            case Pane::TimeMachine:
                TimeMachinePane();
                break;
            case Pane::LoginItems:
                LoginItemsPane();
                break;
            case Pane::LanguageAndRegion:
                LanguageRegionPane();
                break;
            case Pane::TransferOrReset:
                TransferPane();
                break;
            case Pane::AppleAccount:
                AppleAccountPane(mac);
                break;
            case Pane::WalletAndApplePay:
                WalletPane();
                break;
            case Pane::GameCenter:
                GameCenterPane(mac);
                break;
            case Pane::Wallpaper:
                WallpaperPane();
                break;
            case Pane::ScreenSaver:
                ScreenSaverPane();
                break;
            case Pane::General:
                PaneWithPages(pane, GeneralPane);
                break;
            default:
                Form([] {});
                break;
        }
    }

    // "Desktop & Dock" -> "desktop-dock": lowercase words joined by dashes.
    static std::string Slug(std::string_view title) {
        std::string slug;
        for (const char c : title) {
            if (std::isalnum(static_cast<unsigned char>(c)))
                slug.push_back(char(std::tolower(static_cast<unsigned char>(c))));
            else if (!slug.empty() && slug.back() != '-')
                slug.push_back('-');
        }
        while (!slug.empty() && slug.back() == '-')
            slug.pop_back();
        return slug;
    }

    bool SelectPane(const Mac& mac, std::string_view slug) {
        const auto select = [](Pane pane) {
            Model().pane = int(pane);
            return true;
        };
        if (slug == "apple-account")
            return select(Pane::AppleAccount);
        // The iCloud page of Internet Accounts shares its title with the iCloud row of the sidebar.
        if (slug == "internet-accounts-icloud")
            return select(Pane::InternetAccountsICloud);
        for (const Page& page : Pages) {
            if (Slug(page.title) == slug)
                return select(page.pane);
        }
        for (const std::vector<SidebarEntry>& group : mac.groups) {
            for (const SidebarEntry& entry : group) {
                if (Slug(entry.title) == slug)
                    return select(entry.pane);
            }
        }
        return false;
    }

    // The battery symbol nearest a charge: empty, a quarter, half, three quarters or full.
    static unsigned BatterySymbol(int level) {
        static const unsigned Quarters[] = {Symbols::Battery0percent, Symbols::Battery25percent, Symbols::Battery50percent, Symbols::Battery75percent, Symbols::Battery100percent};
        return Quarters[ImClamp((level + 12) / 25, 0, 4)];
    }
} // namespace Examples::Settings

namespace Examples {
    using namespace Cupertino;
    using namespace Settings;

    void SystemSettings(bool* open, const SystemSettingsOptions& options) {
        static AppearancePreferences own_preferences;
        const Mac& mac = options.mac ? *options.mac : MacBookAir();
        AppearancePreferences& preferences = options.appearance ? *options.appearance : own_preferences;
        const int pane = Model().pane;
        // As on macOS the window only gets taller or shorter.
        const ImVec2 size = options.placement.SizeOr(mac.windowSize);
        const WindowOptions window = {.size = size, .position = options.placement.position, .minSize = ImVec2(size.x, 400.0f), .maxSize = ImVec2(size.x, 10000.0f)};
        Window("System Settings", open, window, [&] {
            const SettingsModel& model = Model();
            NavigationSplitViewOptions split = {.title = PaneTitle(mac, pane), .showsHistoryButtons = mac.historyButtons, .canGoBack = !model.back.empty(), .canGoForward = !model.forward.empty()};
            const std::string battery_level = "Battery Level: " + std::to_string(mac.battery.level) + "%";
            const auto mouse = std::find_if(mac.bluetooth.paired.begin(), mac.bluetooth.paired.end(), [](const BluetoothDevice& device) { return std::string_view(device.picture) == "magic-mouse"; });
            const std::string mouse_charge = mouse != mac.bluetooth.paired.end() ? std::to_string(mouse->charge) + "%" : "";
            if (pane == int(Pane::Battery))
                split.subtitle = {{BatterySymbol(mac.battery.level), battery_level.c_str()}};
            if (pane == int(Pane::Mouse) && !mouse_charge.empty())
                split.subtitle = {{BatterySymbol(mouse->charge), mouse_charge.c_str()}};
            if (pane == int(Pane::AirPods))
                split.subtitle = {{Symbols::Airpodspro, "100%"}, {Symbols::AirpodsproChargingcaseWirelessFill, "100%"}};
            Step(NavigationSplitView(split, [&] { Sidebar(mac, options.updateAvailable); }, [&] { Id(pane, [&] { Detail(mac, preferences, pane); }); }));
        });
    }
} // namespace Examples
