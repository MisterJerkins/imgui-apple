#include "Settings.h"

#include <string>

namespace Examples::Settings {
    using namespace Cupertino;

    struct BluetoothModel {
        bool enabled = true;
    };

    static BluetoothModel& Model() {
        static BluetoothModel model;
        return model;
    }

    // A device's picture, or the symbol of its kind without the assets.
    static Icon DevicePicture(const BluetoothDevice& device) {
        const std::string_view picture = device.picture;
        const unsigned symbol = picture == "magic-mouse" ? Symbols::Magicmouse : picture == "airpods-pro" ? Symbols::Airpodspro : Symbols::DotRadiowavesLeftAndRight;
        return PictureIcon(std::string("pictures/") + device.picture, {.symbol = symbol, .color = Theme::Colors().secondaryLabel});
    }

    // A paired device: its picture where a plate stands, the name and the state with the battery's charge in Callout, the
    // info button on the name's line (Bluetooth @2x: rows 54.5 pt).
    static void PairedDevice(const BluetoothDevice& device) {
        HStack({.alignment = VerticalAlignment::Top, .spacing = 12.0f}, [&] {
            Padding(EdgeInsets{0.0f, 2.0f, 0.0f, 0.0f}, [&] { Image(DevicePicture(device), ImVec2(24.0f, 33.0f)); });
            Padding(EdgeInsets{1.5f, 0.0f, 0.0f, 0.0f}, [&] {
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 2.0f}, [&] {
                    Text(device.name);
                    HStack({.spacing = 3.0f}, [&] {
                        const Font callout = Font::Style(TextStyle::Callout);
                        Text("Connected \xE2\x80\x93", {.font = callout, .foreground = Foreground::Secondary});
                        Image(Symbols::Battery50percent, {.font = callout.SymbolRenderingMode(SymbolRendering::Hierarchical), .foreground = Foreground::Secondary});
                        Text(std::to_string(device.charge) + "%", {.font = callout, .foreground = Foreground::Secondary});
                    });
                });
            });
            Spacer();
            Padding(EdgeInsets{2.0f, 0.0f, 0.0f, 0.0f}, [&] { InfoButton(device.name); });
        });
    }

    // A device nearby: its picture, the name and Connect, the row 8 pt over and under the picture (Bluetooth @2x: 50 pt).
    static void NearbyDevice(const BluetoothDevice& device) {
        ListRowInsets(8.0f, 8.0f, [&] {
            HStack({.spacing = 11.0f}, [&] {
                Image(DevicePicture(device), ImVec2(27.5f, 34.0f));
                Text(device.name);
                Spacer();
                Button("Connect");
            });
        });
    }

    // A list with nothing in it says so in the middle of its box.
    static void Placeholder(const char* text) {
        Frame({.maxWidth = Infinity}, [&] { Text(text, {.foreground = Foreground::Secondary}); });
    }

    void BluetoothPane(const Mac& mac) {
        BluetoothModel& model = Model();
        const BluetoothPreferences& bluetooth = mac.bluetooth;
        Form([&] {
            Section([&] {
                Toggle("Bluetooth", &model.enabled, {
                    .description = "Connect to accessories you can use for activities such as streaming music, typing, and gaming. [Learn more\xE2\x80\xA6](help:bluetooth)",
                    .descriptionStyle = TextStyle::Callout,
                    .icon = {.plate = IconPlate::Blue, .paint = PaintBluetoothRune},
                });
                Text(std::string("This Mac is discoverable as ") + bluetooth.macName + " while Bluetooth Settings is open.", {.font = Font::Style(TextStyle::Callout), .foreground = Foreground::Secondary, .wraps = true});
            });
            Section({.header = "My Devices"}, [&] {
                if (bluetooth.paired.empty())
                    Placeholder("No Bluetooth Devices");
                for (const BluetoothDevice& device : bluetooth.paired)
                    PairedDevice(device);
            });
            TrailingButtons([] {
                HelpButton();
            });
            Section({.header = "Nearby Devices", .headerAccessory = [] { WithControlSize(ControlSize::Small, [] { ProgressView(); }); }}, [&] {
                if (bluetooth.nearby.empty())
                    Placeholder("Searching\xE2\x80\xA6");
                for (const BluetoothDevice& device : bluetooth.nearby)
                    NearbyDevice(device);
            });
        });
    }
} // namespace Examples::Settings
