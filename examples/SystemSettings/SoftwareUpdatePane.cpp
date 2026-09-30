#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    // The update on offer: the icon, name and size, the two install buttons and the info button.
    static void UpdateHeader() {
        HStack([] {
            Image(Icon{.paint = PaintSequoia}, ImVec2(30.0f, 30.0f));
            VStack({.alignment = HorizontalAlignment::Leading, .spacing = 1.5f}, [] {
                Text("macOS Sequoia 15.5");
                Text("15.5 — 2.97 GB", {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary});
            });
            Spacer();
            Button("Update Tonight");
            Button("Update Now");
            Button("##update-info", {.style = ButtonStyle::Borderless, .symbol = Symbols::InfoCircle});
        });
    }

    // General > Software Update with macOS 15.5 on offer (Software Update @2x).
    void SoftwareUpdatePane() {
        const TextOptions note = {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary, .wraps = true};
        Form([&] {
            Section([&] {
                UpdateHeader();
                // The release notes set their lines 13 pt apart, half a point further in than a row's text.
                Padding(EdgeInsets::Symmetric(0.0f, 0.5f), [] {
                    Text("This update includes enhancements, bug fixes, and security updates for your Mac.\n\nSome features may not be available for all regions, or on all Apple devices.\nFor information on the security content of Apple software updates, please visit:\n<https://support.apple.com/100100>\n", {.font = Font::Style(TextStyle::Subheadline).WithLineHeight(13.0f), .foreground = Foreground::Secondary, .wraps = true});
                });
                Text("Once downloaded, this update will take about 20 minutes to install. [More Info…](more-info)", note);
            });
            Section([] { LabeledContent("Installed", "macOS Sequoia 15.4.1"); });
            Section([] {
                LabeledContent("Automatic Updates", [] {
                    Text("macOS updates and Security Responses", {.foreground = Foreground::Secondary});
                    Button("##automatic-info", {.style = ButtonStyle::Borderless, .symbol = Symbols::InfoCircle});
                });
            });
            Section([&] { Text("Use of this software is subject to the original license agreement that accompanied the software being updated. [Learn more…](license)", note); });
            // The help button keeps 28.5 pt from the last section and reaches 4.5 pt past its end.
            TrailingButtons({.top = 18.5f, .trailing = -4.5f}, [] {
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
