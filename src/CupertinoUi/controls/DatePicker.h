#pragma once

namespace Cupertino {
    // A calendar date and a time of day on a 24-hour clock, as a DatePicker binds them.
    struct DateTime {
        int year = 2000;
        int month = 1;
        int day = 1;
        int hour = 0;
        int minute = 0;
    };

    // The parts a date picker shows, as SwiftUI's displayedComponents.
    enum class DatePickerComponents {
        Date,
        HourAndMinute,
        DateAndTime,
    };

    enum class DatePickerStyle {
        // The date and the time in outlined fields with arrows.
        StepperField,
        // A month's calendar to click a day in, the time field under it when the time is shown.
        Graphical,
    };

    struct DatePickerOptions {
        DatePickerStyle style = DatePickerStyle::StepperField;
        DatePickerComponents components = DatePickerComponents::DateAndTime;
        // Hours from 0 to 23 instead of 1 to 12 with AM and PM.
        bool uses24HourClock = false;
    };

    // SwiftUI's DatePicker. In the field and stepper style (Screen Time's Downtime) the date and the time each sit in an
    // outlined field with arrows: a click selects a part; the arrows, Up and Down step it, digits type it, Left, Right and
    // Tab move between parts. The graphical style shows the month to click a day in. Returns true when the value changes.
    bool DatePicker(const char* label, DateTime* value, const DatePickerOptions& options = {});
} // namespace Cupertino
