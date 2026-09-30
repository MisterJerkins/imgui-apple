#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    struct NotificationsModel {
        int previews = 1;
        bool whileDisplaySleeping = false;
        bool whileLocked = true;
        bool whileMirroring = false;
    };

    // Notifications as 512 Pixels captured it: the pane's header, Notification Center, and the apps.
    void NotificationsPane() {
        static NotificationsModel model;
        Form([] {
            Section([] {
                Label("Notifications", {.description = "Customize when and how notifications appear, what they sound like, and which apps can send them. [Learn more\xE2\x80\xA6](help:notifications)", .icon = PaneIcon(Pane::Notifications)});
            });
            Section({.header = "Notification Center", .description = "Notification Center shows your notifications in the top-right corner of your screen. You can show and hide Notification Center by clicking the clock in the menu bar."}, [] {
                Picker("Show previews", &model.previews, {"Always", "When Unlocked", "Never"});
                Toggle("Allow notifications when the display is sleeping", &model.whileDisplaySleeping);
                Toggle("Allow notifications when the screen is locked", &model.whileLocked);
                Toggle("Allow notifications when mirroring or sharing the display", &model.whileMirroring);
            });
            // An app: its icon, its name over the alerts it may use, and a chevron.
            Section({.header = "Application Notifications"}, [] {
                PictureLink("FaceTime", "app-facetime", {Symbols::VideoFill, IconPlate::Green}, {.description = "Badges, Sounds, Banners"});
                PictureLink("Find My", "app-find-my", {Symbols::LocationFill, IconPlate::Green}, {.description = "Badges, Sounds, Time Sensitive"});
                PictureLink("Game Center", "app-game-center", {.symbol = Symbols::GamecontrollerFill, .plate = IconPlate::White, .color = Rgba::Hex(0x8E5AF7)}, {.description = "Badges, Sounds, Banners, Time Sensitive"});
                PictureLink("Home", "app-home", {Symbols::HouseFill, IconPlate::Orange}, {.description = "Badges, Sounds, Time Sensitive"});
            });
        });
    }
} // namespace Examples::Settings
