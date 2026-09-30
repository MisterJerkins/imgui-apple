#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    // General > Transfer or Reset as 512 Pixels captured it: Migration Assistant with what it moves, and erasing the Mac.
    void TransferPane() {
        Form([] {
            Section([] {
                // Migration Assistant's row is 46 pt: its picture, 26 pt with its margins, half a point in and the title
                // 8.5 pt after it, all centered.
                Padding(EdgeInsets{0.0f, 0.5f, 0.0f, 0.0f}, [] {
                    HStack({.spacing = 8.5f}, [] {
                        Image(PictureIcon("pictures/app-migration-assistant", {Symbols::MacbookAndIphone, IconPlate::Gray}), ImVec2(25.0f, 26.0f));
                        Text("Migration Assistant");
                        Spacer();
                        Button("Open Migration Assistant\xE2\x80\xA6");
                    });
                });
                Text("Use Migration Assistant to transfer information (data, computer settings, and apps) to this Mac from another Mac, a Windows PC, a Time Machine backup, or disk. You can also transfer information from this Mac to another Mac.", {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary, .wraps = true});
            });
            TrailingButtons({.top = 10.0f, .spacing = 10.0f}, [] {
                Button("Erase All Content and Settings\xE2\x80\xA6");
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
