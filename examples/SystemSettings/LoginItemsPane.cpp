#include "Settings.h"

#include <string>
#include <vector>

namespace Examples::Settings {
    using namespace Cupertino;

    struct LoginItemsModel {
        std::vector<bool> items = {false};
    };

    // An extension and what it offers on one line cut off with an ellipsis: the plate 26 pt, 13 pt in and 14 pt down, the
    // text 13 pt after it, the info button on the title's line (Login Items @2x: rows 54 pt).
    static void ExtensionRow(const char* title, const char* offers, const char* picture, const Icon& icon) {
        HStack({.alignment = VerticalAlignment::Top, .spacing = 13.0f}, [&] {
            Padding(EdgeInsets{4.0f, 3.0f, 0.0f, 0.0f}, [&] { Image(PictureIcon(std::string("sidebar/") + picture, icon), ImVec2(26.0f, 26.5f)); });
            Padding(EdgeInsets{1.5f, 0.0f, 0.5f, 0.0f}, [&] {
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 2.0f}, [&] {
                    Text(title);
                    Text(offers, {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary});
                });
            });
            Spacer();
            Padding(EdgeInsets{2.0f, 0.0f, 0.0f, 0.0f}, [&] { InfoButton(title); });
        });
    }

    // General > Login Items & Extensions as 512 Pixels captured it: the app that opens at login in a table with + and −,
    // and the extensions installed.
    void LoginItemsPane() {
        static LoginItemsModel model;
        Form([] {
            Section({.header = "Open at Login", .description = "These items will open automatically when you log in.", .boxed = false}, [] {
                BorderedList("##login-items", {.rows = 1, .height = 120.0f, .alternatesRows = false, .showsAddRemove = true, .columns = {{"Item", 233.0f}, {"Kind"}}}, &model.items, [](int) {
                    Frame({.width = 232.5f, .alignment = {HorizontalAlignment::Leading, VerticalAlignment::Center}}, [] {
                        HStack({.spacing = 8.5f}, [] {
                            Padding(EdgeInsets{0.0f, 1.5f, 0.0f, 0.0f}, [] { Image(PictureIcon("pictures/app-chess", {Symbols::GamecontrollerFill, IconPlate::DarkGray}), ImVec2(17.5f, 17.0f)); });
                            Text("Chess");
                        });
                    });
                    Text("Application");
                });
            });
            Section({.header = "Extensions", .description = "Extensions add extra functionality to your Mac and apps. Some extensions may run in the background."}, [] {
                ExtensionRow("Actions", "Markup", "extension-actions", {Symbols::PuzzlepieceExtensionFill, IconPlate::Gray});
                ExtensionRow("Finder", "Rotate Left, Markup, Trim, Create PDF, Convert Image, Remove Background", "extension-finder", {Symbols::FaceSmiling, IconPlate::Blue});
                ExtensionRow("Photos Editing", "Markup", "extension-photos-editing", {Symbols::PhotoFill, IconPlate::Orange});
                ExtensionRow("Quick Look", "TipsQuicklook", "extension-quick-look", {Symbols::EyeFill, IconPlate::Gray});
                ExtensionRow("Sharing", "Add to Photos, Add to Reading List, AirDrop, Copy Link, Freeform, Mail, Messages, Notes", "extension-sharing", {Symbols::SquareAndArrowUpFill, IconPlate::Blue});
            });
            TrailingButtons({.top = 10.0f}, [] {
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
