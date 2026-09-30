#include "Settings.h"

#include <string>

namespace Examples::Settings {
    using namespace Cupertino;

    struct WiFiModel {
        bool enabled = true;
        bool detailsPresented = false;
        int page = 0;
        bool autoJoin = true;
        int privateAddress = -1;
        bool lowDataMode = false;
        bool limitTracking = true;
    };

    // The pane starts from the Mac's own settings.
    static WiFiModel& Model(const Mac& mac) {
        static WiFiModel model;
        if (model.privateAddress < 0)
            model.privateAddress = mac.privateAddress;
        return model;
    }

    // The pages of a network's details after the network itself.
    static const char* const DetailPages[] = {"TCP/IP", "DNS", "WINS", "802.1X", "Proxies", "Hardware"};

    // What the private address does in each mode, under the pop-up.
    static const char* const PrivateAddressNotes[] = {
        "Using a private address helps reduce tracking of your Mac by Wi-Fi network operators.",
        "Using a private address helps reduce tracking of your Mac by Wi-Fi network operators. A fixed private address allows this network to track this Mac, but helps reduce tracking across other networks.",
        "Using a private address helps reduce tracking of your Mac by Wi-Fi network operators. A rotating private address reduces tracking on this network, and across other networks.",
    };

    // The network at the top of the details sidebar (Wi-Fi details @2x): a 26 pt plate 17 pt in, the name and the
    // state 7 pt after it, the row 47.5 pt.
    static void NetworkLabel(const char* network) {
        Padding(EdgeInsets{8.0f, 17.0f, 8.0f, 10.0f}, [&] {
            HStack({.spacing = 7.0f}, [&] {
                Image(PaneIcon(Pane::WiFi), ImVec2(26.0f, 26.0f));
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 1.5f}, [&] {
                    Text(network);
                    ServiceStatus(Theme::SystemGreen(), "Connected");
                });
                Spacer();
            });
        });
    }

    // Wi-Fi > Details… for the connected network: a sheet with the network's pages in a sidebar, the main page with the
    // pop-up of the private address, and the footer.
    static void NetworkDetails(WiFiModel& model, const char* network) {
        NavigationSplitView({.sidebarWidth = 200.0f}, [&] {
            List([&] {
                NavigationLink(&model.page, 0, [&] { NetworkLabel(network); });
                for (int i = 0; i < IM_ARRAYSIZE(DetailPages); ++i)
                    NavigationLink(DetailPages[i], &model.page, i + 1);
            });
        }, [&] {
            VStack({.spacing = 0.0f}, [&] {
                Form([&] {
                    Section([&] { Toggle("Automatically join this network", &model.autoJoin); });
                    Section([&] {
                        Picker("Private Wi-Fi address", &model.privateAddress, {"Off", "Fixed", "Rotating"}, {.description = PrivateAddressNotes[model.privateAddress], .menuCoversIndicator = true});
                        LabeledContent("Wi-Fi address", "f6:dd:d8:f9:7c:0a");
                    });
                    Section([&] {
                        Toggle("Low data mode", &model.lowDataMode, {.description = "Low data mode helps reduce your Mac data usage over specific Wi-Fi networks you select."});
                        Toggle("Limit IP address tracking", &model.limitTracking, {.description = "Limit IP address tracking by hiding your IP address from known trackers in Mail and Safari."});
                    });
                    Section([] {
                        LabeledContent("IP address", "10.0.1.4");
                        LabeledContent("Router", "10.0.1.1");
                    });
                });
                SheetFooter([] { Button("Forget This Network…"); }, [&] {
                    if (Button("Cancel"))
                        model.detailsPresented = false;
                    if (Button("OK", {.role = ButtonRole::Default}))
                        model.detailsPresented = false;
                });
            });
        });
    }

    // A network's signal: Wi-Fi in semibold lit up to its bars (Wi-Fi @2x: 30 px wide beside a regular lock of 18).
    static void Signal(int bars) {
        Image(Symbols::Wifi, {.font = Font::System(13.0f, FontWeight::Semibold).VariableValue(float(bars) / 3.0f)});
    }

    // The network the Mac is on: its name over its state, and at the top of the row the lock and the signal in the label
    // color 13.5 pt apart, centered on Details… 13 pt after them (Wi-Fi @2x: the row 53.5 pt).
    static void ConnectedNetwork(const char* network, WiFiModel& model) {
        HStack({.alignment = VerticalAlignment::Top}, [&] {
            VStack({.alignment = HorizontalAlignment::Leading, .spacing = 1.5f}, [&] {
                Text(network);
                ServiceStatus(Theme::SystemGreen(), "Connected", TextStyle::Body);
            });
            Spacer();
            HStack({.spacing = 13.0f}, [&] {
                HStack({.spacing = 13.5f}, [] {
                    Image(Symbols::LockFill);
                    Signal(3);
                });
                if (Button("Details\xE2\x80\xA6"))
                    model.detailsPresented = true;
            });
        });
    }

    // A network in range: a small semibold checkmark 16 pt into the row before the one the Mac is on, the name 30.5 pt in,
    // the lock and the signal in the label color 9 pt apart, and 6.5 pt after them the network's menu, a borderless
    // pull-down 20.5 pt tall, 22 pt from the edge (Wi-Fi @2x: rows 40.5 pt).
    static void NetworkRow(const WiFiNetwork& network, bool current) {
        HStack({.spacing = 0.0f}, [&] {
            Frame({.width = 30.5f}, [&] {
                if (current)
                    Padding(EdgeInsets{0.0f, 1.5f, 0.0f, 0.0f}, [] { Image(Symbols::Checkmark, {.font = Font::System(13.0f, FontWeight::Semibold).ImageScale(SymbolScale::Small)}); });
            });
            Text(network.name);
            Spacer();
            HStack({.spacing = 6.5f}, [&] {
                HStack({.spacing = 9.0f}, [&] {
                    Image(Symbols::LockFill);
                    Signal(network.bars);
                });
                Frame({.height = 20.5f}, [&] {
                    Padding(EdgeInsets{0.0f, 0.0f, 0.0f, 9.5f}, [&] { Button((std::string("##menu-") + network.name).c_str(), {.style = ButtonStyle::Borderless, .symbol = Symbols::EllipsisCircle}); });
                });
            });
        });
    }

    void WiFiPane(const Mac& mac) {
        WiFiModel& model = Model(mac);
        const char* network = mac.network;
        Form([&] {
            Section([&] {
                Toggle("Wi-Fi", &model.enabled, {
                    .description = "Set up Wi-Fi to wirelessly connect your Mac to the internet. Turn on Wi-Fi, then choose a network to join. [Learn More\xE2\x80\xA6](help:wifi)",
                    .descriptionStyle = TextStyle::Subheadline,
                    .icon = PaneIcon(Pane::WiFi),
                });
                ConnectedNetwork(network, model);
            });
            if (!mac.knownNetworks.empty()) {
                Section({.header = mac.knownNetworks.size() == 1 ? "Known Network" : "Known Networks"}, [&] {
                    for (const WiFiNetwork& known : mac.knownNetworks)
                        NetworkRow(known, std::string_view(known.name) == network);
                });
            }
            Section({.header = "Other Networks", .headerAccessory = [] { WithControlSize(ControlSize::Small, [] { ProgressView(); }); }}, [&] {
                for (const WiFiNetwork& other : mac.otherNetworks)
                    NetworkRow(other, false);
            });
        });
        Sheet(&model.detailsPresented, {.width = 670.0f, .height = 425.0f}, [&] { NetworkDetails(model, network); });
    }
} // namespace Examples::Settings
