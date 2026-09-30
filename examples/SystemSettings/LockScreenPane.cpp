#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    static const char* const SaverTitles[] = {"Never", "For 1 minute", "For 2 minutes", "For 5 minutes", "For 10 minutes", "For 20 minutes", "For 30 minutes", "For 1 hour"};
    static const int SaverMinutes[] = {0, 1, 2, 5, 10, 20, 30, 60};
    static const char* const DisplayTitles[] = {"Never", "For 1 minute", "For 2 minutes", "For 3 minutes", "For 5 minutes", "For 10 minutes", "For 20 minutes", "For 30 minutes", "For 1 hour", "For 1 hour, 30 minutes", "For 2 hours", "For 3 hours"};
    static const int DisplayMinutes[] = {0, 1, 2, 3, 5, 10, 20, 30, 60, 90, 120, 180};
    static const char* const PasswordDelays[] = {"Immediately", "After 5 seconds", "After 1 minute", "After 5 minutes", "After 15 minutes", "After 1 hour", "After 4 hours", "After 8 hours", "Never"};

    // The item of a pop-up of delays that has the given minutes.
    static int DelayIndex(std::span<const int> minutes, int value) {
        for (size_t i = 0; i < minutes.size(); ++i) {
            if (minutes[i] == value)
                return int(i);
        }
        return 0;
    }

    struct LockScreenModel {
        int screenSaver = 0;
        int displayOnBattery = 0;
        int displayOnAdapter = 0;
        int requirePassword = 0;
        int largeClock = 0;
        bool hourTime = false;
        bool userNameAndPhoto = false;
        bool passwordHints = false;
        bool messageWhenLocked = false;
        int loginWindowShows = 0;
        bool shutDownButtons = true;
    };

    static LockScreenModel& Model(const LockScreenPreferences& preferences) {
        static LockScreenModel model = {
            .screenSaver = DelayIndex(SaverMinutes, preferences.screenSaver),
            .displayOnBattery = DelayIndex(DisplayMinutes, preferences.displayOnBattery),
            .displayOnAdapter = DelayIndex(DisplayMinutes, preferences.displayOnAdapter),
            .requirePassword = preferences.requirePassword,
            .largeClock = preferences.largeClock,
            .userNameAndPhoto = preferences.userNameAndPhoto,
        };
        return model;
    }

    static PickerOptions Warning(const char* text) {
        return {.description = text, .descriptionSymbol = Symbols::ExclamationmarkTriangleFill, .descriptionSymbolColor = Theme::SystemYellow()};
    }

    // Lock Screen as 512 Pixels and a Super User post captured it, with macOS's warnings: the screen saver cannot start
    // after the display sleeps, a display left on an hour or more costs energy, and iPhone Mirroring that signs in by
    // itself fixes the password to Immediately.
    void LockScreenPane(const Mac& mac) {
        const LockScreenPreferences& preferences = mac.lockScreen;
        LockScreenModel& model = Model(preferences);
        const int saver = SaverMinutes[model.screenSaver];
        const int battery = DisplayMinutes[model.displayOnBattery];
        const int adapter = DisplayMinutes[model.displayOnAdapter];
        const bool saver_too_late = saver > 0 && ((battery > 0 && battery < saver) || (adapter > 0 && adapter < saver));
        Form([&] {
            Section([&] {
                Picker("Start Screen Saver when inactive", &model.screenSaver, SaverTitles, saver_too_late ? Warning("Display will sleep before screen saver starts.") : PickerOptions{});
                Picker("Turn display off on battery when inactive", &model.displayOnBattery, DisplayTitles);
                Picker("Turn display off on power adapter when inactive", &model.displayOnAdapter, DisplayTitles, adapter >= 60 ? Warning("Energy usage may be higher when this Mac is inactive for longer periods of time before the display turns off.") : PickerOptions{});
                Picker("Require password after screen saver begins or display is turned off", &model.requirePassword, PasswordDelays, {.description = preferences.mirroringSignsIn ? "Password immediately required when iPhone Mirroring is set to automatically authenticate." : nullptr, .controlDisabled = preferences.mirroringSignsIn});
            });
            Section([&] {
                Picker("Show large clock", &model.largeClock, {"On Screen Saver and Lock Screen", "On Lock Screen", "Never"});
                Toggle("Show 24-hour time", &model.hourTime);
                Toggle("Show user name and photo", &model.userNameAndPhoto);
                Toggle("Show password hints", &model.passwordHints);
                // The message switch and its button, which waits until the switch is on, stand centered a point into the
                // row's insets (38 pt) with the title at the usual top (@2x).
                HStack({.alignment = VerticalAlignment::Top}, [&] {
                    Text("Show message when locked");
                    Spacer();
                    Padding(EdgeInsets{-1.0f, 0.0f, 0.0f, 0.0f}, [&] {
                        SwitchWithButton("##message", &model.messageWhenLocked, "Set\xE2\x80\xA6");
                    });
                });
            });
            Section({.header = "When Switching User"}, [&] {
                Picker("Login window shows", &model.loginWindowShows, {"List of users", "Name and password"}, {.style = PickerStyle::RadioGroup, .horizontal = true});
                Toggle("Show the Sleep, Restart, and Shut Down buttons", &model.shutDownButtons);
            });
            TrailingButtons({.top = 10.0f, .spacing = 10.0f}, [] {
                Button("Accessibility Options\xE2\x80\xA6");
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
