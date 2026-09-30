#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    // The state shown on Apple's Desktop & Dock screenshot; the Dock's size and the bouncing come from the Mac.
    struct DesktopDockModel {
        float size = 0.097f;
        float magnification = 0.0f;
        int position = 1;
        int minimizeEffect = 0;
        int doubleClick = 1;
        bool minimizeIntoIcon = false;
        bool autoHide = false;
        bool animateOpening = false;
        bool showIndicators = true;
        bool showRecents = true;
        bool itemsOnDesktop = true;
        bool itemsInStageManager = false;
        int clickWallpaper = 0;
        bool stageManager = false;
        bool recentAppsInStageManager = true;
        int windowsFromApp = 0;
        bool widgetsOnDesktop = true;
        bool widgetsInStageManager = true;
        int widgetStyle = 0;
        bool iPhoneWidgets = true;
        int browser = 0;
        int preferTabs = 2;
        bool askToKeepChanges = false;
        bool closeWindowsOnQuit = false;
        bool tileByDragging = false;
        bool holdOptionToTile = false;
        bool tiledMargins = true;
        bool rearrangeSpaces = false;
        bool switchToSpace = true;
        bool groupByApp = false;
        bool separateSpaces = true;
        bool dragToMissionControl = true;
    };

    static DesktopDockModel& Model(const Mac& mac) {
        static DesktopDockModel model = {.size = mac.dockSize, .animateOpening = mac.animateOpening};
        return model;
    }

    // Size and Magnification side by side, each a title over a slider with captions. Measured @2x: columns of 195.5 and
    // 215.5 pt, 28 pt apart; Magnification's "Small" tick sits at 13% of its track and its captions are a row with
    // "Small" 8 pt after "Off".
    static void DockSliders(DesktopDockModel& model) {
        const TextOptions caption{.font = Font::System(10.0f)};
        HStack({.alignment = VerticalAlignment::Top, .spacing = 28.0f}, [&] {
            VStack({.alignment = HorizontalAlignment::Leading, .spacing = 2.0f}, [&] {
                Text("Size");
                Slider("##size", &model.size, 0.0f, 1.0f, {.width = 195.5f, .ticks = 2, .captions = {"Small", "Large"}});
            });
            VStack({.alignment = HorizontalAlignment::Leading, .spacing = 2.0f}, [&] {
                Text("Magnification");
                Slider("##magnification", &model.magnification, 0.0f, 1.0f, {.width = 215.5f, .tickValues = {0.0f, 0.13f, 1.0f}});
                Frame({.width = 215.5f}, [&] {
                    HStack({.spacing = 8.0f}, [&] {
                        Text("Off", caption);
                        Text("Small", caption);
                        Spacer();
                        Text("Large", caption);
                    });
                });
            });
        });
    }

    // Safari's compass before its name in the browser pop-up, standing in for the app's icon (Desktop & Dock @2x).
    static void PaintBrowserIcon(ImDrawList* draw, const ImRect& frame, int) {
        Typography::DrawSymbol(draw, Symbols::SafariFill, Font::System(13.0f), frame, Theme::SystemBlue());
    }

    // A row title with two checkboxes on the trailing side, 16 pt apart.
    static void CheckboxPair(const char* title, const char* first, bool* first_on, const char* second, bool* second_on) {
        LabeledContent(title, [&] {
            HStack({.spacing = 16.0f}, [&] {
                Toggle(first, first_on, {.style = ToggleStyle::Checkbox});
                Toggle(second, second_on, {.style = ToggleStyle::Checkbox});
            });
        });
    }

    void DesktopAndDockPane(const Mac& mac) {
        DesktopDockModel& model = Model(mac);
        const bool british = mac.british;
        Form([&] {
            Section({.header = "Dock"}, [&] { DockSliders(model); });
            Section([&] {
                Picker("Position on screen", &model.position, {"Left", "Bottom", "Right"});
                Picker(british ? "Minimise windows using" : "Minimize windows using", &model.minimizeEffect, {"Genie Effect", "Scale Effect"});
                Picker("Double-click a window\xE2\x80\x99s title bar to", &model.doubleClick, {"Fill", "Zoom", british ? "Minimise" : "Minimize", "Do Nothing"});
                Toggle(british ? "Minimise windows into application icon" : "Minimize windows into application icon", &model.minimizeIntoIcon);
            });
            Section([&] {
                Toggle("Automatically hide and show the Dock", &model.autoHide);
                Toggle("Animate opening applications", &model.animateOpening);
                Toggle("Show indicators for open applications", &model.showIndicators);
                Toggle("Show suggested and recent apps in Dock", &model.showRecents);
            });
            Section({.header = "Desktop & Stage Manager"}, [&] {
                CheckboxPair("Show Items", "On Desktop##items", &model.itemsOnDesktop, "In Stage Manager##items", &model.itemsInStageManager);
                Picker("Click wallpaper to reveal desktop", &model.clickWallpaper, {"Always", "Only in Stage Manager"}, {.description = "Clicking your wallpaper will move all windows out of the way to allow access to your desktop items and widgets."});
            });
            Section([&] {
                Toggle("Stage Manager", &model.stageManager, {.description = "Stage Manager arranges your recent windows into a single strip for reduced clutter and quick access."});
                Toggle("Show recent apps in Stage Manager", &model.recentAppsInStageManager);
                Picker("Show windows from an application", &model.windowsFromApp, {"All at Once", "One at a Time"});
            });
            Section({.header = "Widgets"}, [&] {
                CheckboxPair("Show Widgets", "On Desktop##widgets", &model.widgetsOnDesktop, "In Stage Manager##widgets", &model.widgetsInStageManager);
                Picker("Widget style", &model.widgetStyle, {"Automatic", "Monochrome", british ? "Full-colour" : "Full-color"});
                Toggle("Use iPhone widgets", &model.iPhoneWidgets);
            });
            Section([&] {
                Picker("Default web browser", &model.browser, {"Safari.app"}, {.itemImage = PaintBrowserIcon, .itemImageSize = ImVec2(16.0f, 16.0f)});
            });
            Section({.header = "Windows"}, [&] {
                Picker("Prefer tabs when opening documents", &model.preferTabs, {"Never", "Always", "In Full Screen"});
                Toggle("Ask to keep changes when closing documents", &model.askToKeepChanges);
                Toggle("Close windows when quitting an application", &model.closeWindowsOnQuit, {.description = "When enabled, open documents and windows will not be restored when you reopen an application."});
            });
            Section([&] {
                Toggle("Tile by dragging windows to screen edges", &model.tileByDragging);
                Toggle("Hold \xE2\x8C\xA5 key while dragging windows to tile", &model.holdOptionToTile);
                Toggle("Tiled windows have margins", &model.tiledMargins);
            });
            Section({.header = "Mission Control", .description = "Mission Control shows an overview of your open windows and thumbnails of full-screen applications, all arranged in a unified view."}, [&] {
                Toggle("Automatically rearrange Spaces based on most recent use", &model.rearrangeSpaces);
                Toggle("When switching to an application, switch to a Space with open windows for the application", &model.switchToSpace);
                Toggle("Group windows by application", &model.groupByApp);
                Toggle("Displays have separate Spaces", &model.separateSpaces);
                Toggle("Drag windows to top of screen to enter Mission Control", &model.dragToMissionControl);
            });
            TrailingButtons({.top = 10.0f, .spacing = 10.0f}, [] {
                Button("Shortcuts\xE2\x80\xA6");
                Button("Hot Corners\xE2\x80\xA6");
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
