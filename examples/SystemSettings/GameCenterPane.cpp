#include "Settings.h"

#include <cctype>
#include <string>

namespace Examples::Settings {
    using namespace Cupertino;

    struct GameCenterModel {
        std::string nickname;
        int profilePrivacy = 0;
        bool findingByFriends = false;
        bool contactsOnly = false;
        bool nearbyPlayers = true;
        bool connectWithFriends = true;
    };

    // The player's monogram: the nickname's initial in white on the gray gradient of Contacts, Semibold 11 a point and a
    // half below the middle (Game Center @2x).
    static void PaintMonogram(ImDrawList* draw, const ImRect& rect, const char* nickname) {
        const float radius = rect.GetWidth() * 0.5f;
        Draw::FillVerticalGradient(draw, rect, CornerRadii(radius), Rgba::Hex(0xA5ABB9), Rgba::Hex(0x848993), CornerStyle::Circular);
        const ImVec2 drop(0.0f, Px(1.5f));
        const char initial[] = {char(std::toupper(static_cast<unsigned char>(nickname[0]))), 0};
        Typography::Draw(draw, Font::System(11.0f, FontWeight::Semibold), ImRect(rect.Min + drop, rect.Max + drop), Rgba::White(1.0f), initial, TextAlignment::Center);
    }

    // Game Center as 512 Pixels captured it: the signed-in player, who sees the profile, and how friends find the
    // player.
    void GameCenterPane(const Mac& mac) {
        static GameCenterModel model = {.nickname = mac.nickname};
        Form([&] {
            Section([&] {
                // The player's row keeps 13 pt around the 32 pt monogram.
                Padding(EdgeInsets{3.0f, 3.0f, 3.0f, 0.0f}, [&] {
                    HStack({.spacing = 11.0f}, [&] {
                        Canvas(ImVec2(32.0f, 32.0f), [&](ImDrawList* draw, const ImRect& rect) { PaintMonogram(draw, rect, mac.nickname); });
                        Text(mac.nickname);
                        Spacer();
                        HStack({.spacing = 8.0f}, [] {
                            Button("Show Profile\xE2\x80\xA6");
                            Button("Invite Friends\xE2\x80\xA6");
                        });
                    });
                });
                TextField("Nickname", &model.nickname);
                Text("Your nickname will represent you in Game Center and is visible to everyone for games with global leaderboards.", {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary, .wraps = true});
            });
            Section([] {
                Picker("Profile Privacy", &model.profilePrivacy, {"Only You", "Friends Only", "Everyone"}, {.description = "Choose who can see your activity. Your game activity includes your achievements and recently played games.\n[See how your data is managed\xE2\x80\xA6](help:game-center-privacy)"});
            });
            Section([] {
                Toggle("Allow Finding by Friends", &model.findingByFriends, {.description = "Help your Game Center friends find you more easily based on the name they have for you in their Contacts application. To do this, Game Center will use the email address and phone number associated with your Apple Account."});
                Toggle("Requests from Contacts Only", &model.contactsOnly, {.description = "Only friend requests you receive from your contacts will be displayed in your inbox."});
                Toggle("Nearby Players", &model.nearbyPlayers, {.description = "Allow nearby Game Center players in the same game to invite you to a multiplayer game over Wi-Fi or Bluetooth."});
            });
            Section([] {
                Toggle("Connect with Friends", &model.connectWithFriends, {.description = "Turning off Connect with Friends prevents applications from asking if they can connect you with your Game Center friends and will stop sharing your friends list with previously allowed applications."});
                TrailingButtons([] {
                    Button("Show Applications\xE2\x80\xA6");
                });
            });
        });
    }
} // namespace Examples::Settings
