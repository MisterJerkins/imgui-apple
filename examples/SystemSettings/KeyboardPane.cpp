#include "Settings.h"

#include <string>

namespace Examples::Settings {
    using namespace Cupertino;

    struct KeyboardModel {
        float repeatRate = 6.0f / 7.0f;
        float repeatDelay = 0.6f;
        bool adjustBrightness = true;
        int backlightOff = 0;
        int globeKey = 0;
        bool keyboardNavigation = false;
        bool dictation = false;
        int microphone = 0;
        int dictationShortcut = 0;
        bool autoPunctuation = true;
    };

    // A title 8 pt over a slider with ticks and its captions, as wide as its column (Desktop & Dock keeps its titles
    // closer).
    static void TitledSlider(const char* title, const char* id, float* value, int ticks, const std::function<void()>& captions) {
        Frame({.maxWidth = Infinity}, [&] {
            VStack({.alignment = HorizontalAlignment::Leading, .spacing = 8.0f}, [&] {
                Text(title);
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 2.0f}, [&] {
                    Slider(id, value, 0.0f, 1.0f, {.ticks = ticks, .snapsToTicks = true});
                    captions();
                });
            });
        });
    }

    // The Dictation switch: a microphone centered on a footnote instead of a title, the switch at the top of the row as
    // in rows with a description.
    static void DictationRow(bool* on) {
        HStack({.alignment = VerticalAlignment::Top, .spacing = 10.0f}, [&] {
            Frame({.maxWidth = Infinity, .alignment = {HorizontalAlignment::Leading, VerticalAlignment::Center}}, [&] {
                HStack({.spacing = 6.5f}, [&] {
                    Image(Symbols::MicFill, {.font = Font::System(21.0f), .foreground = Foreground::Secondary});
                    Text("Use Dictation wherever you can type text. To start dictating, use the shortcut or select Start Dictation from the Edit menu.", {.font = Font::Style(TextStyle::Footnote), .foreground = Foreground::Secondary, .wraps = true});
                });
            });
            WithControlSize(ControlSize::Mini, [&] { Toggle("##dictation", on, {.style = ToggleStyle::Switch}); });
        });
    }

    // A value in secondary text and a push button after it, as Input Sources shows "U.S." and Edit.
    static void ValueWithButton(const char* label, const char* value, const char* button) {
        LabeledContent(label, [&] {
            HStack({.spacing = 8.0f}, [&] {
                Text(value, {.foreground = Foreground::Secondary});
                Button(button);
            });
        });
    }

    // Keyboard as 512 Pixels captured it: the key repeat sliders, the backlight and Globe key, Text Input and Dictation.
    void KeyboardPane() {
        static KeyboardModel model;
        static const std::string press_globe = "Press " + Typography::SymbolText(Symbols::Globe) + " key to";
        static const std::string press_microphone = "Press " + Typography::SymbolText(Symbols::MicFill);
        const TextOptions caption{.font = Font::Style(TextStyle::Footnote)};
        Form([&] {
            Section([&] {
                HStack({.alignment = VerticalAlignment::Top, .spacing = 20.0f}, [&] {
                    TitledSlider("Key repeat rate", "##repeat-rate", &model.repeatRate, 8, [&] {
                        HStack({.spacing = 11.5f}, [&] {
                            Text("Off", caption);
                            Text("Slow", caption);
                            Spacer();
                            Text("Fast", caption);
                        });
                    });
                    TitledSlider("Delay until repeat", "##repeat-delay", &model.repeatDelay, 6, [&] {
                        HStack([&] {
                            Text("Long", caption);
                            Spacer();
                            Text("Short", caption);
                        });
                    });
                });
            });
            Section([&] {
                Toggle("Adjust keyboard brightness in low light", &model.adjustBrightness);
                Picker("Turn keyboard backlight off after inactivity", &model.backlightOff, {"Never", "After 5 secs", "After 10 secs", "After 30 secs", "After 1 min", "After 5 mins"});
                Picker(press_globe.c_str(), &model.globeKey, {"Show Emoji & Symbols", "Change Input Source", "Start Dictation", "Do Nothing"});
                Toggle("Keyboard navigation", &model.keyboardNavigation, {.description = "Use keyboard navigation to move focus between controls. Press the Tab key to move focus forward and Shift Tab to move focus backward."});
                TrailingButtons([] { Button("Keyboard Shortcuts\xE2\x80\xA6"); });
            });
            Section({.header = "Text Input"}, [&] {
                ValueWithButton("Input Sources", "U.S.", "Edit\xE2\x80\xA6##sources");
                TrailingButtons([] { Button("Text Replacements\xE2\x80\xA6"); });
            });
            Section({.header = "Dictation"}, [&] {
                DictationRow(&model.dictation);
                ValueWithButton("Languages", "English (United States)", "Edit\xE2\x80\xA6##languages");
                Picker("Microphone source", &model.microphone, {"Automatic (MacBook Air Microphone)", "MacBook Air Microphone"});
                Picker("Shortcut", &model.dictationShortcut, {press_microphone.c_str(), "Off", "Press Control Key Twice", "Press Command Key Twice"});
                Toggle("Auto-punctuation", &model.autoPunctuation);
            });
            TrailingButtons([] {
                Button("About Ask Siri, Dictation & Privacy\xE2\x80\xA6");
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
