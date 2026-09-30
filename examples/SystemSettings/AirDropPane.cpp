#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    struct AirDropModel {
        bool handoff = true;
        int airDrop = 2;
        bool airPlayReceiver = true;
        int airPlayFor = 0;
        bool requirePassword = false;
    };

    // General > AirDrop & Handoff as 512 Pixels captured it: Handoff, who AirDrop takes files from, and AirPlay to this
    // Mac.
    void AirDropPane() {
        static AirDropModel model;
        Form([] {
            Section([] { Toggle("Allow Handoff between this Mac and your iCloud devices", &model.handoff); });
            Section([] {
                Picker("AirDrop", &model.airDrop, {"No One", "Contacts Only", "Everyone"}, {.description = "AirDrop lets you share instantly with people nearby. You can be discoverable in AirDrop to receive from everyone or only people in your contacts."});
                TrailingButtons([] {
                    Button("About AirDrop & Privacy\xE2\x80\xA6");
                });
            });
            Section([] {
                Toggle("AirPlay Receiver", &model.airPlayReceiver, {.description = "AirPlay Receiver allows nearby Apple devices to send video and audio content to your Mac with AirPlay."});
                Picker("Allow AirPlay for", &model.airPlayFor, {"Current User", "Anyone on the Same Network", "Everyone"}, {.description = "Only devices signed in to your Apple Account can see and AirPlay to this computer."});
                // Unlike Lock Screen's message row, the title stands centered with the switch and its button in a row
                // 39.5 pt tall (@2x).
                Padding(EdgeInsets{-0.5f, 0.0f, 0.0f, 0.0f}, [] {
                    HStack([] {
                        Text("Require password");
                        Spacer();
                        SwitchWithButton("##password", &model.requirePassword, "Set\xE2\x80\xA6");
                    });
                });
            });
            TrailingButtons({.top = 10.0f}, [] {
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
