#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    // Screen Time as 512 Pixels captured it: the activity reports, the limits, communication and the restrictions.
    void ScreenTimePane() {
        Form([] {
            Section({.header = "Activity"}, [] {
                PictureLink("App & Website Activity", "app-website-activity", {Symbols::ChartBarXaxis, IconPlate::Purple});
                NavigationLink("Notifications", {.icon = PaneIcon(Pane::Notifications)});
                PictureLink("Pickups", "pickups", {Symbols::ArrowUpForwardApp, IconPlate::Orange});
            });
            Section({.header = "Limit Usage"}, [] {
                PictureLink("Downtime", "downtime", {Symbols::MoonZzzFill, IconPlate::Purple});
                PictureLink("App Limits", "app-limits", {Symbols::Hourglass, IconPlate::Orange});
                PictureLink("Always Allowed", "always-allowed", {Symbols::CheckmarkCircleFill, IconPlate::Green});
                PictureLink("Screen Distance", "screen-distance", {Symbols::ArrowLeftAndRight, IconPlate::Blue});
            });
            Section({.header = "Communication"}, [] {
                PictureLink("Communication Limits", "communication-limits", {Symbols::PersonCropCircleFill, IconPlate::Green});
                PictureLink("Communication Safety", "communication-safety", {Symbols::BubbleLeftAndExclamationmarkBubbleRightFill, IconPlate::Blue});
            });
            Section({.header = "Restrictions"}, [] { NavigationLink("Content & Privacy"); });
        });
    }
} // namespace Examples::Settings
