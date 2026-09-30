#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    // The selected system: an inset box 150 x 181 pt with the disk's picture 19.5 pt from its top and the volume's name
    // and system 20 pt under it (Startup Disk @2x).
    static void SystemTile(const char* volume, const char* system) {
        Background(DrawInsetBox, [&] {
            Frame({.width = 150.0f, .height = 181.0f, .alignment = {HorizontalAlignment::Center, VerticalAlignment::Top}}, [&] {
                Padding(EdgeInsets{19.5f, 0.0f, 0.0f, 0.0f}, [&] {
                    VStack({.spacing = 20.0f}, [&] {
                        Image(PictureIcon("pictures/startup-disk", {.symbol = Symbols::InternaldriveFill, .color = Theme::Colors().secondaryLabel}), ImVec2(87.0f, 89.0f));
                        VStack({.spacing = 3.5f}, [&] {
                            Text(volume);
                            Text(system);
                        });
                    });
                });
            });
        });
    }

    // General > Startup Disk as 512 Pixels captured it: the one system on the Mac, selected, and the restart that starts
    // it.
    void StartupDiskPane() {
        Form([] {
            Section([] {
                // The prompt's row is 41 pt with the text in its middle.
                Padding(EdgeInsets::Symmetric(0.0f, 2.5f), [] { Text("Select the system you want to use to start up your computer"); });
                // The tile stands centered 14 pt under the line and 14.5 pt above the next.
                Padding(EdgeInsets{4.0f, 0.0f, 4.5f, 0.0f}, [] {
                    Frame({.maxWidth = Infinity}, [] { SystemTile("Macintosh HD", "macOS 15.0"); });
                });
                HStack({.alignment = VerticalAlignment::FirstTextBaseline, .spacing = 8.0f}, [] {
                    Text("You have selected macOS 15.0 on the disk \xE2\x80\x9CMacintosh HD\xE2\x80\x9D.", {.foreground = Foreground::Secondary});
                    Spacer();
                    Button("Restart\xE2\x80\xA6");
                });
            });
            TrailingButtons({.top = 10.0f}, [] {
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
