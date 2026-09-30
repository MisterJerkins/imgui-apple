#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    // The account's head: the picture 100 pt, 10 pt under the toolbar, the name in Title2 Bold 4 pt under it and the
    // email address 5 pt lower (Apple Account @2x).
    static void AccountHead(const Mac& mac) {
        const Bitmap picture = mac.accountPictureLarge ? Picture(std::string("sidebar/") + mac.accountPictureLarge) : Bitmap{};
        const Bitmap hidden_email = mac.hiddenEmail ? Picture(std::string("pictures/") + mac.hiddenEmail) : Bitmap{};
        Padding(EdgeInsets{10.0f, 0.0f, 8.0f, 0.0f}, [&] {
            Frame({.maxWidth = Infinity}, [&] {
                VStack({.spacing = 4.0f}, [&] {
                    Canvas(ImVec2(100.0f, 100.0f), [&](ImDrawList* draw, const ImRect& rect) { DrawAvatar(draw, rect, mac.initials, picture); });
                    VStack({.spacing = 5.0f}, [&] {
                        Text(mac.accountName, {.font = Font::Style(TextStyle::Title2).Weight(FontWeight::Bold)});
                        if (hidden_email.IsEmpty())
                            Text(mac.email, {.foreground = Foreground::Secondary});
                        else
                            Image(hidden_email, ImVec2(182.0f, 19.0f));
                    });
                });
            });
        });
    }

    // Apple Account as 512 Pixels captured it: the account, what it holds, and the devices signed in to it.
    void AppleAccountPane(const Mac& mac) {
        Form([&] {
            AccountHead(mac);
            Section([] {
                PictureLink("Personal Information", "personal-information", {Symbols::PersonTextRectangleFill, IconPlate::Gray});
                PictureLink("Sign-In & Security", "sign-in-security", {Symbols::LockShieldFill, IconPlate::Gray});
                PictureLink("Payment & Shipping", "payment-shipping", {Symbols::CreditcardFill, IconPlate::Gray});
            });
            Section([] {
                PictureLink("iCloud", "icloud-row", PaneIcon(Pane::ICloud));
                PictureLink("Family", "family-row", PaneIcon(Pane::Family), {.value = "Set Up"});
                PictureLink("Media & Purchases", "media-purchases", {Symbols::BagFill, IconPlate::Blue});
                PictureLink("Sign in with Apple", "sign-in-with-apple", {Symbols::AppleLogo, IconPlate::Black});
            });
            // A device: its picture 34 pt wide 3 pt into the row, the name 13 pt after it, a point and a half under the row's
            // inset (the window cuts the row off).
            Section({.header = "Devices"}, [&] {
                NavigationLink("##this-mac", [&] {
                    Padding(EdgeInsets{1.5f, 0.0f, 0.0f, 0.0f}, [&] {
                        HStack({.alignment = VerticalAlignment::Top, .spacing = 13.0f}, [&] {
                            Padding(EdgeInsets{4.0f, 3.0f, 0.0f, 0.0f}, [] { Image(PictureIcon("pictures/macbook-air", {.symbol = Symbols::Laptopcomputer, .color = Theme::Colors().secondaryLabel}), ImVec2(34.0f, 14.5f)); });
                            VStack({.alignment = HorizontalAlignment::Leading, .spacing = 2.0f}, [&] {
                                Text(mac.computerName);
                                Text("This MacBook Air", {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary});
                            });
                        });
                    });
                });
            });
        });
    }
} // namespace Examples::Settings
