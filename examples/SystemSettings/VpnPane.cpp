#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    // VPN with one configuration whose switch connects it, its state under its name; then the pull-down that adds another
    // beside the help button.
    void VpnPane() {
        static bool connected = false;
        Form([] {
            Section([] {
                const ServiceState state = {connected ? Theme::SystemGreen() : Theme::SystemGray(), connected ? "Connected" : "Not Connected"};
                SwitchRow("Office VPN", PaneIcon(Pane::Vpn), &connected, &state);
            });
            TrailingButtons({.top = 10.0f, .spacing = 10.0f}, [] {
                Menu("Add VPN Configuration", {"IKEv2\xE2\x80\xA6", "L2TP over IPSec\xE2\x80\xA6", "IPSec\xE2\x80\xA6"});
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
