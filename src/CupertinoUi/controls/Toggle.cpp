#include "Toggle.h"

#include "Bezel.h"
#include "Button.h"
#include "LabeledContent.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/MenuContent.h"
#include "core/Metrics.h"
#include "core/Motion.h"
#include "core/State.h"
#include "core/Symbols.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"

namespace Cupertino {
    struct SwitchState {
        AnimatedFloat position;
    };

    static ToggleStyle Resolve(ToggleStyle style) {
        if (style != ToggleStyle::Automatic)
            return style;
        const EnvironmentValues& environment = Environment();
        return environment.insideForm || environment.platform == Platform::IOS ? ToggleStyle::Switch : ToggleStyle::Checkbox;
    }

    // The knob slides from 0 (off) to 1 (on); shadows and track colors cross-fade with it. A disabled switch loses
    // the accent: when on, its knob stays right over the gray track, as the kit draws it.
    static void DrawSwitch(ImDrawList* draw, const ImRect& track, float knob_size, float position, bool enabled) {
        const Palette& colors = Theme::Colors();
        const CornerRadii radii(track.GetHeight() * 0.5f);
        const CornerStyle style = CornerStyle::Circular;
        const float tint = enabled ? position : 0.0f;
        Draw::FillRoundedRect(draw, track, radii, colors.switchTrack.Opacity(1.0f - tint), style);
        Draw::FillVerticalGradient(draw, track, radii, colors.switchOnTop.Opacity(tint), colors.switchOnBottom.Opacity(tint), style);
        Draw::InnerShadows(draw, track, radii, Theme::SwitchTrackInnerShadows(false), style, 1.0f - tint);
        Draw::InnerShadows(draw, track, radii, Theme::SwitchTrackInnerShadows(true), style, tint);

        const float inset = (track.GetHeight() - knob_size) * 0.5f;
        const float x = ImLerp(track.Min.x + inset, track.Max.x - inset - knob_size, position);
        const ImRect knob(x, track.Min.y + inset, x + knob_size, track.Min.y + inset + knob_size);
        const CornerRadii knob_radii(knob_size * 0.5f);
        Draw::DropShadows(draw, knob, knob_radii, Theme::KnobShadows(false), style, 1.0f - tint, true);
        Draw::DropShadows(draw, knob, knob_radii, Theme::KnobShadows(true), style, tint, true);
        Draw::FillRoundedRect(draw, knob, knob_radii, colors.knob, style);
        Draw::StrokeRoundedRect(draw, knob, knob_radii, colors.controlBorder, Px(0.5f), StrokeAlignment::Outside, style);
    }

    // Handles the click on a switch and returns the animated knob position.
    static float SwitchBehavior(ImGuiID id, const ImRect& track, bool* is_on, bool* changed) {
        const Interaction::Response response = Interaction::Button(id, track);
        if (response.pressed) {
            *is_on = !*is_on;
            *changed = true;
        }
        SwitchState& state = State::Get<SwitchState>(id);
        const float position = state.position.Update(*is_on ? 1.0f : 0.0f, Animation::EaseInOut(0.2f));
        if (response.focused)
            Draw::FocusRing(ImGui::GetWindowDrawList(), track, CornerRadii(track.GetHeight() * 0.5f), Theme::Colors().accent, CornerStyle::Circular);
        return position;
    }

    // In a form section the switch is the trailing accessory of a row, as in System Settings: a mini switch, or a
    // regular one 10 pt from the top in a row with a large icon (the header of the Bluetooth pane, @2x).
    static bool SwitchRow(const char* label, bool* is_on, const ToggleOptions& options) {
        const Metrics::FormMetrics& form = Metrics::Form();
        const bool header = !options.icon.IsEmpty();
        const Metrics::SwitchMetrics metrics = Metrics::Switch(header ? ControlSize::Regular : ControlSize::Mini);
        const FormRowSpec spec = {
            .label = Interaction::VisibleLabel(label),
            .description = options.description ? options.description : "",
            .descriptionStyle = options.descriptionStyle,
            .icon = options.icon,
            .accessorySize = metrics.track,
            .trailingInset = form.switchTrailing,
            .accessoryTop = header ? form.rowVerticalInset : form.switchTop,
        };
        const FormRowFrame row = PlaceFormRow(spec);
        if (Layout::IsMeasuring())
            return false;
        bool changed = false;
        DrawFormRowLabel(row, spec);
        const Interaction::DisabledFade fade;
        const float position = SwitchBehavior(ImGui::GetID(label), row.accessory, is_on, &changed);
        DrawSwitch(ImGui::GetWindowDrawList(), row.accessory, Px(metrics.knob), position, Environment().enabled);
        return changed;
    }

    static bool LabeledSwitch(const char* label, bool* is_on) {
        const Metrics::SwitchMetrics metrics = Metrics::Switch(Environment().controlSize);
        const std::optional<ImRect> track = PlaceLabeledControl({.label = Interaction::VisibleLabel(label), .accessorySize = metrics.track}, metrics.labelSpacing);
        if (!track)
            return false;
        bool changed = false;
        const Interaction::DisabledFade fade;
        const float position = SwitchBehavior(ImGui::GetID(label), *track, is_on, &changed);
        DrawSwitch(ImGui::GetWindowDrawList(), *track, Px(metrics.knob), position, Environment().enabled);
        return changed;
    }

    static bool Checkbox(const char* label, bool* is_on, bool mixed, bool radio) {
        const ControlSize control_size = Environment().controlSize;
        const Metrics::CheckboxMetrics metrics = Metrics::Checkbox(control_size, Environment().insideForm);
        const std::string_view text = Interaction::VisibleLabel(label);
        const Font font = control_size == ControlSize::Small ? Font::System(11.0f) : control_size == ControlSize::Mini ? Font::System(9.0f) : Font::Style(TextStyle::Body);
        const float text_width = text.empty() ? 0.0f : Px(metrics.labelSpacing) + Typography::Width(font, text);
        // Without a title the checkbox is just its box (the listening modes of AirPods @2x).
        const float height = text.empty() ? metrics.box : metrics.height;
        const ImRect rect = Layout::Place(Layout::Placement{.size = ImVec2(Px(metrics.box) + text_width, Px(height)), .baseline = text.empty() ? -1.0f : Typography::CenteredBaseline(font, Px(height))});
        if (Layout::IsMeasuring())
            return false;

        const ImGuiID id = ImGui::GetID(label);
        const Interaction::Response response = Interaction::Button(id, rect);
        const bool was_on = *is_on;
        if (response.pressed)
            *is_on = radio || mixed ? true : !*is_on;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const float box_top = rect.Min.y + (rect.GetHeight() - Px(metrics.box)) * 0.5f;
        const ImRect box(rect.Min.x, box_top, rect.Min.x + Px(metrics.box), box_top + Px(metrics.box));
        const CornerStyle style = radio ? CornerStyle::Circular : CornerStyle::Continuous;
        const CornerRadii radii(radio ? box.GetWidth() * 0.5f : Px(metrics.radius));
        const Bezel::Mark mark = mixed ? Bezel::Mark::Dash : !*is_on ? Bezel::Mark::None : radio ? Bezel::Mark::Dot : Bezel::Mark::Check;
        Bezel::Check(draw, box, radii, mark, response.held && response.hovered, style);
        if (!text.empty()) {
            const ImRect label_rect(box.Max.x + Px(metrics.labelSpacing), rect.Min.y, rect.Max.x, rect.Max.y);
            Typography::Draw(draw, font, label_rect, colors.LabelColor(Environment().enabled), text);
        }
        if (response.focused)
            Draw::FocusRing(draw, box, radii, colors.accent, style);
        return *is_on != was_on || (response.pressed && mixed);
    }

    bool Toggle(const char* label, bool* is_on, const ToggleOptions& options) {
        // In a menu a toggle is an item with a checkmark.
        if (MenuContent::IsCollecting()) {
            if (!MenuContent::AddItem({.title = std::string(Interaction::VisibleLabel(label)), .checkable = true, .checked = *is_on}))
                return false;
            *is_on = !*is_on;
            return true;
        }
        const ToggleStyle style = Resolve(options.style);
        if (style == ToggleStyle::Switch)
            return Layout::ParentRole() == Layout::Role::FormSection ? SwitchRow(label, is_on, options) : LabeledSwitch(label, is_on);
        if (style == ToggleStyle::Button || style == ToggleStyle::TitleButton)
            return ButtonToggle(label, is_on, style == ToggleStyle::Button);
        if (style == ToggleStyle::AccessoryBar)
            return AccessoryBarToggle(label, is_on);
        return Checkbox(label, is_on, options.mixed, style == ToggleStyle::Radio);
    }
} // namespace Cupertino
