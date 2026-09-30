#include "Settings.h"

#include <vector>

namespace Examples::Settings {
    using namespace Cupertino;

    struct LanguageRegionModel {
        std::vector<bool> languages = {false};
        int region = 0;
        int calendar = 0;
        int temperature = 1;
        int measurement = 1;
        int firstDay = 0;
        int dateFormat = 0;
        int numberFormat = 0;
        bool liveText = true;
    };

    // How the region writes dates, times, money and numbers: two centered lines of Subheadline, the second's samples
    // 16 pt apart (Language & Region @2x).
    static void FormatPreview() {
        const TextOptions sample = {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary};
        Padding(EdgeInsets{0.0f, 0.0f, 0.5f, 0.0f}, [&] {
            Frame({.maxWidth = Infinity}, [&] {
                VStack({.spacing = 1.5f}, [&] {
                    Text("Thursday, September 12, 2024 at 6:51:59\xE2\x80\xAFPM CDT", sample);
                    HStack({.spacing = 16.0f}, [&] {
                        Text("9/12/24, 6:51\xE2\x80\xAFPM", sample);
                        Text("$12,345.67", sample);
                        Text("4,567.89", sample);
                    });
                });
            });
        });
    }

    // General > Language & Region as 512 Pixels captured it: the preferred languages, the region's formats and Live
    // Text.
    void LanguageRegionPane() {
        static LanguageRegionModel model;
        Form([] {
            BorderedList("##languages", {.rows = 1, .height = 117.0f, .alternatesRows = false, .showsAddRemove = true, .title = "Preferred Languages"}, &model.languages, [](int) {
                Padding(EdgeInsets{0.0f, 3.0f, 0.0f, 0.0f}, [] {
                    Frame({.width = 149.0f, .alignment = {HorizontalAlignment::Leading, VerticalAlignment::Center}}, [] { Text("English"); });
                });
                Text("English (US) \xE2\x80\x94 Primary", {.foreground = Foreground::Secondary});
            });
            Section([] {
                FormatPreview();
                // Region's row keeps 3 pt more under its pop-up than the rows after it (@2x: 39 pt).
                ListRowInsets(10.0f, 13.0f, [] { Picker("Region", &model.region, {"United States", "United Kingdom", "Canada"}); });
                Picker("Calendar", &model.calendar, {"Gregorian", "Buddhist", "Japanese"});
                Picker("Temperature", &model.temperature, {"Celsius (\xC2\xB0" "C)", "Fahrenheit (\xC2\xB0" "F)"}, {.style = PickerStyle::RadioGroup, .horizontal = true});
                Picker("Measurement system", &model.measurement, {"Metric", "US", "UK"}, {.style = PickerStyle::RadioGroup, .horizontal = true});
                Picker("First day of week", &model.firstDay, {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"});
                Picker("Date format", &model.dateFormat, {"8/19/24", "19/8/24", "2024-08-19"});
                Picker("Number format", &model.numberFormat, {"1,234,567.89", "1.234.567,89", "1 234 567,89"});
            });
            // Live Text's description is Footnote (10 pt), the Applications one Subheadline (@2x).
            Section([] { Toggle("Live Text", &model.liveText, {.description = "Select text in images to copy or take action.", .descriptionStyle = TextStyle::Footnote}); });
            Section([] {
                Padding(EdgeInsets{-0.5f, 0.0f, 0.0f, 0.0f}, [] {
                    VStack({.alignment = HorizontalAlignment::Leading, .spacing = 2.0f}, [] {
                        Text("Applications", {.font = Font::Style(TextStyle::Body).Weight(FontWeight::Semibold)});
                        Text("Customize language settings for the following applications:", {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary});
                    });
                });
            });
        });
    }
} // namespace Examples::Settings
