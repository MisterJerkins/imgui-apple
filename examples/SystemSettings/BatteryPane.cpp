#include "Settings.h"

#include <algorithm>
#include <array>

namespace Examples::Settings {
    using namespace Cupertino;

    // The Battery pane (Energy Mode and Low Power Mode @2x) and its Options sheet (Battery Options @2x).
    struct BatteryModel {
        int lowPowerMode = 0;
        int onBattery = 0;
        int onPowerAdapter = 1;
        int period = 0;
        bool optionsPresented = false;
        bool dimDisplay = true;
        bool preventSleep = false;
        int powerNap = 2;
        int wakeForNetwork = 2;
        bool graphicsSwitching = true;
        bool optimizeStreaming = false;
    };

    static BatteryModel& Model() {
        static BatteryModel model;
        return model;
    }

    // The last 24 hours in 15-minute slots.
    static constexpr int DaySlots = 96;

    // Both charts keep their plots ending together, at the width of the widest axis label ("100%").
    static float AxisLabelWidth() {
        return Pt(Typography::Width(Font::Style(TextStyle::Callout).MonospacedDigit(), "100%"));
    }

    // Hours from the start of the charts to midnight.
    static int HoursToMidnight(const BatteryPreferences& battery) {
        return (24 - battery.startHour) % 24;
    }

    static void BatteryLevelChart(const BatteryPreferences& battery) {
        std::vector<float> levels(battery.levels.size());
        std::transform(battery.levels.begin(), battery.levels.end(), levels.begin(), [](int percent) { return float(percent) / 100.0f; });
        std::array<bool, DaySlots> charging{};
        std::fill(charging.begin() + battery.charging.first, charging.begin() + battery.charging.second, true);
        const std::array<const char*, 8>& hours = battery.hours;
        BarChart(levels, {
            .slots = DaySlots,
            .yMarks = {{1.0f, "100%"}, {0.75f}, {0.5f, "50%"}, {0.25f}, {0.0f, "0%"}},
            .labelWidth = AxisLabelWidth(),
            .gridEvery = 12,
            .xMarks = {{0.0f, hours[0]}, {12.0f, hours[1]}, {24.0f, hours[2]}, {36.0f, hours[3]}, {48.0f, hours[4]}, {60.0f, hours[5]}, {72.0f, hours[6]}, {84.0f, hours[7]}},
            .dayMarks = {{float(HoursToMidnight(battery) * 4)}},
            .band = std::span<const bool>(charging.data(), levels.size()),
        });
    }

    static void ScreenOnUsageChart(const BatteryPreferences& battery) {
        const std::array<const char*, 8>& hours = battery.hours;
        BarChart(battery.screenMinutes, {
            .maximum = 60.0f,
            .barWidth = 11.0f,
            .color = Theme::SystemBlue(),
            .yMarks = {{60.0f, "60m"}, {45.0f}, {30.0f, "30m"}, {15.0f}, {0.0f, "0m"}},
            .labelWidth = AxisLabelWidth(),
            .gridEvery = 3,
            .xMarks = {{0.0f, hours[0]}, {3.0f, hours[1]}, {6.0f, hours[2]}, {9.0f, hours[3]}, {12.0f, hours[4]}, {15.0f, hours[5]}, {18.0f, hours[6]}, {21.0f, hours[7]}},
            .dayMarks = {{float(HoursToMidnight(battery)), battery.newDay}},
        });
    }

    static void BatteryOptions(BatteryModel& model) {
        static const char* const Schedule[] = {"Never", "Always", "Only on Power Adapter"};
        Form([&] {
            Section([&] {
                Toggle("Slightly dim the display on battery", &model.dimDisplay);
                Toggle("Prevent automatic sleeping on power adapter when the display is off", &model.preventSleep);
                Picker("Enable Power Nap", &model.powerNap, Schedule, {.description = "While sleeping, your Mac can periodically check for new email, calendar, and other iCloud updates."});
                Picker("Wake for network access", &model.wakeForNetwork, Schedule);
                Toggle("Automatic graphics switching", &model.graphicsSwitching, {.description = "To increase battery life, your Mac will automatically choose the best graphics mode based on your usage."});
                Toggle("Optimize video streaming while on battery", &model.optimizeStreaming, {.description = "To increase battery life, your Mac will stream high-dynamic-range (HDR) video in standard-dynamic-range (SDR)."});
            });
        });
        SheetFooter([&] {
            if (Button("Done", {.role = ButtonRole::Default}))
                model.optionsPresented = false;
        });
    }

    void BatteryPane(const BatteryPreferences& battery) {
        static const char* const EnergyModes[] = {"Low Power", "Automatic", "High Power"};
        static constexpr const char* EnergyDescription = "Your Mac will automatically choose the best level of performance and energy usage.";
        const TextOptions heading = {.font = Font::Style(TextStyle::Headline)};
        BatteryModel& model = Model();
        Form([&] {
            if (!battery.energyModes)
                Section([&] { Picker("Low Power Mode", &model.lowPowerMode, {"Never", "Always", "Only on Battery", "Only on Power Adapter"}); });
            Section([&] {
                LabeledContent("Battery Health", [] {
                    Text("Normal", {.foreground = Foreground::Secondary});
                    Button("##health", {.style = ButtonStyle::Borderless, .symbol = Symbols::InfoCircle});
                });
            });
            if (battery.energyModes) {
                Section({.header = "Energy Mode", .description = "Your Mac can optimize either its battery usage with Low Power Mode, or its performance in resource-intensive tasks with High Power Mode."}, [&] {
                    Picker("On battery", &model.onBattery, EnergyModes, {.description = EnergyDescription});
                    Picker("On power adapter", &model.onPowerAdapter, EnergyModes, {.description = EnergyDescription});
                });
            }
            Section([&] {
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 8.0f}, [&] {
                    Picker("##period", &model.period, {"Last 24 Hours", "Last 10 Days"}, {.style = PickerStyle::Segmented});
                    VStack({.alignment = HorizontalAlignment::Leading, .spacing = 2.0f}, [&] {
                        Text("Last charged to 100%");
                        Text(battery.lastCharged, {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary});
                    });
                });
                // A chart starts 3.5 pt under its title and the next title 8 pt under its axis (Battery @2x).
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 8.0f}, [&] {
                    VStack({.alignment = HorizontalAlignment::Leading, .spacing = 3.5f}, [&] {
                        Text("Battery Level", heading);
                        BatteryLevelChart(battery);
                    });
                    VStack({.alignment = HorizontalAlignment::Leading, .spacing = 3.5f}, [&] {
                        Text("Screen On Usage", heading);
                        ScreenOnUsageChart(battery);
                    });
                });
            });
            TrailingButtons({.top = 10.0f}, [&] {
                if (Button("Options\xE2\x80\xA6"))
                    model.optionsPresented = true;
                HelpButton();
            });
        });
        Sheet(&model.optionsPresented, {.width = 470.0f}, [&] { BatteryOptions(model); });
    }
} // namespace Examples::Settings
