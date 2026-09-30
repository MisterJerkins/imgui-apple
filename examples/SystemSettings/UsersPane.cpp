#include "Settings.h"

#include <string>

namespace Examples::Settings {
    using namespace Cupertino;

    struct UsersModel {
        int automaticLogin = 0;
    };

    // A user (Users & Groups @2x): the 32 pt picture 12 pt from the row's top, the name over the kind of account 8 pt
    // after it, the info button on the name's line at the end; the row is 53 pt.
    static void UserRow(const char* name, const char* kind, const char* initials, const char* picture) {
        const Bitmap bitmap = picture ? Picture(std::string("sidebar/") + picture) : Bitmap{};
        Padding(EdgeInsets{2.0f, 0.0f, 0.0f, 0.0f}, [&] {
            HStack({.alignment = VerticalAlignment::Top, .spacing = 8.0f}, [&] {
                Canvas(ImVec2(32.0f, 32.0f), [&](ImDrawList* draw, const ImRect& rect) { DrawAvatar(draw, rect, initials, bitmap); });
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 2.0f}, [&] {
                    Text(name);
                    Text(kind, {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary});
                });
                Spacer();
                InfoButton(name);
            });
        });
    }

    // Users & Groups as 512 Pixels captured it: the Mac's users, the buttons that add users and groups, and the login
    // settings.
    void UsersPane(const Mac& mac) {
        static UsersModel model;
        Form([&] {
            Section([&] {
                UserRow(mac.accountName, "Admin", mac.initials, mac.userPicture);
                UserRow("Guest User", "Off", "G", "guest-user");
            });
            TrailingButtons({.bottom = 20.0f}, [] {
                Button("Add Group\xE2\x80\xA6");
                Button("Add User\xE2\x80\xA6");
            });
            Section([&] {
                Picker("Automatically log in as", &model.automaticLogin, {"Off", mac.accountName});
                // Apple's row keeps 12 pt more under its button (@2x: 51 pt).
                Padding(EdgeInsets{0.0f, 0.0f, 12.0f, 0.0f}, [] { LabeledContent("Network account server", [] { Button("Edit\xE2\x80\xA6"); }); });
            });
            TrailingButtons({.top = 10.0f}, [] {
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
