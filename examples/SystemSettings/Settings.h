#pragma once

#include "Assets/Assets.h"
#include "Examples.h"

#include "Cupertino.h"

#include <array>
#include <functional>
#include <span>
#include <string_view>
#include <vector>

// System Settings of macOS 15: its panes, each rebuilt after Apple's screenshots, and the Mac whose settings they show.
namespace Examples::Settings {
    enum class Pane {
        WiFi,
        Bluetooth,
        Network,
        Battery,
        Vpn,
        General,
        Accessibility,
        Appearance,
        ControlCenter,
        DesktopAndDock,
        Displays,
        ScreenSaver,
        Siri,
        Wallpaper,
        Notifications,
        Sound,
        Focus,
        ScreenTime,
        Family,
        LockScreen,
        PrivacyAndSecurity,
        TouchIdAndPassword,
        LoginPassword,
        UsersAndGroups,
        InternetAccounts,
        GameCenter,
        ICloud,
        WalletAndApplePay,
        AirPods,
        Keyboard,
        Mouse,
        Trackpad,
        PrintersAndScanners,
        Spotlight,
        // Pages of General, shown with General selected in the sidebar.
        Sharing,
        SoftwareUpdate,
        Storage,
        About,
        AirDropAndHandoff,
        DateAndTime,
        StartupDisk,
        TimeMachine,
        LoginItems,
        LanguageAndRegion,
        TransferOrReset,
        // The account row at the top of the sidebar.
        AppleAccount,
        // Pages of Accessibility, shown with Accessibility selected in the sidebar.
        AccessibilityDisplay,
        // The iCloud account of Internet Accounts.
        InternetAccountsICloud,
    };

    struct SidebarEntry {
        const char* title;
        Pane pane;
        Cupertino::Icon icon;
    };

    // Lock Screen as the Mac has it: minutes before the screen saver starts and the display turns off (0 is never), the
    // password delay and the large clock (items of their pop-ups), and whether iPhone Mirroring signs in by itself.
    struct LockScreenPreferences {
        int screenSaver = 20;
        int displayOnBattery = 2;
        int displayOnAdapter = 10;
        int requirePassword = 3;
        int largeClock = 1;
        bool userNameAndPhoto = true;
        bool mirroringSignsIn = false;
    };

    // A Wi-Fi network in range: its name and its signal in bars, 1 to 3.
    struct WiFiNetwork {
        const char* name;
        int bars = 3;
    };

    // Battery as the Mac has it: its charge, energy modes, when it was last charged, and its last 24 hours from their first
    // hour (the level every 15 minutes, the charging slots, the screen use per hour). The charts label every third hour
    // (15.1 marks the first with its half of the day, 15.0 only noon and midnight) and date the day that begins in them.
    struct BatteryPreferences {
        int level = 100;
        bool energyModes = true;
        const char* lastCharged = "Today, 9:41 AM";
        int startHour = 15;
        std::vector<int> levels = std::vector<int>(91, 100);
        std::pair<int, int> charging = {0, 91};
        std::vector<float> screenMinutes = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 55, 34};
        std::array<const char*, 8> hours = {"3\xE2\x80\xAF" "P", "6", "9", "12\xE2\x80\xAF" "A", "3", "6", "9", "12\xE2\x80\xAF" "P"};
        const char* newDay = nullptr;
    };

    // A Bluetooth device: its name and picture in the assets' pictures folder; a paired one shows the charge of its battery.
    struct BluetoothDevice {
        const char* name;
        const char* picture;
        int charge = 0;
    };

    // Bluetooth as the Mac has it: its name as the build quotes it (15.0 in straight quotes), the devices paired with it
    // and those nearby; with none nearby the pane is searching.
    struct BluetoothPreferences {
        const char* macName = "\"John's MacBook Air\"";
        std::vector<BluetoothDevice> paired;
        std::vector<BluetoothDevice> nearby = {{"John's AirPods Pro", "airpods-pro"}};
    };

    // The Mac whose settings the window shows: its account and devices, the sidebar's order and wording of its macOS
    // version and language, and what its panes are set to.
    struct Mac {
        ImVec2 windowSize;
        const char* accountName;
        const char* accountSubtitle;
        const char* initials;
        Cupertino::FontWeight accountWeight;
        // The account's picture in the assets' sidebar folder: in the sidebar, on its Apple Account page and in Users &
        // Groups; the initials stand in without them.
        const char* accountPicture = nullptr;
        const char* accountPictureLarge = nullptr;
        const char* userPicture = nullptr;
        const char* email = "john.appleseed@icloud.com";
        // A capture's pixelated email in the assets' pictures folder, drawn in the email's place: this name on the Apple
        // Account page, with -icloud on the iCloud page.
        const char* hiddenEmail = nullptr;
        // What the Mac and the account's iPad are called, and the account's Game Center nickname.
        const char* computerName = "John\xE2\x80\x99s MacBook Air";
        const char* ipadName = "John\xE2\x80\x99s iPad";
        const char* nickname = "johnappleseed";
        // The serial number About shows; null hides it under tiles as captures do.
        const char* serialNumber = "FVFGK2ABQ6L4";
        bool familyRow;
        std::vector<std::vector<SidebarEntry>> groups;
        bool historyButtons;
        const char* accentTitle;
        const char* highlightTitle;
        const char* accentItem;
        const char* multicolorName;
        // The Wi-Fi network the Mac is connected to and its private address (Off, Fixed or Rotating), the networks in range
        // it knows and the others.
        const char* network = "Home";
        int privateAddress = 2;
        std::vector<WiFiNetwork> knownNetworks;
        std::vector<WiFiNetwork> otherNetworks = {{"Guest"}, {"Studio", 1}, {"Library-5G", 2}, {"Office"}};
        // Desktop & Dock: British spelling ("Minimise", "Full-colour") as on Apple's art, the Dock's size on its slider
        // and whether opening applications bounce.
        bool british = true;
        float dockSize = 0.097f;
        bool animateOpening = false;
        // Spotlight: the categories left in search results; all of them when empty.
        std::vector<const char*> spotlightResults;
        LockScreenPreferences lockScreen;
        BluetoothPreferences bluetooth;
        BatteryPreferences battery;
        // Privacy & Security as 15.0 lists it: the kinds of data with the picture of their app and what apps have them,
        // rather than every permission with its count.
        bool privacyByData = false;
        // Sharing: the name the Mac has on the local network, and whether File Sharing is on.
        const char* hostname = "Johns-MacBook-Air.local";
        bool fileSharing = true;
    };

    // A MacBook Air on macOS 15.0 with Apple's sidebar, the window the example opens without a Mac of its own.
    const Mac& MacBookAir();
    // A sidebar row with its pane's macOS 15 icon.
    SidebarEntry Row(const char* title, Pane pane);
    // Opens the pane of a sidebar row or a pane's page by its title in lowercase words joined by dashes ("desktop-dock");
    // false when the Mac has none of that name.
    bool SelectPane(const Mac& mac, std::string_view slug);

    // The macOS 15 icon of a pane, glyph sizes measured on the native @2x sidebars (SidebarIcons.cpp).
    const Cupertino::Icon& PaneIcon(Pane pane);
    // A row that opens a page with one of macOS's pictures for its icon, from the assets' sidebar folder, or fallback
    // without it (Rows.cpp).
    bool PictureLink(std::string_view title, const char* picture, const Cupertino::Icon& fallback, Cupertino::NavigationLinkOptions options = {});
    // Buttons at the trailing end of a row of their own, as a pane ends in its help button: room over, under and after
    // the row, and between the buttons, in points (Rows.cpp).
    struct TrailingRow {
        float top = 0.0f;
        float bottom = 0.0f;
        float trailing = 0.0f;
        float spacing = 8.0f;
    };
    void TrailingButtons(const TrailingRow& row, const std::function<void()>& buttons);
    void TrailingButtons(const std::function<void()>& buttons);
    // The borderless info.circle button at the end of a row, identified by the row's name; true when clicked.
    bool InfoButton(std::string_view name);
    // A mini switch and the button it enables, 10.5 pt apart (AirDrop's password, Lock Screen's message @2x); true when
    // the button is clicked.
    bool SwitchWithButton(const char* id, bool* on, const char* button);
    // A dot of color diameter points across (a legend's key).
    void Dot(Cupertino::Rgba color, float diameter);
    // A state after a dot of its color (green connected, gray inactive, red not connected) in secondary text of style:
    // Footnote under a printer, Subheadline under a service (Network, Wi-Fi details), Body under the Wi-Fi pane's network.
    void ServiceStatus(Cupertino::Rgba color, const char* state, Cupertino::TextStyle style = Cupertino::TextStyle::Subheadline);
    struct ServiceState {
        Cupertino::Rgba color;
        const char* text = nullptr;
    };
    // A service turned on by a mini switch, 50 pt with everything on its middle (Sharing @2x): the plate where a form row's
    // large icon stands, the title 11 pt after it (over its state), and a point and a half lower the switch with the info
    // button 9 pt after it; true when the info button is clicked.
    bool SwitchRow(const char* title, const Cupertino::Icon& icon, bool* on, const ServiceState* state = nullptr);
    // The icon a Mac's sidebar gives a pane: Apple Intelligence & Siri draws its ring from 15.1 on.
    Cupertino::Icon SidebarIcon(const Mac& mac, Pane pane);

    // The galleries of Wallpaper and Screen Saver (WallpaperGallery.cpp). A picture is a pane's photo in the assets'
    // wallpapers folder, a gradient of its own without it; a picture still to download ends its name in an arrow.
    void DrawWallpaper(ImDrawList* draw, const ImRect& rect, const char* pane, const char* picture, float radius);
    struct WallpaperItem {
        const char* name;
        const char* picture;
        bool download = false;
    };
    // How a pane draws its thumbnails: its pictures, the photos' width and how far the first stands in from the margin
    // (the panes differ by a point, @2x).
    struct WallpaperGallery {
        const char* pane;
        float photoWidth;
        float leading;
    };
    // A category: its title with Show All and the count, then a row of thumbnails the window cuts off with their names
    // under them, the selected one ringed.
    void WallpaperCategory(const WallpaperGallery& gallery, const char* title, int count, std::span<const WallpaperItem> items, int selected = -1);
    // The first landscapes and cityscapes, which Wallpaper and Screen Saver both show.
    std::span<const WallpaperItem> Landscapes();
    std::span<const WallpaperItem> Cityscapes();

    // Sidebar glyphs SF Symbols does not include, painted in the symbol frame of their plate (SidebarIcons.cpp).
    void PaintBluetoothRune(ImDrawList* draw, const ImRect& frame, Cupertino::Rgba color);
    void PaintVpn(ImDrawList* draw, const ImRect& frame, Cupertino::Rgba color);
    void PaintAppearance(ImDrawList* draw, const ImRect& frame, Cupertino::Rgba color);
    void PaintScreenSaver(ImDrawList* draw, const ImRect& frame, Cupertino::Rgba color);
    void PaintWallpaper(ImDrawList* draw, const ImRect& frame, Cupertino::Rgba color);
    void PaintAppleIntelligence(ImDrawList* draw, const ImRect& frame, Cupertino::Rgba color);
    void PaintLockScreen(ImDrawList* draw, const ImRect& frame, Cupertino::Rgba color);
    void PaintFileVault(ImDrawList* draw, const ImRect& frame, Cupertino::Rgba color);
    void PaintAppleCare(ImDrawList* draw, const ImRect& frame, Cupertino::Rgba color);
    void PaintAirDrop(ImDrawList* draw, const ImRect& frame, Cupertino::Rgba color);
    // macOS Sequoia's icon over the whole plate (Software Update and About).
    void PaintSequoia(ImDrawList* draw, const ImRect& frame, Cupertino::Rgba color);

    // A drawing in units of its own, unit pixels each, from origin: its points, boxes and lengths in pixels.
    struct DrawingSpace {
        ImVec2 origin;
        float unit = 1.0f;

        ImVec2 At(float x, float y) const {
            return origin + ImVec2(x, y) * unit;
        }

        ImRect Box(float x0, float y0, float x1, float y1) const {
            return ImRect(At(x0, y0), At(x1, y1));
        }

        float Length(float length) const {
            return length * unit;
        }
    };

    // A drawing measured in points on a capture, from the top-left of rect and scaled by scale.
    inline DrawingSpace PointSpace(const ImRect& rect, float scale = 1.0f) {
        return {rect.Min, Cupertino::Px(scale)};
    }

    // The Mouse and Trackpad panes open alike (Illustrations.cpp): the device and the first frame of its gesture video side
    // by side, then the tabs.
    using Painter = void (*)(ImDrawList* draw, const ImRect& rect);
    void GestureHeader(Painter device, Painter video, int* tab, std::initializer_list<const char*> tabs);
    // The gesture videos' desktop: the blue backdrop with its menu bar and the Dock, framed like a section.
    void PaintGestureDesktop(ImDrawList* draw, const ImRect& rect);
    // A fingertip resting on the drawn device.
    void PaintFingertip(ImDrawList* draw, ImVec2 center);
    // The arrow pointer in a video frame, black with a white rim, its tip at the given point.
    void PaintPointer(ImDrawList* draw, ImVec2 tip);
    // The MacBook Air of Displays and About (Illustrations.cpp), 126 x 78 pt at full size, scaled to rect, with screen
    // painting its display.
    void PaintMacBookAir(ImDrawList* draw, const ImRect& rect, Painter screen);

    void AppearancePane(const Mac& mac, AppearancePreferences& preferences);
    void DesktopAndDockPane(const Mac& mac);
    void DisplayPane();
    void BatteryPane(const BatteryPreferences& battery);
    void BluetoothPane(const Mac& mac);
    void SharingPane(const Mac& mac);
    void SoftwareUpdatePane();
    void PrivacySecurityPane(const Mac& mac);
    void WiFiPane(const Mac& mac);
    void AirPodsPane();
    void SoundPane();
    void KeyboardPane();
    void TrackpadPane();
    void MousePane();
    void DisplaysPane(const Mac& mac);
    void PrintersPane();
    // Sets page to the account's page when its row is clicked.
    void InternetAccountsPane(int* page);
    void ICloudPane(const Mac& mac);
    void StoragePane();
    void AboutPane(const Mac& mac);
    void ControlCenterPane(const Mac& mac);
    void SpotlightPane(const Mac& mac);
    void NetworkPane();
    void VpnPane();
    void FocusPane();
    void ScreenTimePane();
    void LockScreenPane(const Mac& mac);
    void UsersPane(const Mac& mac);
    void NotificationsPane();
    void SiriPane();
    void TouchIdPane();
    void AirDropPane();
    void DateTimePane();
    void StartupDiskPane();
    void TimeMachinePane();
    void LoginItemsPane();
    void LanguageRegionPane();
    void TransferPane();
    void AppleAccountPane(const Mac& mac);
    // Sets page to the page of Accessibility whose row is clicked.
    void AccessibilityPane(int* page);
    void WalletPane();
    void GameCenterPane(const Mac& mac);
    void WallpaperPane();
    void ScreenSaverPane();
    // Sets page to the page of General whose row is clicked.
    void GeneralPane(int* page);
} // namespace Examples::Settings
