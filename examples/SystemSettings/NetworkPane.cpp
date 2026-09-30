#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    // A service in its own section (Network @2x): its 26 pt plate 11 pt in, the name and its state 2 pt under it
    // 11.5 pt after the plate, a chevron at the end; the row is 51 pt.
    static void Service(const SectionOptions& section, const char* name, const Icon& icon, Rgba state_color, const char* state) {
        Section(section, [&] {
            NavigationLink(name, [&] {
                HStack({.spacing = 11.5f}, [&] {
                    Image(icon, ImVec2(26.0f, 26.0f));
                    VStack({.alignment = HorizontalAlignment::Leading, .spacing = 2.0f}, [&] {
                        Text(name);
                        ServiceStatus(state_color, state);
                    });
                });
            });
        });
    }

    // Network as 512 Pixels captured it: Wi-Fi and the firewall, the other services under a header, and the actions
    // pull-down beside the help button.
    void NetworkPane() {
        Form([] {
            Service({}, "Wi-Fi", PaneIcon(Pane::WiFi), Theme::SystemGreen(), "Connected");
            Service({}, "Firewall", PictureIcon("sidebar/firewall", {Symbols::Firewall, IconPlate::Red}), Rgba::Hex(0xADADAC), "Inactive");
            Service({.header = "Other Services"}, "Thunderbolt Bridge", PictureIcon("sidebar/thunderbolt-bridge", {Symbols::ArrowLeftArrowRight, IconPlate::Gray}), Theme::SystemRed(), "Not connected");
            TrailingButtons({.top = 10.0f, .spacing = 10.0f}, [] {
                Menu("Actions", {"Add Service\xE2\x80\xA6", "Set Service Order\xE2\x80\xA6", "Manage Virtual Interfaces\xE2\x80\xA6", "Locations"}, {.symbol = Symbols::Ellipsis, .plainIndicator = true});
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
