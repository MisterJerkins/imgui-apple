#include "ComboBox.h"

#include "Bezel.h"
#include "LabeledContent.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/TextInput.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "overlays/PopUpMenu.h"

namespace Cupertino {
    bool ComboBox(const char* label, std::string* text, std::span<const char* const> items, const ComboBoxOptions& options) {
        const Metrics::BezelPopUpMetrics metrics = Metrics::BezelPopUp();
        const Metrics::FormMetrics& form = Metrics::Form();
        const Font font = Font::Style(TextStyle::Body);
        const float width = options.width > 0.0f ? options.width : Pt(Typography::WidestWidth(font, items)) + metrics.comboTextInset + metrics.gap + metrics.indicator + metrics.comboTrailingInset;
        const ImVec2 size(width, metrics.height);
        const std::optional<ImRect> placed = PlaceLabeledControl({.label = Interaction::VisibleLabel(label), .accessorySize = size, .trailingInset = form.valueTrailing, .accessoryTop = (form.rowHeight - size.y) * 0.5f}, Metrics::TextField().labelSpacing);
        if (!placed)
            return false;

        const ImRect frame = *placed;
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const ImGuiID id = ImGui::GetID(label);
        const bool open = PopUpMenu::IsOpen(id);
        Bezel::ComboBox(draw, frame);
        ImGui::PushID(label);
        const ImGuiID field = ImGui::GetID("field");

        // The field takes the button's label; the indicator opens the items under the whole control.
        const Palette& colors = Theme::Colors();
        const TextInput::Style style = {.font = font, .color = colors.LabelColor(Environment().enabled), .placeholder = options.placeholder ? options.placeholder : "", .completions = items};
        const TextInput::Result edit = Environment().enabled ? TextInput::Edit(field, Bezel::ComboBoxText(frame), text, style) : TextInput::Result{};
        bool changed = edit.changed;
        // While the field is edited the ring goes around the whole pill.
        if (edit.focused)
            Bezel::MenuButtonFocusRing(draw, frame);
        if (!Environment().enabled)
            Typography::Draw(draw, font, Bezel::ComboBoxText(frame), colors.tertiaryLabel, *text);
        const ImRect button(Bezel::ComboBoxPlate(frame).Min.x - Px(metrics.gap * 0.5f), frame.Min.y, frame.Max.x, frame.Max.y);
        if (Interaction::Button(ImGui::GetID("button"), button, ImGuiButtonFlags_PressedOnClick, ImGuiItemFlags_NoTabStop).pressed && !open)
            PopUpMenu::Open(id, Bezel::MenuButtonBezel(frame), -1, PopUpMenu::Placement::ComboBoxList, Bezel::ComboBoxText(frame).Min.x, Pt(frame.GetWidth()) + Metrics::Menu().listExtraWidth);
        ImGui::PopID();

        const int chosen = PopUpMenu::Show(id, items);
        if (chosen >= 0 && *text != items[size_t(chosen)]) {
            *text = items[size_t(chosen)];
            changed = true;
        }
        return changed;
    }

    bool ComboBox(const char* label, std::string* text, std::initializer_list<const char*> items, const ComboBoxOptions& options) {
        return ComboBox(label, text, std::span<const char* const>(items.begin(), items.size()), options);
    }
} // namespace Cupertino
