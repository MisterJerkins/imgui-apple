#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    struct SoundDevice {
        const char* name;
        const char* type;
    };

    struct SoundModel {
        int alertSound = 0;
        int effectsOutput = 0;
        float alertVolume = 1.0f;
        bool startupSound = true;
        bool interfaceSounds = true;
        bool volumeFeedback = false;
        int direction = 0;
        std::vector<bool> outputs = {true};
        std::vector<bool> inputs = {true};
        float outputVolume = 0.5f;
        bool mute = false;
        float balance = 0.5f;
        float inputVolume = 0.6f;
    };

    static const char* const AlertSounds[] = {"Boop", "Breeze", "Bubble", "Crystal", "Funky", "Heroine", "Jump", "Mezzo", "Pebble", "Pluck", "Pong", "Sonar", "Sonumi", "Submerge"};

    static const SoundDevice Outputs[] = {
        {"MacBook Air Speakers", "Built-in"},
        {"Beam", "AirPlay"},
        {"Blue mini", "AirPlay"},
        {"Den 4K HDR", "AirPlay"},
        {"Orange Pair", "AirPlay"},
        {"PodCabin", "AirPlay"},
        {"White mini", "AirPlay"},
        {"Yellow mini", "AirPlay"},
    };

    static const SoundDevice Inputs[] = {
        {"MacBook Air Microphone", "Built-in"},
    };

    // Sound as 512 Pixels captured it: the alert sound effects, then the output or input devices in a table under the
    // Output & Input switch with the volume of the side shown under them.
    void SoundPane() {
        static SoundModel model;
        Form([] {
            Section({.header = "Sound Effects"}, [] {
                LabeledContent("Alert sound", [] {
                    HStack({.spacing = 6.0f}, [] {
                        Picker("##alert-sound", &model.alertSound, AlertSounds);
                        Button("##play", {.style = ButtonStyle::Borderless, .symbol = Symbols::PlayCircle});
                    });
                });
                Picker("Play sound effects through", &model.effectsOutput, {"Selected Sound Output Device", "MacBook Air Speakers"});
                Slider("Alert volume", &model.alertVolume, 0.0f, 1.0f, {.width = 200.0f, .ticks = 7, .minimumSymbol = Symbols::SpeakerFill, .maximumSymbol = Symbols::SpeakerWave3Fill});
                Toggle("Play sound on startup", &model.startupSound);
                Toggle("Play user interface sound effects", &model.interfaceSounds);
                Toggle("Play feedback when volume is changed", &model.volumeFeedback);
            });
            Section({.header = "Output & Input"}, [] {
                Picker("##direction", &model.direction, {"Output", "Input"}, {.style = PickerStyle::Segmented});
                const std::span<const SoundDevice> devices = model.direction == 0 ? std::span<const SoundDevice>(Outputs) : std::span<const SoundDevice>(Inputs);
                std::vector<bool>& selection = model.direction == 0 ? model.outputs : model.inputs;
                const TableOptions table = {.style = TableStyle::FullWidth, .columns = {{"Name", 291.0f}, {"Type"}}, .rows = int(devices.size())};
                Table(model.direction == 0 ? "##outputs" : "##inputs", table, &selection, [&](const TableCell& cell) {
                    const SoundDevice& device = devices[size_t(cell.row)];
                    TableText(cell, cell.column == 0 ? device.name : device.type);
                });
                if (model.direction == 0) {
                    LabeledContent("Output volume", [] {
                        HStack({.spacing = 12.0f}, [] {
                            Slider("##output-volume", &model.outputVolume, 0.0f, 1.0f, {.width = 200.0f, .minimumSymbol = Symbols::SpeakerFill, .maximumSymbol = Symbols::SpeakerWave3Fill});
                            HStack({.spacing = 6.0f}, [] {
                                Text("Mute");
                                WithControlSize(ControlSize::Mini, [] { Toggle("##mute", &model.mute, {.style = ToggleStyle::Switch}); });
                            });
                        });
                    });
                    Slider("Balance", &model.balance, 0.0f, 1.0f, {.width = 200.0f, .tickValues = {0.5f}, .captions = {"Left", "Right"}});
                } else {
                    Slider("Input volume", &model.inputVolume, 0.0f, 1.0f, {.width = 200.0f, .minimumSymbol = Symbols::MicFill, .maximumSymbol = Symbols::MicFill, .symbolScale = SymbolScale::Large});
                    LevelIndicator("Input level", 4.0f, {.style = LevelIndicatorStyle::DiscreteCapacity, .maximum = 15.0f, .width = 200.0f});
                }
            });
            TrailingButtons({.top = 10.0f}, [] { HelpButton(); });
        });
    }
} // namespace Examples::Settings
