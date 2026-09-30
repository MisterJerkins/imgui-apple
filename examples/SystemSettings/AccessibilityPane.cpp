#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    // Accessibility as 512 Pixels captured it: the pane's header and the features for vision and hearing; Display opens
    // its page.
    void AccessibilityPane(int* page) {
        Form([&] {
            Section([] {
                Label("Accessibility", {.description = "Personalize Mac in ways that work best for you with accessibility features for vision, hearing, motor, speech, and cognition. [Learn more\xE2\x80\xA6](help:accessibility)", .icon = PaneIcon(Pane::Accessibility)});
            });
            Section({.header = "Vision"}, [&] {
                PictureLink("VoiceOver", "voiceover", {Symbols::Voiceover, IconPlate::Black});
                PictureLink("Zoom", "zoom", {Symbols::PlusMagnifyingglass, IconPlate::Black});
                PictureLink("Hover Text", "hover-text", {Symbols::TextMagnifyingglass, IconPlate::Black});
                if (PictureLink("Display", "accessibility-display", {Symbols::SunMaxFill, IconPlate::Blue}))
                    *page = int(Pane::AccessibilityDisplay);
                PictureLink("Spoken Content", "spoken-content", {Symbols::SpeakerWave2BubbleFill, IconPlate::Black});
                PictureLink("Audio Descriptions", "audio-descriptions", {Symbols::TextBelowPhotoFill, IconPlate::Black});
            });
            Section({.header = "Hearing"}, [] {
                PictureLink("Audio", "audio", {Symbols::EarFill, IconPlate::Blue});
                PictureLink("Captions", "captions", {Symbols::CaptionsBubbleFill, IconPlate::Blue});
                PictureLink("Live Captions", "live-captions", {Symbols::WaveformAndPersonFilled, IconPlate::Blue});
            });
        });
    }
} // namespace Examples::Settings
