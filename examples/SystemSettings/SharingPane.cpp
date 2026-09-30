#include "Settings.h"

#include <array>
#include <string>

namespace Examples::Settings {
    using namespace Cupertino;

    // General > Sharing with its File Sharing sheet: shared folders and who may use them (Sharing @2x).
    struct SharingModel {
        std::array<bool, 10> services = {};
        bool fileSharingPresented = false;
        bool fullDiskAccess = true;
        std::vector<bool> folders = {true};
        std::vector<bool> users = {true, false, false};
        std::array<int, 3> access = {0, 1, 1};
    };

    // The pane starts from the Mac's own settings.
    static SharingModel& Model(const Mac& mac) {
        static SharingModel model = [&] {
            SharingModel initial;
            initial.services[0] = mac.fileSharing;
            return initial;
        }();
        return model;
    }

    struct Service {
        const char* title;
        Icon icon;
    };

    // A service's row; its info button opens the service's sheet when it has one.
    static void ServiceRow(const Service& service, bool* on, bool* info) {
        if (SwitchRow(service.title, service.icon, on) && info)
            *info = true;
    }

    // A list's title in Semibold, 15 pt in from the list's edge, over the list.
    static void TitledList(const char* title, float width, const std::function<void()>& list) {
        Frame({.width = width}, [&] {
            VStack({.alignment = HorizontalAlignment::Leading, .spacing = 8.5f}, [&] {
                Padding(EdgeInsets{7.5f, 15.0f, 0.0f, 0.0f}, [&] { Text(title, {.font = Font::System(13.0f, FontWeight::Semibold)}); });
                list();
            });
        });
    }

    static void SharedFolders(SharingModel& model) {
        static const char* const Actions[] = {"Show in Finder", "Apply Permissions to Enclosed Items", "Get Info", "Advanced Options…"};
        TitledList("Shared Folders", 225.0f, [&] {
            BorderedList("##folders", {.rows = 1, .rowHeight = 29.0f, .height = 168.0f, .showsAddRemove = true, .contextMenu = Actions}, &model.folders, [](int) {
                Frame({.width = 20.0f}, [] { Canvas(ImVec2(14.0f, 13.0f), [](ImDrawList* draw, const ImRect& rect) { DrawFolder(draw, rect); }); });
                Text("Time Machine Backup");
            });
        });
    }

    // The account's user first, then the groups.
    static void Users(SharingModel& model, const char* account) {
        const char* const names[] = {account, "Staff", "Everyone"};
        static const unsigned Glyphs[] = {Symbols::PersonFill, Symbols::Person2Fill, Symbols::Person3Fill};
        static const char* const Access[] = {"Read & Write", "Read Only", "Write Only (Drop Box)", "No Access"};
        TitledList("Users", 290.0f, [&] {
            BorderedList("##users", {.rows = 3, .rowHeight = 29.0f, .height = 168.0f, .showsAddRemove = true}, &model.users, [&](int row) {
                Frame({.width = 20.0f}, [&] { Image(Glyphs[row]); });
                Text(names[row]);
                Spacer();
                Picker((std::string("##access") + std::to_string(row)).c_str(), &model.access[size_t(row)], Access, {.width = 112.0f});
            });
        });
    }

    static void FileSharingSheet(SharingModel& model, const Mac& mac) {
        Form([&] {
            Section([&] {
                Toggle("File Sharing: On", &model.services[0], {.description = "Other users can access shared folders on this computer, and administrators all volumes, at smb://10.0.1.4", .icon = {Symbols::FolderFill, IconPlate::Cyan}});
            });
            Section([&] { Toggle("Allow full disk access for all users", &model.fullDiskAccess); });
            // The lists end a point closer to the footer than a section would.
            Padding(EdgeInsets{0.0f, 0.0f, -1.0f, 0.0f}, [&] {
                HStack({.alignment = VerticalAlignment::Top, .spacing = 20.0f}, [&] {
                    SharedFolders(model);
                    Users(model, mac.accountName);
                });
            });
        });
        SheetFooter([] {
            HelpButton();
            Button("Options…");
        }, [&] {
            if (Button("Done", {.role = ButtonRole::Default}))
                model.fileSharingPresented = false;
        });
    }

    // The services by group; their plates are pictures in the assets' sidebar folder (from 512 Pixels' Sharing @2x and,
    // for the last two that window cuts off, the dark Sharing of Ask Different).
    void SharingPane(const Mac& mac) {
        static const std::array<Service, 4> content = {{
            {"File Sharing", PictureIcon("sidebar/sharing-file", {Symbols::FolderFill, IconPlate::Cyan})},
            {"Media Sharing", PictureIcon("sidebar/sharing-media", {Symbols::MusicNoteHouseFill, IconPlate::Pink})},
            {"Screen Sharing", PictureIcon("sidebar/sharing-screen", {Symbols::Display, IconPlate::Blue})},
            {"Content Caching", PictureIcon("sidebar/sharing-content-caching", {Symbols::ArrowDownCircleFill, IconPlate::Orange})},
        }};
        static const std::array<Service, 3> accessories = {{
            {"Bluetooth Sharing", PictureIcon("sidebar/sharing-bluetooth", {.plate = IconPlate::Blue, .paint = PaintBluetoothRune})},
            {"Printer Sharing", PictureIcon("sidebar/sharing-printer", {Symbols::PrinterFill, IconPlate::Gray})},
            {"Internet Sharing", PictureIcon("sidebar/sharing-internet", {Symbols::GlobeAmericasFill, IconPlate::Blue})},
        }};
        static const std::array<Service, 3> advanced = {{
            {"Remote Management", PictureIcon("sidebar/sharing-remote-management", {Symbols::WrenchAndScrewdriverFill, IconPlate::Gray})},
            {"Remote Login", PictureIcon("sidebar/sharing-remote-login", {Symbols::AppleTerminal, IconPlate::Black})},
            {"Remote Application Scripting", PictureIcon("sidebar/sharing-remote-scripting", {Symbols::ApplescriptFill, IconPlate::Gray})},
        }};
        SharingModel& model = Model(mac);
        Form([&] {
            Section({.header = "Content & Media"}, [&] {
                for (size_t i = 0; i < content.size(); ++i)
                    ServiceRow(content[i], &model.services[i], i == 0 ? &model.fileSharingPresented : nullptr);
            });
            Section({.header = "Accessories & Internet"}, [&] {
                for (size_t i = 0; i < accessories.size(); ++i)
                    ServiceRow(accessories[i], &model.services[content.size() + i], nullptr);
            });
            Section({.header = "Advanced"}, [&] {
                for (size_t i = 0; i < advanced.size(); ++i)
                    ServiceRow(advanced[i], &model.services[content.size() + accessories.size() + i], nullptr);
            });
            // The note under the hostname wraps at 340 pt, well before Edit… (Sharing, dark @2x).
            Section([&] {
                LabeledContent("Local hostname", mac.hostname);
                HStack([] {
                    Frame({.maxWidth = 340.0f, .alignment = {HorizontalAlignment::Leading, VerticalAlignment::Center}}, [] {
                        Text("Computers on your local network can access your computer at this address.", {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary, .wraps = true});
                    });
                    Spacer();
                    Button("Edit\xE2\x80\xA6");
                });
            });
            TrailingButtons({.top = 10.0f}, [] {
                HelpButton();
            });
        });
        Sheet(&model.fileSharingPresented, {.width = 575.0f}, [&] { FileSharingSheet(model, mac); });
    }
} // namespace Examples::Settings
