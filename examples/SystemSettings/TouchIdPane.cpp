#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    struct TouchIdModel {
        bool unlock = true;
        bool applePay = true;
        bool purchases = true;
        bool autofill = true;
        bool userSwitching = true;
    };

    // A fingerprint or the button that adds one (Touch ID & Password @2x): a 32 pt picture over its Subheadline caption,
    // in columns 128.75 pt wide centered under the header.
    static void FingerprintCell(const char* caption, const std::function<void(ImDrawList*, const ImRect&)>& picture) {
        Frame({.width = 128.75f}, [&] {
            VStack({.alignment = HorizontalAlignment::Center, .spacing = 4.0f}, [&] {
                Canvas(ImVec2(32.0f, 32.0f), picture);
                Text(caption, {.font = Font::Style(TextStyle::Subheadline)});
            });
        });
    }

    // Touch ID & Password as 512 Pixels captured it: the password, the enrolled fingerprints and what Touch ID is used
    // for.
    void TouchIdPane() {
        static TouchIdModel model;
        Form([] {
            Section({.header = "Password"}, [] { LabeledContent("A login password has been set for this user.", [] { Button("Change\xE2\x80\xA6"); }); });
            Section({.header = "Touch ID", .description = "Touch ID lets you use your fingerprint to unlock your Mac and make purchases with Apple Pay, iTunes Store, App Store, and Apple Books. [About Touch ID & Privacy\xE2\x80\xA6](help:touch-id)"}, [] {});
            // The fingerprints stand 2 pt lower than a section would and keep 9.5 pt more under them.
            Padding(EdgeInsets{2.0f, 0.0f, 9.5f, 0.0f}, [] { HStack({.spacing = 0.0f}, [] {
                Spacer();
                FingerprintCell("Finger 1", [](ImDrawList* draw, const ImRect& rect) {
                    Typography::DrawSymbol(draw, Symbols::Touchid, Font::System(28.0f), rect, Theme::Colors().secondaryLabel);
                });
                FingerprintCell("Add Fingerprint", [](ImDrawList* draw, const ImRect& rect) {
                    Draw::FillCircle(draw, rect.GetCenter(), rect.GetWidth() * 0.5f, Theme::Colors().fill);
                    Typography::DrawSymbol(draw, Symbols::Plus, Font::System(17.0f, FontWeight::Medium), rect, Theme::Colors().secondaryLabel);
                });
                Spacer();
            }); });
            Section([] {
                Toggle("Use Touch ID to unlock your Mac", &model.unlock);
                Toggle("Use Touch ID for Apple Pay", &model.applePay);
                // Apple joins product names with a no-break space: "Apple Books" wraps as one word.
                Toggle("Use Touch ID for purchases in iTunes Store, App Store, and Apple\xC2\xA0" "Books", &model.purchases);
                Toggle("Use Touch ID for autofilling passwords", &model.autofill);
                Toggle("Use Touch ID for fast user switching", &model.userSwitching);
            });
            TrailingButtons({.top = 10.0f}, [] {
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
