#include "Stepper.h"

#include "Bezel.h"
#include "LabeledContent.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/Symbols.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"

namespace Cupertino {
    // The kit's Controls/Stepper: a white pill split by a line with a faint shade on both sides of the split. The
    // kit fades a disabled pill twice (the pill and its group), the split once.
    static void DrawArrows(ImDrawList* draw, const ImRect& rect, int pressed_half) {
        const Metrics::StepperMetrics& metrics = Metrics::Stepper();
        const Palette& colors = Theme::Colors();
        const CornerRadii radii(Px(metrics.radius));
        const float middle = rect.GetCenter().y;
        {
            const Interaction::DisabledFade fade(0.25f);
            Bezel::Pill(draw, rect, radii, false);
        }
        {
            const Interaction::DisabledFade fade;
            if (pressed_half != 0) {
                draw->PushClipRect(ImVec2(rect.Min.x, pressed_half > 0 ? rect.Min.y : middle), ImVec2(rect.Max.x, pressed_half > 0 ? middle : rect.Max.y), true);
                Draw::FillRoundedRect(draw, rect, radii, colors.controlPressed);
                draw->PopClipRect();
            }
            Draw::FillRect(draw, ImRect(rect.Min.x, middle - Px(0.5f), rect.Max.x, middle + Px(0.5f)), Rgba::Black(0.12f));
            Draw::FillRect(draw, ImRect(rect.Min.x, middle - Px(1.0f), rect.Max.x, middle + Px(1.0f)), Rgba::Black(0.05f));
            Draw::FillVerticalGradient(draw, ImRect(rect.Min.x, middle - Px(4.0f), rect.Max.x, middle), 0.0f, Rgba::White(0.0512f), Rgba::Black(0.0512f));
            Draw::FillVerticalGradient(draw, ImRect(rect.Min.x, middle, rect.Max.x, middle + Px(4.0f)), 0.0f, Rgba::Black(0.0512f), Rgba::White(0.0512f));
        }
        const float x = rect.Min.x + Px(metrics.chevronX);
        Bezel::UpDownChevrons(draw, ImRect(x, rect.Min.y, x + Px(metrics.chevrons.frame.x), rect.Max.y), colors.LabelColor(Environment().enabled), metrics.chevrons);
    }

    void DrawValueField(ImDrawList* draw, const ImRect& rect) {
        Draw::StrokeRoundedRect(draw, rect, CornerRadii(Px(Metrics::Stepper().fieldRadius)), Theme::Colors().separator, Px(1.0f));
    }

    int StepperArrows(ImGuiID id, const ImRect& rect) {
        const float middle = rect.GetCenter().y;
        ImGui::PushID(int(id));
        ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, true);
        const Interaction::Response up = Interaction::Button(ImGui::GetID("up"), ImRect(rect.Min, ImVec2(rect.Max.x, middle)), ImGuiButtonFlags_PressedOnClick);
        const Interaction::Response down = Interaction::Button(ImGui::GetID("down"), ImRect(ImVec2(rect.Min.x, middle), rect.Max), ImGuiButtonFlags_PressedOnClick);
        ImGui::PopItemFlag();
        ImGui::PopID();
        DrawArrows(ImGui::GetWindowDrawList(), rect, up.held && up.hovered ? 1 : down.held && down.hovered ? -1 : 0);
        if (up.focused || down.focused)
            Draw::FocusRing(ImGui::GetWindowDrawList(), rect, CornerRadii(Px(Metrics::Stepper().radius)), Theme::Colors().accent);
        return up.pressed ? 1 : down.pressed ? -1 : 0;
    }

    // Shared by every value type, the way ImGui's scalar widgets are.
    static void StepperScalar(const char* label, ImGuiDataType type, void* value, const void* min, const void* max, const void* step, const char* format) {
        const Metrics::StepperMetrics& metrics = Metrics::Stepper();
        const Font font = Font::Style(TextStyle::Body);
        const std::string_view text = Interaction::VisibleLabel(label);
        // The value field is as wide as the longest bound, so the arrows stay put while the value changes.
        char formatted[64];
        float field_width = 0.0f;
        if (format) {
            ImGui::DataTypeFormatString(formatted, IM_ARRAYSIZE(formatted), type, min, format);
            field_width = Pt(Typography::Width(font, formatted));
            ImGui::DataTypeFormatString(formatted, IM_ARRAYSIZE(formatted), type, max, format);
            field_width = ImCeil(ImMax(field_width, Pt(Typography::Width(font, formatted))));
        }
        const ImVec2 accessory = format ? ImVec2(metrics.fieldInset + field_width + metrics.fieldGap + metrics.size.x + metrics.fieldTrailing, metrics.fieldHeight) : metrics.size;

        const Metrics::FormMetrics& form = Metrics::Form();
        const std::optional<ImRect> placed = PlaceLabeledControl({.label = text, .accessorySize = accessory, .trailingInset = form.valueTrailing, .accessoryTop = (form.rowHeight - accessory.y) * 0.5f}, metrics.labelSpacing);
        if (!placed)
            return;
        const ImRect frame = *placed;
        const ImVec2 arrows_min = format ? ImVec2(frame.Max.x - Px(metrics.fieldTrailing + metrics.size.x), frame.GetCenter().y - Px(metrics.size.y) * 0.5f) : frame.Min;
        const ImRect arrows(arrows_min, arrows_min + Px(metrics.size));

        // The top half steps up, the bottom one down.
        const int direction = StepperArrows(ImGui::GetID(label), arrows);
        if (direction != 0) {
            ImGui::DataTypeApplyOp(type, direction > 0 ? '+' : '-', value, value, step);
            ImGui::DataTypeClamp(type, value, min, max);
        }

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        if (format) {
            DrawValueField(draw, frame);
            const float value_top = frame.GetCenter().y - Px(font.lineHeight) * 0.5f;
            const ImRect value_frame(frame.Min.x + Px(metrics.fieldInset), value_top, frame.Min.x + Px(metrics.fieldInset + field_width), value_top + Px(font.lineHeight));
            ImGui::DataTypeFormatString(formatted, IM_ARRAYSIZE(formatted), type, value, format);
            Typography::Draw(draw, font, value_frame, colors.LabelColor(Environment().enabled), formatted, TextAlignment::Trailing);
        }
    }

    bool Stepper(const char* label, int* value, int min, int max, int step, const char* format) {
        const int previous = *value;
        StepperScalar(label, ImGuiDataType_S32, value, &min, &max, &step, format);
        return *value != previous;
    }

    bool Stepper(const char* label, float* value, float min, float max, float step, const char* format) {
        const float previous = *value;
        StepperScalar(label, ImGuiDataType_Float, value, &min, &max, &step, format);
        return *value != previous;
    }
} // namespace Cupertino
