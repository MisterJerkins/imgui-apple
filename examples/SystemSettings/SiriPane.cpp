#include "Settings.h"

#include <string>

namespace Examples::Settings {
    using namespace Cupertino;

    struct SiriModel {
        bool siri = true;
        int listenFor = 0;
        int shortcut = 0;
        int language = 0;
    };

    // Siri as 512 Pixels captured it: the pane's header, the requests, and the buttons about privacy and responses.
    void SiriPane() {
        static SiriModel model;
        // The shortcut is the key and the microphone after it.
        static const std::string hold = "Hold " + Typography::SymbolText(Symbols::Mic);
        Form([] {
            Section([] { Label("Siri", {.description = "Siri is an intelligent assistant that helps you find information and get things done. [Learn more\xE2\x80\xA6](help:siri)", .icon = PictureIcon("sidebar/siri-large", PaneIcon(Pane::Siri))}); });
            Section({.header = "Siri Requests"}, [] {
                Toggle("Siri", &model.siri);
                Picker("Listen for", &model.listenFor, {"Off", "\xE2\x80\x9CSiri\xE2\x80\x9D or \xE2\x80\x9CHey Siri\xE2\x80\x9D", "\xE2\x80\x9CHey Siri\xE2\x80\x9D"});
                Picker("Keyboard shortcut", &model.shortcut, {hold.c_str(), "Press Either Command Key Twice", "Press Right Command Key Twice", "Off"});
                Picker("Language", &model.language, {"English (United States)", "English (United Kingdom)"});
                LabeledContent("Voice", [] {
                    Text("American (Voice 1)", {.foreground = Foreground::Secondary});
                    Button("Select\xE2\x80\xA6");
                });
                LabeledContent("Siri history", [] { Button("Delete Siri & Dictation History\xE2\x80\xA6"); });
            });
            TrailingButtons([] {
                Button("About Ask Siri, Dictation & Privacy\xE2\x80\xA6");
                Button("Siri Responses\xE2\x80\xA6");
            });
            TrailingButtons({.top = 20.0f}, [] {
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
