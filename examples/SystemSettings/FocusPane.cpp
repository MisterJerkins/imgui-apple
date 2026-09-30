#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    struct FocusModel {
        bool shareAcrossDevices = true;
    };

    // Focus as 512 Pixels captured it: the Do Not Disturb focus, the button that adds one, sharing across devices and
    // the focus status.
    void FocusPane() {
        static FocusModel model;
        Form([] {
            // The focus row (@2x): a 26 pt plate 13 pt in and 13 pt from the top, the name 13 pt after it, 51.5 pt.
            Section([] {
                NavigationLink("Do Not Disturb", [] {
                    Padding(EdgeInsets{2.75f, 3.0f, 2.75f, 0.0f}, [] {
                        HStack({.spacing = 13.0f}, [] {
                            Image(PaneIcon(Pane::Focus), ImVec2(26.0f, 26.0f));
                            Text("Do Not Disturb");
                        });
                    });
                });
            });
            // The button stands a point under the section's spacing, the next section 30 pt under it.
            TrailingButtons({.top = 1.0f, .bottom = 20.0f}, [] {
                Button("Add Focus\xE2\x80\xA6");
            });
            Section([] { Toggle("Share across devices", &model.shareAcrossDevices, {.description = "Focus is shared across your devices, and turning one on for this device will turn it on for all of them."}); });
            // Apple breaks this description earlier than the width would (@2x).
            Section([] { NavigationLink("Focus status", {.value = "On", .description = "When you give an app permission, it can share that you have\nnotifications silenced when using Focus."}); });
            TrailingButtons({.top = 10.0f}, [] {
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
