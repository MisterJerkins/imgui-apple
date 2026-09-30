#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    struct DateTimeModel {
        bool automatic = true;
        bool hourTime = false;
        bool hourTimeOnLockScreen = false;
        bool automaticTimeZone = true;
    };

    // General > Date & Time as 512 Pixels captured it: the time server, the date and the clock, and the time zone.
    void DateTimePane() {
        static DateTimeModel model;
        Form([] {
            Section([] {
                Toggle("Set time and date automatically", &model.automatic);
                LabeledContent("Source", [] {
                    Text("Apple (time.apple.com.)", {.foreground = Foreground::Secondary});
                    Button("Set\xE2\x80\xA6");
                });
            });
            Section([] {
                LabeledContent("Date and time", "Sep 12, 2024 at 6:46:40\xE2\x80\xAFPM");
                Toggle("24-hour time", &model.hourTime);
                Toggle("Show 24-hour time on Lock Screen", &model.hourTimeOnLockScreen);
            });
            Section([] {
                Toggle("Set time zone automatically using your current location", &model.automaticTimeZone);
                LabeledContent("Time zone", "Central Daylight Time");
                LabeledContent("Closest city", "Chicago, IL - United States");
            });
            TrailingButtons({.top = 10.0f}, [] {
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
