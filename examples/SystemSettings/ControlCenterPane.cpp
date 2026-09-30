#include "Settings.h"

#include <string>

namespace Examples::Settings {
    using namespace Cupertino;

    struct ControlCenterModule {
        const char* title;
        Icon icon;
        // Modules that only show while in use offer that instead of always showing.
        bool whileActive;
        int choice;
    };

    // A module that can join Control Center and the menu bar, with the switches for both.
    struct OtherModule {
        const char* title;
        Icon icon;
        bool inMenuBar = false;
        bool inControlCenter = false;
    };

    // A menu bar item that has no module, with when it shows; Siri takes the icon of the Mac's sidebar.
    struct MenuBarItem {
        const char* title;
        Icon icon;
        int choice;
    };

    struct ControlCenterModel {
        std::vector<ControlCenterModule> modules;
        std::vector<OtherModule> others;
        bool batteryPercentage = false;
        int fastUserSwitching = 3;
        std::vector<MenuBarItem> menuBarOnly;
        int autoHide = 3;
        int recentItems = 1;
    };

    static ControlCenterModel& Model() {
        static ControlCenterModel model = {
            .modules = {
                {"Wi-Fi", PaneIcon(Pane::WiFi), false, 0},
                {"Bluetooth", PaneIcon(Pane::Bluetooth), false, 1},
                {"AirDrop", {.plate = IconPlate::White, .paint = PaintAirDrop}, false, 1},
                {"Focus", PaneIcon(Pane::Focus), true, 1},
                {"Stage Manager", {Symbols::SquaresLeadingRectangle, IconPlate::Black, SymbolScale::Large, 0.8f}, false, 1},
                {"Screen Mirroring", {Symbols::RectangleOnRectangle, IconPlate::Cyan, SymbolScale::Large, 0.8f}, true, 1},
                {"Display", PaneIcon(Pane::Displays), true, 1},
                {"Sound", PaneIcon(Pane::Sound), true, 1},
                {"Now Playing", {Symbols::PlayFill, IconPlate::Orange, SymbolScale::Large, 0.8f}, true, 1},
            },
            .others = {
                {"Accessibility Shortcuts", PaneIcon(Pane::Accessibility)},
                {"Battery", PaneIcon(Pane::Battery)},
                {"Hearing", {Symbols::EarFill, IconPlate::Blue}},
                {"Fast User Switching", {Symbols::Person2CircleFill, IconPlate::Blue}},
                {"Keyboard Brightness", PictureIcon("sidebar/control-keyboard-brightness", {Symbols::KeyboardFill, IconPlate::Blue})},
            },
            .menuBarOnly = {
                {"Spotlight", PaneIcon(Pane::Spotlight), 0},
                {"Siri", {}, 1},
                {"Time Machine", PictureIcon("sidebar/control-time-machine", {Symbols::ClockArrowCirclepath, IconPlate::Green}), 1},
                {"VPN", PaneIcon(Pane::Vpn), 1},
                {"Weather", PictureIcon("sidebar/control-weather", {Symbols::CloudSunFill, IconPlate::Blue}), 1},
            },
        };
        return model;
    }

    // Control Center as macOS 15 lists it (512 Pixels, and the end of it in dark Ask Different @2x): each module with its
    // icon and when it shows in the menu bar, the modules that can be added with a switch for each place, the items only
    // the menu bar shows, and how the menu bar hides. British English spells it Control Centre.
    void ControlCenterPane(const Mac& mac) {
        static const char* const Always[] = {"Always Show in Menu Bar", "Show When Active", "Don\xE2\x80\x99t Show in Menu Bar"};
        static const char* const Show[] = {"Show in Menu Bar", "Don\xE2\x80\x99t Show in Menu Bar"};
        const char* const control_center = mac.british ? "Control Centre" : "Control Center";
        const std::string show_in_control_center = std::string("Show in ") + control_center;
        ControlCenterModel& model = Model();
        Form([&] {
            Section({.header = (std::string(control_center) + " Modules").c_str(), .description = mac.british ? "These modules are always visible in Control Centre. You can choose when they should also show in the Menu Bar." : "These modules are always visible in Control Center. You can choose when they should also show in the Menu Bar."}, [&] {
                for (ControlCenterModule& module : model.modules) {
                    ImGui::PushID(module.title);
                    if (module.whileActive)
                        Picker(module.title, &module.choice, Always, {.icon = module.icon});
                    else
                        Picker(module.title, &module.choice, Show, {.icon = module.icon});
                    ImGui::PopID();
                }
            });
            // Each module that can be added is a section of its own: its title, then its places, the first under the group's
            // header.
            for (size_t i = 0; i < model.others.size(); ++i) {
                OtherModule& module = model.others[i];
                ImGui::PushID(module.title);
                SectionOptions options;
                if (i == 0) {
                    options.header = "Other Modules";
                    options.description = mac.british ? "These modules can be added to Control Centre and the Menu Bar." : "These modules can be added to Control Center and the Menu Bar.";
                }
                Section(options, [&] {
                    Label(module.title, {.icon = module.icon});
                    // The rows under a module start where its title does, and so do the separators under them.
                    ListRowInsets(10.0f, 41.0f, 10.0f, [&] {
                        if (std::string_view(module.title) == "Fast User Switching")
                            Picker("Show in Menu Bar", &model.fastUserSwitching, {"Full Name", "Account Name", "Icon", "Don\xE2\x80\x99t Show"});
                        else
                            Toggle("Show in Menu Bar", &module.inMenuBar);
                        Toggle(show_in_control_center.c_str(), &module.inControlCenter);
                        if (std::string_view(module.title) == "Battery")
                            Toggle("Show Percentage", &model.batteryPercentage);
                    });
                });
                ImGui::PopID();
            }
            Section({.header = "Menu Bar Only"}, [&] {
                // The clock's row, 41 pt with the button a point into its top inset: its small plate at the row inset, the
                // title after it where a navigation row's starts, the button at the end (Control Centre @2x).
                Frame({.minHeight = 22.0f}, [] {
                    HStack({.spacing = 10.0f}, [] {
                        Image(PictureIcon("sidebar/control-clock", {.symbol = Symbols::ClockFill, .plate = IconPlate::White, .color = Rgba::Black(1.0f)}), ImVec2(20.0f, 20.0f));
                        Text("Clock");
                        Spacer();
                        Button("Clock Options\xE2\x80\xA6");
                    });
                });
                for (MenuBarItem& item : model.menuBarOnly) {
                    ImGui::PushID(item.title);
                    Picker(item.title, &item.choice, Show, {.icon = item.icon.IsEmpty() ? SidebarIcon(mac, Pane::Siri) : item.icon});
                    ImGui::PopID();
                }
            });
            Section([&] {
                Picker("Automatically hide and show the menu bar", &model.autoHide, {"Always", "On Desktop Only", "In Full Screen Only", "Never"});
                Picker("Recent documents, applications and servers", &model.recentItems, {"None", "5", "10", "15", "20", "30", "50"});
            });
            TrailingButtons({.top = 10.0f}, [] {
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
