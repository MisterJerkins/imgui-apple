#include "DatePicker.h"

#include "LabeledContent.h"
#include "Stepper.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/State.h"
#include "core/Symbols.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

namespace Cupertino {
    enum class DatePart {
        Month,
        Day,
        Year,
        Hour,
        Minute,
        Meridiem,
    };

    // A part of a field and the text after it: 9/26/2026 or 10:00 PM, with the narrow no-break space macOS puts before AM
    // and PM.
    struct FieldPart {
        DatePart part;
        const char* separator;
    };

    // The selected part of a field while it has focus, and the digits typed into it so far.
    struct DateFieldState {
        int selected = 0;
        int typed = 0;
        int typedValue = 0;
    };

    static int DaysInMonth(int year, int month) {
        static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        const bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
        return month == 2 && leap ? 29 : days[ImClamp(month, 1, 12) - 1];
    }

    static std::vector<FieldPart> Parts(bool date, bool clock24) {
        if (date)
            return {{DatePart::Month, "/"}, {DatePart::Day, "/"}, {DatePart::Year, ""}};
        if (clock24)
            return {{DatePart::Hour, ":"}, {DatePart::Minute, ""}};
        return {{DatePart::Hour, ":"}, {DatePart::Minute, "\xE2\x80\xAF"}, {DatePart::Meridiem, ""}};
    }

    static std::string PartText(const DateTime& value, DatePart part, bool clock24) {
        char text[16];
        switch (part) {
            case DatePart::Month:
                std::snprintf(text, sizeof(text), "%d", value.month);
                break;
            case DatePart::Day:
                std::snprintf(text, sizeof(text), "%d", value.day);
                break;
            case DatePart::Year:
                std::snprintf(text, sizeof(text), "%d", value.year);
                break;
            case DatePart::Hour:
                std::snprintf(text, sizeof(text), clock24 ? "%02d" : "%d", clock24 ? value.hour : (value.hour + 11) % 12 + 1);
                break;
            case DatePart::Minute:
                std::snprintf(text, sizeof(text), "%02d", value.minute);
                break;
            case DatePart::Meridiem:
                return value.hour < 12 ? "AM" : "PM";
        }
        return text;
    }

    // Steps a part by direction, wrapping inside its range as NSDatePicker does: the hour keeps AM or PM, the day stays
    // within its month.
    static void StepPart(DateTime& value, DatePart part, int direction, bool clock24) {
        const auto wrap = [](int v, int min, int max) { return min + ((v - min) % (max - min + 1) + (max - min + 1)) % (max - min + 1); };
        switch (part) {
            case DatePart::Month:
                value.month = wrap(value.month + direction, 1, 12);
                break;
            case DatePart::Day:
                value.day = wrap(value.day + direction, 1, DaysInMonth(value.year, value.month));
                break;
            case DatePart::Year:
                value.year = ImClamp(value.year + direction, 1, 9999);
                break;
            case DatePart::Hour:
                value.hour = clock24 ? wrap(value.hour + direction, 0, 23) : wrap(value.hour % 12 + direction, 0, 11) + (value.hour >= 12 ? 12 : 0);
                break;
            case DatePart::Minute:
                value.minute = wrap(value.minute + direction, 0, 59);
                break;
            case DatePart::Meridiem:
                value.hour = (value.hour + 12) % 24;
                break;
        }
        value.day = ImMin(value.day, DaysInMonth(value.year, value.month));
    }

    // Digits typed into a part replace it, two at most (four for the year); returns whether the part is full, so typing
    // moves on to the next one.
    static bool TypePart(DateTime& value, DatePart part, int typed_value, int typed, bool clock24) {
        switch (part) {
            case DatePart::Month:
                value.month = ImClamp(typed_value, 1, 12);
                return typed >= 2 || typed_value > 1;
            case DatePart::Day:
                value.day = ImClamp(typed_value, 1, DaysInMonth(value.year, value.month));
                return typed >= 2 || typed_value > 3;
            case DatePart::Year:
                value.year = ImMax(typed_value, 1);
                return typed >= 4;
            case DatePart::Hour:
                if (clock24)
                    value.hour = ImClamp(typed_value, 0, 23);
                else
                    value.hour = ImClamp(typed_value, 1, 12) % 12 + (value.hour >= 12 ? 12 : 0);
                return typed >= 2 || typed_value > (clock24 ? 2 : 1);
            case DatePart::Minute:
                value.minute = ImClamp(typed_value, 0, 59);
                return typed >= 2 || typed_value > 5;
            case DatePart::Meridiem:
                return true;
        }
        return true;
    }

    // Days since 1970-01-01 of a civil date (Howard Hinnant's days_from_civil), for the weekday a month starts on.
    static int DaysFromCivil(int year, int month, int day) {
        year -= month <= 2 ? 1 : 0;
        const int era = (year >= 0 ? year : year - 399) / 400;
        const int year_of_era = year - era * 400;
        const int day_of_year = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
        const int day_of_era = year_of_era * 365 + year_of_era / 4 - year_of_era / 100 + day_of_year;
        return era * 146097 + day_of_era - 719468;
    }

    // 0 for Sunday, as the en_US calendar starts its weeks.
    static int Weekday(int year, int month, int day) {
        const int days = DaysFromCivil(year, month, day);
        return days >= -4 ? (days + 4) % 7 : (days + 5) % 7 + 6;
    }

    static CalendarDay Today() {
        const CalendarDay pinned = Environment().today;
        if (pinned.year != 0)
            return pinned;
        const std::time_t now = std::time(nullptr);
        std::tm local = {};
        localtime_s(&local, &now);
        return CalendarDay{local.tm_year + 1900, local.tm_mon + 1, local.tm_mday};
    }

    // The month a calendar shows, which its arrows change apart from the value.
    struct CalendarState {
        int year = 0;
        int month = 0;
    };

    // A month's calendar: the month and year between the arrows, the weekday letters, and six weeks of days, those of
    // the neighbouring months dimmed; a click on a day picks it (and its month).
    static bool Calendar(ImGuiID id, const ImRect& rect, DateTime* value) {
        const Metrics::DatePickerMetrics& metrics = Metrics::DatePicker();
        const Palette& colors = Theme::Colors();
        const bool enabled = Environment().enabled;
        ImDrawList* draw = ImGui::GetWindowDrawList();
        CalendarState& state = State::Get<CalendarState>(id);
        if (state.year == 0) {
            state.year = value->year;
            state.month = value->month;
        }
        const DateTime previous = *value;
        ImGui::PushID(int(id));

        // The header: chevrons at the ends, the month and year between them.
        static const char* const MonthNames[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
        const float header = Px(metrics.calendarHeader);
        const float column = Px(metrics.calendarColumn);
        const Font chevron_font = Font::System(metrics.calendarHeaderSize);
        for (const int direction : {-1, 1}) {
            const float left = direction < 0 ? rect.Min.x : rect.Max.x - column;
            const ImRect arrow(left, rect.Min.y, left + column, rect.Min.y + header);
            const Interaction::Response response = Interaction::Button(ImGui::GetID(direction), arrow);
            if (response.pressed) {
                state.month += direction;
                if (state.month < 1) {
                    state.month = 12;
                    --state.year;
                } else if (state.month > 12) {
                    state.month = 1;
                    ++state.year;
                }
            }
            const Rgba color = !enabled ? colors.tertiaryLabel : response.held && response.hovered ? colors.label : colors.secondaryLabel;
            Typography::DrawSymbol(draw, direction < 0 ? Symbols::ChevronLeft : Symbols::ChevronRight, chevron_font, arrow, color);
        }
        char title[32];
        std::snprintf(title, sizeof(title), "%s %d", MonthNames[state.month - 1], state.year);
        Typography::Draw(draw, Font::System(metrics.calendarHeaderSize, FontWeight::Semibold), ImRect(rect.Min.x, rect.Min.y, rect.Max.x, rect.Min.y + header), enabled ? colors.secondaryLabel : colors.tertiaryLabel, title, TextAlignment::Center);

        // Weekday letters, then six weeks from the Sunday on or before the first of the month.
        static const char* const Letters[] = {"S", "M", "T", "W", "T", "F", "S"};
        const float row = Px(metrics.calendarRow);
        const Font weekday_font = Font::System(metrics.calendarWeekdaySize, FontWeight::Semibold);
        for (int d = 0; d < 7; ++d) {
            const float left = rect.Min.x + column * float(d);
            Typography::Draw(draw, weekday_font, ImRect(left, rect.Min.y + header, left + column, rect.Min.y + header + row), enabled ? colors.secondaryLabel : colors.tertiaryLabel, Letters[d], TextAlignment::Center);
        }
        const CalendarDay today = Today();
        const int lead = Weekday(state.year, state.month, 1);
        const Font day_font = Font::System(metrics.calendarDaySize);
        const Font chosen_font = Font::System(metrics.calendarDaySize, FontWeight::Semibold);
        for (int cell = 0; cell < 42; ++cell) {
            // The civil date of the cell, from its day number.
            int year = state.year;
            int month = state.month;
            int day = cell - lead + 1;
            if (day < 1) {
                month = month == 1 ? 12 : month - 1;
                year -= state.month == 1 ? 1 : 0;
                day += DaysInMonth(year, month);
            } else if (day > DaysInMonth(state.year, state.month)) {
                day -= DaysInMonth(state.year, state.month);
                month = month == 12 ? 1 : month + 1;
                year += state.month == 12 ? 1 : 0;
            }
            const float left = rect.Min.x + column * float(cell % 7);
            const float top = rect.Min.y + header + row * float(1 + cell / 7);
            const ImRect frame(left, top, left + column, top + row);
            const Interaction::Response response = Interaction::Button(ImGui::GetID(100 + cell), frame);
            if (response.pressed) {
                value->year = year;
                value->month = month;
                value->day = day;
                state.year = year;
                state.month = month;
            }
            const bool chosen = value->year == year && value->month == month && value->day == day;
            const bool is_today = today.year == year && today.month == month && today.day == day;
            const bool in_month = month == state.month;
            Rgba color = !in_month ? colors.tertiaryLabel : is_today ? colors.accent : colors.label;
            if (chosen) {
                const float radius = Px(metrics.calendarDisc) * 0.5f;
                Draw::FillCircle(draw, frame.GetCenter(), radius, response.held && response.hovered ? colors.accentPressed : colors.controlAccent);
                color = colors.selectedContent;
            } else if (response.held && response.hovered) {
                Draw::FillCircle(draw, frame.GetCenter(), Px(metrics.calendarDisc) * 0.5f, colors.quaternaryFill);
            }
            if (!enabled)
                color = colors.tertiaryLabel;
            char number[4];
            std::snprintf(number, sizeof(number), "%d", day);
            Typography::Draw(draw, chosen || is_today ? chosen_font : day_font, frame, color, number, TextAlignment::Center);
        }
        ImGui::PopID();
        return std::memcmp(&previous, value, sizeof(DateTime)) != 0;
    }

    // The widest text a field can show, so it keeps its width as the value changes.
    static float FieldTextWidth(const Font& font, bool date, bool clock24) {
        if (date)
            return Typography::Width(font, "00/00/0000");
        if (clock24)
            return Typography::Width(font, "00:00");
        return ImMax(Typography::Width(font, "00:00\xE2\x80\xAF" "AM"), Typography::Width(font, "00:00\xE2\x80\xAF" "PM"));
    }

    // One field: the parts on the trailing side of its outline, the arrows after them.
    static bool DateField(ImGuiID id, const ImRect& field, DateTime* value, bool date, bool clock24) {
        ImGuiContext& g = *GImGui;
        const ImGuiIO& io = g.IO;
        const Metrics::StepperMetrics& stepper = Metrics::Stepper();
        const Metrics::DatePickerMetrics& metrics = Metrics::DatePicker();
        const Font font = Font::Style(TextStyle::Body);
        const std::vector<FieldPart> parts = Parts(date, clock24);
        DateFieldState& state = State::Get<DateFieldState>(id);
        state.selected = ImClamp(state.selected, 0, int(parts.size()) - 1);
        const DateTime previous = *value;
        const bool enabled = Environment().enabled;

        const ImVec2 arrows_min(field.Max.x - Px(stepper.fieldTrailing + stepper.size.x), field.GetCenter().y - Px(stepper.size.y) * 0.5f);
        const ImRect arrows(arrows_min, arrows_min + Px(stepper.size));
        const float line_top = field.GetCenter().y - Px(font.lineHeight) * 0.5f;
        const ImRect line(field.Min.x + Px(stepper.fieldInset), line_top, arrows.Min.x - Px(stepper.fieldGap), line_top + Px(font.lineHeight));

        // The parts laid out from the trailing edge of the text line.
        std::vector<std::string> texts;
        std::vector<float> starts;
        float width = 0.0f;
        for (const FieldPart& part : parts) {
            texts.push_back(PartText(*value, part.part, clock24));
            width += Typography::Width(font, texts.back()) + Typography::Width(font, part.separator);
        }
        float x = line.Max.x - width;
        for (size_t i = 0; i < parts.size(); ++i) {
            starts.push_back(x);
            x += Typography::Width(font, texts[i]) + Typography::Width(font, parts[i].separator);
        }

        // A click on the text focuses the field and selects the part under the pointer with the separator after it: the
        // first part left of the text, the last one right of it.
        const ImRect hit(field.Min, ImVec2(arrows.Min.x, field.Max.y));
        if (!ImGui::ItemAdd(hit, id, nullptr, enabled ? ImGuiItemFlags_None : ImGuiItemFlags_Disabled))
            return false;
        const bool hovered = ImGui::ItemHoverable(hit, id, g.LastItemData.ItemFlags);
        if (hovered && io.MouseClicked[ImGuiMouseButton_Left]) {
            ImGui::SetActiveID(id, g.CurrentWindow);
            ImGui::SetFocusID(id, g.CurrentWindow);
            state.typed = 0;
            state.selected = 0;
            for (size_t i = 1; i < parts.size(); ++i) {
                if (io.MousePos.x >= starts[i])
                    state.selected = int(i);
            }
        } else if (g.ActiveId == id && ((io.MouseClicked[ImGuiMouseButton_Left] && !hovered) || !enabled)) {
            ImGui::ClearActiveID();
        }

        // The arrows step the selected part and give the field focus.
        const int step = StepperArrows(id, arrows);
        if (step != 0) {
            if (g.ActiveId != id) {
                ImGui::SetActiveID(id, g.CurrentWindow);
                ImGui::SetFocusID(id, g.CurrentWindow);
            }
            state.typed = 0;
            StepPart(*value, parts[size_t(state.selected)].part, step, clock24);
        }

        const bool active = g.ActiveId == id;
        if (active) {
            for (const ImGuiKey key : {ImGuiKey_LeftArrow, ImGuiKey_RightArrow, ImGuiKey_UpArrow, ImGuiKey_DownArrow})
                ImGui::SetKeyOwner(key, id);
            g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right) | (1 << ImGuiDir_Up) | (1 << ImGuiDir_Down);
            const int count = int(parts.size());
            const auto select = [&](int part) {
                state.selected = ImClamp(part, 0, count - 1);
                state.typed = 0;
            };
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
                select(state.selected - 1);
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
                select(state.selected + 1);
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
                state.typed = 0;
                StepPart(*value, parts[size_t(state.selected)].part, ImGui::IsKeyPressed(ImGuiKey_UpArrow) ? 1 : -1, clock24);
            }
            if (ImGui::Shortcut(ImGuiKey_Tab, ImGuiInputFlags_None, id)) {
                if (state.selected + 1 < count)
                    select(state.selected + 1);
                else
                    ImGui::ClearActiveID();
            }
            if (ImGui::Shortcut(ImGuiKey_Escape, ImGuiInputFlags_None, id) || ImGui::Shortcut(ImGuiKey_Enter, ImGuiInputFlags_None, id))
                ImGui::ClearActiveID();
            // Digits type the selected part; A and P choose the half of the day.
            for (const ImWchar character : io.InputQueueCharacters) {
                const DatePart part = parts[size_t(state.selected)].part;
                if (part == DatePart::Meridiem) {
                    if (character == 'a' || character == 'A')
                        value->hour %= 12;
                    if (character == 'p' || character == 'P')
                        value->hour = value->hour % 12 + 12;
                } else if (character >= '0' && character <= '9') {
                    state.typedValue = (state.typed > 0 ? state.typedValue * 10 : 0) + int(character - '0');
                    ++state.typed;
                    if (TypePart(*value, part, state.typedValue, state.typed, clock24))
                        select(state.selected + 1 < count ? state.selected + 1 : state.selected);
                }
            }
            g.IO.InputQueueCharacters.resize(0);
        }

        // The outline, the ring while focused, and the parts, the selected one on an accent plate.
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        DrawValueField(draw, field);
        if (g.ActiveId == id)
            Draw::FocusRing(draw, field, CornerRadii(Px(stepper.fieldRadius)), colors.accent);
        texts.clear();
        for (const FieldPart& part : parts)
            texts.push_back(PartText(*value, part.part, clock24));
        x = line.Max.x;
        for (int i = int(parts.size()) - 1; i >= 0; --i)
            x -= Typography::Width(font, texts[size_t(i)]) + Typography::Width(font, parts[size_t(i)].separator);
        const Rgba text_color = colors.LabelColor(enabled);
        for (size_t i = 0; i < parts.size(); ++i) {
            const float part_width = Typography::Width(font, texts[i]);
            const ImRect frame(x, line.Min.y, x + part_width, line.Max.y);
            const bool selected = g.ActiveId == id && int(i) == state.selected;
            if (selected) {
                const float pad = Px(metrics.partPadding);
                Draw::FillRoundedRect(draw, ImRect(frame.Min.x - pad, frame.Min.y, frame.Max.x + pad, frame.Max.y), CornerRadii(Px(metrics.partRadius)), colors.accent);
            }
            Typography::Draw(draw, font, frame, selected ? colors.selectedContent : text_color, texts[i]);
            x += part_width;
            Typography::Draw(draw, font, ImRect(x, line.Min.y, x + Typography::Width(font, parts[i].separator), line.Max.y), text_color, parts[i].separator);
            x += Typography::Width(font, parts[i].separator);
        }
        return std::memcmp(&previous, value, sizeof(DateTime)) != 0;
    }

    // The graphical style: the calendar, the time field under it when the time is shown.
    static bool GraphicalDatePicker(const char* label, DateTime* value, const DatePickerOptions& options) {
        const Metrics::StepperMetrics& stepper = Metrics::Stepper();
        const Metrics::DatePickerMetrics& metrics = Metrics::DatePicker();
        const bool time = options.components != DatePickerComponents::Date;
        const float time_width = stepper.fieldInset + ImCeil(Pt(FieldTextWidth(Font::Style(TextStyle::Body), false, options.uses24HourClock))) + stepper.fieldGap + stepper.size.x + stepper.fieldTrailing;
        const ImVec2 size(7.0f * metrics.calendarColumn, metrics.calendarHeader + 7.0f * metrics.calendarRow + (time ? metrics.calendarTimeSpacing + stepper.fieldHeight : 0.0f));
        const Metrics::FormMetrics& form = Metrics::Form();
        const std::optional<ImRect> placed = PlaceLabeledControl({.label = Interaction::VisibleLabel(label), .accessorySize = size, .trailingInset = form.valueTrailing, .accessoryTop = form.rowVerticalInset}, stepper.labelSpacing);
        if (!placed)
            return false;
        ImGui::PushID(label);
        bool changed = Calendar(ImGui::GetID("calendar"), ImRect(placed->Min, ImVec2(placed->Max.x, placed->Min.y + Px(metrics.calendarHeader + 7.0f * metrics.calendarRow))), value);
        if (time) {
            const float top = placed->Max.y - Px(stepper.fieldHeight);
            const float left = placed->GetCenter().x - Px(time_width) * 0.5f;
            changed |= DateField(ImGui::GetID("time"), ImRect(left, top, left + Px(time_width), placed->Max.y), value, false, options.uses24HourClock);
        }
        ImGui::PopID();
        return changed;
    }

    bool DatePicker(const char* label, DateTime* value, const DatePickerOptions& options) {
        if (options.style == DatePickerStyle::Graphical)
            return GraphicalDatePicker(label, value, options);
        const Metrics::StepperMetrics& stepper = Metrics::Stepper();
        const Metrics::DatePickerMetrics& metrics = Metrics::DatePicker();
        const Metrics::FormMetrics& form = Metrics::Form();
        const Font font = Font::Style(TextStyle::Body);
        const bool date = options.components != DatePickerComponents::HourAndMinute;
        const bool time = options.components != DatePickerComponents::Date;
        const auto field_width = [&](bool is_date) {
            return stepper.fieldInset + ImCeil(Pt(FieldTextWidth(font, is_date, options.uses24HourClock))) + stepper.fieldGap + stepper.size.x + stepper.fieldTrailing;
        };
        const float date_width = date ? field_width(true) : 0.0f;
        const float time_width = time ? field_width(false) : 0.0f;
        const ImVec2 accessory(date_width + time_width + (date && time ? metrics.fieldSpacing : 0.0f), stepper.fieldHeight);
        const std::optional<ImRect> placed = PlaceLabeledControl({.label = Interaction::VisibleLabel(label), .accessorySize = accessory, .trailingInset = form.valueTrailing, .accessoryTop = (form.rowHeight - accessory.y) * 0.5f}, stepper.labelSpacing);
        if (!placed)
            return false;
        bool changed = false;
        ImGui::PushID(label);
        float x = placed->Min.x;
        if (date) {
            changed |= DateField(ImGui::GetID("date"), ImRect(x, placed->Min.y, x + Px(date_width), placed->Max.y), value, true, options.uses24HourClock);
            x += Px(date_width + metrics.fieldSpacing);
        }
        if (time)
            changed |= DateField(ImGui::GetID("time"), ImRect(x, placed->Min.y, x + Px(time_width), placed->Max.y), value, false, options.uses24HourClock);
        ImGui::PopID();
        return changed;
    }
} // namespace Cupertino
