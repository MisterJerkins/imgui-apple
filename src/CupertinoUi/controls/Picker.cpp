#include "Picker.h"

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
#include "overlays/PopUpMenu.h"

#include <functional>

namespace Cupertino {
    static const char* CurrentItem(int selection, std::span<const char* const> items) {
        return items.empty() ? "" : items[size_t(ImClamp(selection, 0, int(items.size()) - 1))];
    }

    // Opens the menu on press and applies the item picked in it; shared by every pop-up look. The menu ends at
    // menu_limit (the indicator of a form pop-up).
    static bool PopUpBehavior(ImGuiID id, const ImRect& button, const ImRect& label_frame, float menu_limit, int* selection, std::span<const char* const> items, const PickerOptions& options, bool* open) {
        const Interaction::Response response = Interaction::Button(id, button, ImGuiButtonFlags_PressedOnClick);
        if (response.pressed && !PopUpMenu::IsOpen(id))
            PopUpMenu::Open(id, label_frame, *selection, PopUpMenu::Placement::OverLabel, menu_limit);
        *open = PopUpMenu::IsOpen(id);
        const int picked = PopUpMenu::Show(id, items, options.itemImage, options.itemImageSize);
        if (picked < 0 || picked == *selection)
            return false;
        *selection = picked;
        return true;
    }

    // The value width of a borderless pop-up in points: an item image, then the current item's title.
    static float BorderlessValueWidth(int selection, std::span<const char* const> items, const PickerOptions& options) {
        const Metrics::FormPopUpMetrics& metrics = Metrics::FormPopUp();
        const float image_width = options.itemImage ? options.itemImageSize.x + metrics.imageSpacing : 0.0f;
        return image_width + Pt(Typography::Width(Font::Style(TextStyle::Body), CurrentItem(selection, items)));
    }

    // The borderless pop-up of a form row on its text line (from the value to the end of the indicator plate): the
    // value, the plate with two chevrons the given drop below the line's top, and the pressed underlay a little wider
    // than both.
    static bool BorderlessPopUp(const char* label, int* selection, std::span<const char* const> items, const PickerOptions& options, const ImRect& line, float indicator_drop) {
        const Metrics::FormPopUpMetrics& metrics = Metrics::FormPopUp();
        const Font font = Font::Style(TextStyle::Body);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const float image_width = options.itemImage ? options.itemImageSize.x + metrics.imageSpacing : 0.0f;
        const ImRect label_frame(line.Min, ImVec2(line.Min.x + Px(BorderlessValueWidth(*selection, items, options)), line.Max.y));
        const float button_top = line.GetCenter().y - Px(metrics.height) * 0.5f;
        const ImRect button(line.Min.x - Px(metrics.labelInset), button_top, line.Max.x + Px(metrics.trailingInset), button_top + Px(metrics.height));
        const float indicator_left = line.Max.x - Px(metrics.indicator);
        bool open = false;
        const bool changed = PopUpBehavior(ImGui::GetID(label), button, label_frame, options.menuCoversIndicator ? FLT_MAX : indicator_left, selection, items, options, &open);

        if (open)
            Draw::FillRoundedRect(draw, button, CornerRadii(Px(metrics.pressedRadius)), colors.popUpPressed);
        if (Interaction::FocusVisible(ImGui::GetID(label)))
            Draw::FocusRing(draw, button, CornerRadii(Px(metrics.pressedRadius)), colors.accent);
        const bool enabled = Environment().enabled;
        const float indicator_top = line.Min.y + Px(indicator_drop);
        const ImRect indicator(indicator_left, indicator_top, indicator_left + Px(metrics.indicator), indicator_top + Px(metrics.indicator));
        if (options.itemImage) {
            const ImVec2 min(label_frame.Min.x, indicator.GetCenter().y - Px(options.itemImageSize.y) * 0.5f);
            options.itemImage(draw, ImRect(min, min + Px(options.itemImageSize)), *selection);
        }
        const ImRect title(label_frame.Min.x + Px(image_width), label_frame.Min.y, label_frame.Max.x, label_frame.Max.y);
        Typography::Draw(draw, font, title, colors.LabelColor(enabled), CurrentItem(*selection, items));
        // The kit fades only the indicator of a disabled pop-up; the label is already tertiary.
        const Interaction::DisabledFade fade;
        if (!open)
            Draw::FillRoundedRect(draw, indicator, CornerRadii(Px(metrics.indicatorRadius)), colors.popUpIndicator);
        Bezel::UpDownChevrons(draw, indicator, colors.label, metrics.chevrons);
        return changed;
    }

    // System Settings form row: the label, and the borderless pop-up on the trailing side.
    static bool FormPopUpRow(const char* label, int* selection, std::span<const char* const> items, const PickerOptions& options) {
        const Metrics::FormPopUpMetrics& metrics = Metrics::FormPopUp();
        const Font font = Font::Style(TextStyle::Body);
        const std::string_view description = options.description ? options.description : "";
        // The accessory is the button from its leading inset to the plate, on the value's text line.
        const float width = metrics.labelInset + BorderlessValueWidth(*selection, items, options) + metrics.gap + metrics.indicator;
        const FormRowSpec spec = {.label = Interaction::VisibleLabel(label), .description = description, .descriptionSymbol = options.descriptionSymbol, .descriptionSymbolColor = options.descriptionSymbolColor, .icon = options.icon, .iconSize = RowIconSize::Small, .accessorySize = ImVec2(width, font.lineHeight), .trailingInset = metrics.rowTrailing, .accessoryTop = Metrics::Form().rowVerticalInset, .accessorySpacing = metrics.descriptionTrailing};
        const FormRowFrame row = PlaceFormRow(spec);
        if (Layout::IsMeasuring())
            return false;
        DrawFormRowLabel(row, spec);
        const ImRect line(row.accessory.Min.x + Px(metrics.labelInset), row.accessory.Min.y, row.accessory.Max.x, row.accessory.Max.y);
        bool changed = false;
        Disabled(options.controlDisabled, [&] { changed = BorderlessPopUp(label, selection, items, options, line, metrics.indicatorDrop); });
        return changed;
    }

    // The borderless pop-up inside a row. In a bordered list (File Sharing @2x) the indicator plate is centered in the
    // row with the text line its drop above, and a fixed width keeps values and plates of all rows in columns. Next to
    // other content of a form row (AirPods @2x) it takes its whole 20 pt button, the value and the plate both centered.
    static bool InlinePopUp(const char* label, int* selection, std::span<const char* const> items, const PickerOptions& options) {
        const Metrics::FormPopUpMetrics& metrics = Metrics::FormPopUp();
        const Font font = Font::Style(TextStyle::Body);
        const float line_width = options.width > 0.0f ? options.width : BorderlessValueWidth(*selection, items, options) + metrics.gap + metrics.indicator;
        if (Layout::ParentRole() == Layout::Role::ListRow) {
            const ImVec2 size(line_width, font.lineHeight + 2.0f * metrics.indicatorDrop);
            const ImRect frame = Layout::Place(Layout::Placement{.size = Px(size), .baseline = Typography::Baseline(font), .snapsToPixels = false});
            if (Layout::IsMeasuring())
                return false;
            return BorderlessPopUp(label, selection, items, options, ImRect(frame.Min, ImVec2(frame.Max.x, frame.Min.y + Px(font.lineHeight))), metrics.indicatorDrop);
        }
        // The value's text line sits a point above the middle of the button and the plate a point below the line, as in a
        // form row (AirPods and Sound @2x: the value's cap line level with the top of the chevrons).
        const float text_top = (metrics.height - font.lineHeight) * 0.5f - metrics.indicatorDrop;
        const ImVec2 size(metrics.labelInset + line_width + metrics.trailingInset, metrics.height);
        const ImRect frame = Layout::Place(Layout::Placement{.size = Px(size), .baseline = Px(text_top) + Typography::Baseline(font), .snapsToPixels = false});
        if (Layout::IsMeasuring())
            return false;
        const ImVec2 line_min = frame.Min + Px(ImVec2(metrics.labelInset, text_top));
        return BorderlessPopUp(label, selection, items, options, ImRect(line_min, ImVec2(frame.Max.x - Px(metrics.trailingInset), line_min.y + Px(font.lineHeight))), metrics.indicatorDrop);
    }

    // NSPopUpButton outside forms: white bezel, label and an accent plate with white chevrons. The items take the control
    // size's font, the title before the button the body font; a given width is the button's, otherwise it fits the
    // longest item and narrows to what it is offered, cutting its title.
    static bool BezelPopUp(const char* label, int* selection, std::span<const char* const> items, const PickerOptions& options) {
        const Metrics::BezelPopUpMetrics metrics = Metrics::BezelPopUp();
        const float chrome = metrics.labelInset + metrics.gap + metrics.indicator + metrics.trailingInset;
        const float width = options.width > 0.0f ? options.width : chrome + Pt(Typography::WidestWidth(Font::System(metrics.labelSize), items));
        const std::optional<ImRect> frame = PlaceLabeledControl({.label = Interaction::VisibleLabel(label), .accessorySize = ImVec2(width, metrics.height), .accessoryMinWidth = options.width > 0.0f ? 0.0f : chrome}, metrics.titleSpacing);
        if (!frame)
            return false;
        bool open = false;
        const bool changed = PopUpBehavior(ImGui::GetID(label), *frame, Bezel::MenuButtonLabel(*frame), FLT_MAX, selection, items, options, &open);
        Bezel::MenuButton(ImGui::GetWindowDrawList(), *frame, CurrentItem(*selection, items), open, Bezel::MenuIndicator::UpDown, Interaction::FocusVisible(ImGui::GetID(label)));
        return changed;
    }

    // One radio option with its label, whose id is its index; returns true when it changes the selection, by a click or,
    // from the selected option, by the arrows.
    static bool RadioOption(const ImRect& rect, float diameter, float spacing, std::string_view text, int* selection, int index, int count) {
        const bool selected = *selection == index;
        const Interaction::Response response = Interaction::Button(ImGui::GetID(index), rect, 0, selected ? ImGuiItemFlags_None : ImGuiItemFlags_NoTabStop);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const float top = rect.GetCenter().y - diameter * 0.5f;
        const ImRect circle(rect.Min.x, top, rect.Min.x + diameter, top + diameter);
        Bezel::Check(draw, circle, CornerRadii(diameter * 0.5f), selected ? Bezel::Mark::Dot : Bezel::Mark::None, response.held && response.hovered, CornerStyle::Circular);
        const ImRect label_rect(circle.Max.x + spacing, rect.Min.y, rect.Max.x, rect.Max.y);
        Typography::Draw(draw, Font::Style(TextStyle::Body), label_rect, colors.LabelColor(Environment().enabled), text);
        if (response.focused)
            Draw::FocusRing(draw, circle, CornerRadii(diameter * 0.5f), colors.accent, CornerStyle::Circular);
        const int target = response.pressed ? index : response.focused && selected ? Interaction::GroupArrows(index, count) : *selection;
        if (target == *selection)
            return false;
        *selection = target;
        return true;
    }

    // System Settings radio row: the title on top and the options stacked under it.
    static bool RadioGroupRow(const char* label, int* selection, std::span<const char* const> items) {
        const Metrics::FormMetrics& form = Metrics::Form();
        const Metrics::RadioMetrics metrics = Metrics::Radio(ControlSize::Regular, true);
        const float count = float(items.size());
        const float content = form.radioGroupTop + metrics.diameter + metrics.rowPitch * (count - 1.0f) + form.radioGroupBottom;
        Layout::Placement placement;
        placement.size = ImVec2(Layout::FullProposal().x, Px(content));
        placement.ignoresChildInsets = true;
        const ImRect slot = Layout::Place(placement);
        if (Layout::IsMeasuring())
            return false;

        const ImRect title(slot.Min.x + Px(form.rowInset), slot.Min.y + Px(form.rowVerticalInset), slot.Max.x - Px(form.rowInset), slot.Min.y + Px(form.rowVerticalInset + metrics.height));
        const Palette& colors = Theme::Colors();
        Typography::Draw(ImGui::GetWindowDrawList(), Font::Style(TextStyle::Body), title, colors.LabelColor(Environment().enabled), Interaction::VisibleLabel(label));

        bool changed = false;
        ImGui::PushID(label);
        for (size_t i = 0; i < items.size(); ++i) {
            const float top = slot.Min.y + Px(form.radioGroupTop + metrics.rowPitch * float(i) - (metrics.height - metrics.diameter) * 0.5f);
            const float width = Px(metrics.diameter + metrics.labelSpacing) + Typography::Width(Font::Style(TextStyle::Body), items[i]);
            const ImRect option(slot.Min.x + Px(form.rowInset), top, slot.Min.x + Px(form.rowInset) + width, top + Px(metrics.height));
            changed |= RadioOption(option, Px(metrics.diameter), Px(metrics.labelSpacing), items[i], selection, int(i), int(items.size()));
        }
        ImGui::PopID();
        return changed;
    }

    // A form row with its radio buttons side by side on the trailing side (Login window shows, Lock Screen @2x), on the
    // label's line over its description.
    static bool HorizontalRadioRow(const char* label, int* selection, std::span<const char* const> items, const PickerOptions& options) {
        const Metrics::FormMetrics& form = Metrics::Form();
        const Metrics::RadioMetrics metrics = Metrics::Radio(ControlSize::Regular, true);
        const Font font = Font::Style(TextStyle::Body);
        const auto option_width = [&](const char* item) { return metrics.diameter + metrics.labelSpacing + Pt(Typography::Width(font, item)); };
        float width = 0.0f;
        for (const char* item : items)
            width += option_width(item) + (width > 0.0f ? form.radioRowSpacing : 0.0f);
        const std::string_view description = options.description ? options.description : "";
        const FormRowSpec spec = {.label = Interaction::VisibleLabel(label), .description = description, .accessorySize = ImVec2(width, metrics.height), .trailingInset = form.valueTrailing, .accessoryTop = form.rowVerticalInset};
        const FormRowFrame row = PlaceFormRow(spec);
        if (Layout::IsMeasuring())
            return false;
        DrawFormRowLabel(row, spec);
        bool changed = false;
        float x = row.accessory.Min.x;
        ImGui::PushID(label);
        for (size_t i = 0; i < items.size(); ++i) {
            const float option = Px(option_width(items[i]));
            changed |= RadioOption(ImRect(x, row.accessory.Min.y, x + option, row.accessory.Max.y), Px(metrics.diameter), Px(metrics.labelSpacing), items[i], selection, int(i), int(items.size()));
            x += option + Px(form.radioRowSpacing);
        }
        ImGui::PopID();
        return changed;
    }

    static bool RadioGroup(const char* label, int* selection, std::span<const char* const> items) {
        const Metrics::RadioMetrics metrics = Metrics::Radio(Environment().controlSize, Environment().insideForm);
        const Font font = Font::Style(TextStyle::Body);
        const float widest = Typography::WidestWidth(font, items);
        const float count = float(items.size());
        const ImRect rect = Layout::Place(Layout::Placement{.size = ImVec2(Px(metrics.diameter + metrics.labelSpacing) + widest, Px(metrics.height + metrics.rowPitch * (count - 1.0f))), .baseline = Typography::CenteredBaseline(font, Px(metrics.height))});
        if (Layout::IsMeasuring())
            return false;

        bool changed = false;
        ImGui::PushID(label);
        for (size_t i = 0; i < items.size(); ++i) {
            const float top = rect.Min.y + Px(metrics.rowPitch * float(i));
            const ImRect option(rect.Min.x, top, rect.Max.x, top + Px(metrics.height));
            changed |= RadioOption(option, Px(metrics.diameter), Px(metrics.labelSpacing), items[i], selection, int(i), int(items.size()));
        }
        ImGui::PopID();
        return changed;
    }

    struct SegmentedState {
        AnimatedFloat position;
    };

    // Separator i sits before segment i and fades out as the selection comes next to it.
    static float SeparatorVisibility(float position, int i) {
        return ImSaturate(ImMin(ImFabs(position - float(i)), ImFabs(position - float(i - 1))));
    }

    // Where the selection indicator is, in segments: it slides to a new selection.
    static float SelectionPosition(const char* label, int selection, int count) {
        SegmentedState& state = State::Get<SegmentedState>(ImGui::GetID(label));
        return state.position.Update(float(ImClamp(selection, 0, count - 1)), Animation::EaseOut(0.2f));
    }

    // The segments' hit areas and centered titles, width wide and pitch apart, and the focus ring around the control
    // (its corners radii) while a segment has the keyboard focus; true when a click changes the selection.
    static bool SegmentTitles(const char* label, int* selection, std::span<const char* const> items, const ImRect& control, const CornerRadii& radii, float pitch, float width, const std::function<Rgba(int)>& color, SymbolScale scale = SymbolScale::Medium) {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Font font = Font::Style(TextStyle::Body).ImageScale(scale);
        bool changed = false;
        bool focused = false;
        ImGui::PushID(label);
        for (int i = 0; i < int(items.size()); ++i) {
            const float left = control.Min.x + Px(pitch * float(i));
            const ImRect segment(left, control.Min.y, left + Px(width), control.Max.y);
            const Interaction::Response response = Interaction::Button(ImGui::GetID(i), segment, 0, i == *selection ? ImGuiItemFlags_None : ImGuiItemFlags_NoTabStop);
            const int target = response.pressed ? i : response.focused && i == *selection ? Interaction::GroupArrows(i, int(items.size())) : *selection;
            if (target != *selection) {
                *selection = target;
                changed = true;
            }
            focused |= response.focused;
            Typography::Draw(draw, font, Bezel::TitleFrame(segment, 0.0f), color(i), items[size_t(i)], TextAlignment::Center);
        }
        ImGui::PopID();
        if (focused)
            Draw::FocusRing(draw, control, radii, Theme::Colors().accent);
        return changed;
    }

    // The toolbar style (Activity Monitor @2x): an outline instead of the well, segments a point apart with separators
    // in the gaps, and the selected segment filled edge to edge, its title in the toolbar title color.
    static bool ToolbarSegmented(const char* label, int* selection, std::span<const char* const> items, float width) {
        const Metrics::SegmentedMetrics& metrics = Metrics::Segmented();
        const Font font = Font::Style(TextStyle::Body);
        const int count = int(items.size());
        const float gap = metrics.toolbarSeparator.x;
        const float segment = width > 0.0f ? (width - gap * float(count - 1)) / float(count) : Pt(Typography::WidestWidth(font, items)) + 2.0f * metrics.toolbarLabelPadding;
        const ImVec2 size(segment * float(count) + gap * float(count - 1), Metrics::Window().toolbarControlHeight);
        const ImRect control = Layout::Place(Layout::Placement{.size = Px(size), .baseline = Typography::CenteredBaseline(font, Px(size.y)) - Px(1.0f)});
        if (Layout::IsMeasuring())
            return false;

        const float position = SelectionPosition(label, *selection, count);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const float pitch = segment + gap;
        const CornerRadii radii(Px(metrics.toolbarRadius));
        {
            const Interaction::DisabledFade fade;
            // Without a selection the segments are momentary buttons (kit's Bars/Toolbar Segmented Control).
            const float fill_left = control.Min.x + Px(pitch * position);
            if (*selection >= 0)
                Draw::FillRoundedRect(draw, ImRect(fill_left, control.Min.y, fill_left + Px(segment), control.Max.y), radii, colors.tertiaryFill);
            Draw::StrokeRoundedRect(draw, control, radii, colors.tertiaryFill, Px(1.0f));
            const float top = control.GetCenter().y - Px(metrics.toolbarSeparator.y) * 0.5f;
            for (int i = 1; i < count; ++i) {
                const float x = control.Min.x + Px(pitch * float(i) - gap);
                Draw::FillRoundedRect(draw, ImRect(x, top, x + Px(gap), top + Px(metrics.toolbarSeparator.y)), CornerRadii(Px(gap) * 0.5f), colors.tertiaryFill.Opacity(SeparatorVisibility(position, i)), CornerStyle::Circular);
            }
        }

        const bool enabled = Environment().enabled;
        return SegmentTitles(label, selection, items, control, radii, pitch, segment, [&](int i) { return !enabled ? colors.toolbarSymbolDisabled : i == *selection ? colors.toolbarTitle : colors.toolbarSymbol; }, SymbolScale::Large);
    }

    // Single-select segmented control: segments of equal width in a gray well, the selected one a white pill that
    // slides to a new selection. Separators show only between two unselected segments.
    static bool Segmented(const char* label, int* selection, std::span<const char* const> items, float width) {
        const Metrics::SegmentedMetrics& metrics = Metrics::Segmented();
        const Font font = Font::Style(TextStyle::Body);
        const int count = int(items.size());
        if (count == 0)
            return false;
        if (Layout::ParentRole() == Layout::Role::Toolbar)
            return ToolbarSegmented(label, selection, items, width);
        // Without a visible label in a form it takes the offered width, a whole row directly in a section (the period
        // switch of Battery @2x).
        const bool in_form = Layout::ParentRole() == Layout::Role::FormSection;
        if (Environment().insideForm && width <= 0.0f && Interaction::VisibleLabel(label).empty())
            width = in_form ? Pt(Layout::FullProposal().x) - 2.0f * Metrics::Form().rowInset : Pt(Layout::Proposal().x);
        // Segments split the well evenly (Battery @2x: halves of the row); the kit's natural width is 4 x 73 - 1 pt for four.
        const ImVec2 size(width > 0.0f ? width : float(count) * (Pt(Typography::WidestWidth(font, items)) + 2.0f * metrics.labelPadding) - metrics.inset, metrics.height);
        const float pitch = size.x / float(count);

        ImRect control;
        if (in_form) {
            // A labeled control is centered on a 36 pt row; one taking the whole row keeps the row insets around it
            // (Output & Input of Sound @2x).
            const Metrics::FormMetrics& form = Metrics::Form();
            const std::string_view title = Interaction::VisibleLabel(label);
            const FormRowSpec spec = {.label = title, .accessorySize = size, .trailingInset = form.valueTrailing, .accessoryTop = title.empty() ? form.rowVerticalInset : (form.rowHeight - metrics.height) * 0.5f};
            const FormRowFrame row = PlaceFormRow(spec);
            if (Layout::IsMeasuring())
                return false;
            DrawFormRowLabel(row, spec);
            control = row.accessory;
        } else {
            // Labels sit a point above the middle, like button titles.
            control = Layout::Place(Layout::Placement{.size = Px(size), .baseline = Typography::CenteredBaseline(font, Px(size.y)) - Px(1.0f)});
            if (Layout::IsMeasuring())
                return false;
        }

        const float position = SelectionPosition(label, *selection, count);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const bool enabled = Environment().enabled;
        {
            const Interaction::DisabledFade fade;
            Draw::FillRoundedRect(draw, control, CornerRadii(Px(metrics.wellRadius)), colors.tertiaryFill);
            Draw::InnerShadows(draw, control, CornerRadii(Px(metrics.wellRadius)), Theme::SegmentedWellInnerShadows());
            for (int i = 1; i < count; ++i) {
                const float visibility = SeparatorVisibility(position, i);
                const float x = control.Min.x + Px(pitch * float(i) - metrics.separator.x * 0.5f);
                const float top = control.GetCenter().y - Px(metrics.separator.y) * 0.5f;
                Draw::FillRoundedRect(draw, ImRect(x, top, x + Px(metrics.separator.x), top + Px(metrics.separator.y)), CornerRadii(Px(metrics.separator.x) * 0.5f), colors.quaternaryFill.Opacity(visibility), CornerStyle::Circular);
            }
            // The pill is the selected segment inset by a point all around.
            const float pill_left = control.Min.x + Px(pitch * position + metrics.inset);
            const ImRect pill(pill_left, control.Min.y + Px(metrics.inset), pill_left + Px(pitch - 2.0f * metrics.inset), control.Max.y - Px(metrics.inset));
            Bezel::Pill(draw, pill, CornerRadii(Px(metrics.pillRadius)), false);
        }

        return SegmentTitles(label, selection, items, control, CornerRadii(Px(metrics.wellRadius)), pitch, pitch, [&](int) { return colors.LabelColor(enabled); });
    }

    // The arrows-only pop-up (kit): a 22 pt square bezel with chevron.up over chevron.down, Heavy 9 (the kit's upper
    // chevron is Black, a weight the library does not load: a tenth of a point thinner here).
    static bool ArrowsPopUp(const char* label, int* selection, std::span<const char* const> items, const PickerOptions& options) {
        const Metrics::BezelPopUpMetrics metrics = Metrics::BezelPopUp();
        const ImRect frame = Layout::Place(Layout::Placement{.size = Px(ImVec2(metrics.arrowsButton, metrics.arrowsButton)), .baseline = Typography::CenteredBaseline(Font::Style(TextStyle::Body), Px(metrics.arrowsButton))});
        if (Layout::IsMeasuring())
            return false;
        bool open = false;
        const bool changed = PopUpBehavior(ImGui::GetID(label), frame, frame, FLT_MAX, selection, items, options, &open);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        {
            const Interaction::DisabledFade fade;
            Bezel::Pill(draw, frame, CornerRadii(Px(metrics.radius)), open);
        }
        if (Interaction::FocusVisible(ImGui::GetID(label)))
            Draw::FocusRing(draw, frame, CornerRadii(Px(metrics.radius)), colors.accent);
        const float x = frame.Min.x + Px(metrics.arrowsChevronX);
        Bezel::UpDownChevrons(draw, ImRect(x, frame.Min.y, x + Px(metrics.arrowsChevrons.frame.x), frame.Max.y), colors.LabelColor(Environment().enabled), metrics.arrowsChevrons);
        return changed;
    }

    bool SegmentedToggles(const char* label, std::span<bool> on, std::span<const char* const> items, float width) {
        const Metrics::SegmentedMetrics& metrics = Metrics::Segmented();
        const Font font = Font::Style(TextStyle::Body);
        const int count = int(ImMin(on.size(), items.size()));
        if (count == 0)
            return false;
        const ImVec2 size(width > 0.0f ? width : float(count) * (Pt(Typography::WidestWidth(font, items)) + 2.0f * metrics.labelPadding), metrics.togglesHeight);
        const ImRect control = Layout::Place(Layout::Placement{.size = Px(size), .baseline = Typography::CenteredBaseline(font, Px(size.y))});
        if (Layout::IsMeasuring())
            return false;
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const bool enabled = Environment().enabled;
        const CornerRadii radii(Px(metrics.pillRadius));
        // Segment edges on the pixel grid; a segment that is on reaches the separator's column.
        const auto edge = [&](int i) { return Draw::Snap(control.Min.x + control.GetWidth() * float(i) / float(count)); };
        bool changed = false;
        bool focused = false;
        ImGui::PushID(label);
        {
            const Interaction::DisabledFade fade;
            Bezel::Pill(draw, control, radii, false);
        }
        for (int i = 0; i < count; ++i) {
            const ImRect segment(edge(i), control.Min.y, edge(i + 1), control.Max.y);
            const Interaction::Response response = Interaction::Button(ImGui::GetID(i), segment);
            focused |= response.focused;
            if (response.pressed) {
                on[size_t(i)] = !on[size_t(i)];
                changed = true;
            }
            const CornerRadii corners(i == 0 ? radii.topLeft : 0.0f, i == count - 1 ? radii.topRight : 0.0f, i == count - 1 ? radii.bottomRight : 0.0f, i == 0 ? radii.bottomLeft : 0.0f);
            if (on[size_t(i)] && enabled)
                Bezel::AccentFill(draw, segment, corners, response.held && response.hovered ? colors.accentPressed : colors.controlAccent);
            else if (response.held && response.hovered)
                Draw::FillRoundedRect(draw, segment, corners, colors.controlPressed);
            // Separators stand only between two segments that are off.
            if (i > 0 && !on[size_t(i)] && !on[size_t(i - 1)]) {
                const float top = control.GetCenter().y - Px(metrics.separator.y) * 0.5f;
                Draw::FillRoundedRect(draw, ImRect(segment.Min.x, top, segment.Min.x + Px(metrics.separator.x), top + Px(metrics.separator.y)), CornerRadii(Px(metrics.separator.x) * 0.5f), colors.quaternaryFill, CornerStyle::Circular);
            }
            const Rgba color = !enabled ? colors.tertiaryLabel : on[size_t(i)] ? Rgba::White(0.85f) : colors.label;
            Typography::Draw(draw, font, segment, color, items[size_t(i)], TextAlignment::Center);
        }
        ImGui::PopID();
        if (focused)
            Draw::FocusRing(draw, control, radii, colors.accent);
        return changed;
    }

    bool SegmentedToggles(const char* label, std::span<bool> on, std::initializer_list<const char*> items, float width) {
        return SegmentedToggles(label, on, std::span<const char* const>(items.begin(), items.size()), width);
    }

    // A pop-up in a toolbar (kit's Pop Up Button): the chosen item, as wide as the widest, and chevron.up over chevron.down
    // in the toolbar's symbol color on the item's plate; the menu puts the chosen item over the title.
    static bool ToolbarPopUp(const char* label, int* selection, std::span<const char* const> items, const PickerOptions& options) {
        const Metrics::WindowMetrics& metrics = Metrics::Window();
        const Font font = Font::Style(TextStyle::Body);
        const float title_width = Typography::WidestWidth(font, items);
        const ImRect slot = Layout::Place(ImVec2(title_width + Px(metrics.toolbarMenuTitleX + metrics.toolbarPopUpChevronGap + metrics.toolbarChevronFrame + metrics.toolbarPopUpTrailing), Px(metrics.symbolButtonSlot)));
        if (Layout::IsMeasuring())
            return false;
        const ImGuiID id = ImGui::GetID(label);
        const float x = slot.Min.x + Px(metrics.toolbarMenuTitleX);
        const bool pressed = ToolbarButton(label, slot, Environment().enabled, [&](ImDrawList* draw, Rgba color) {
            Typography::Draw(draw, font, ImRect(x, slot.Min.y, x + title_width, slot.Max.y), color, CurrentItem(*selection, items));
            const float chevron = x + title_width + Px(metrics.toolbarPopUpChevronGap);
            const float center = slot.GetCenter().y;
            Bezel::UpDownChevrons(draw, ImRect(chevron, center, chevron + Px(metrics.toolbarPopUpChevrons.frame.x), center), color, metrics.toolbarPopUpChevrons);
        });
        const float line = Px(font.lineHeight);
        if (pressed && !PopUpMenu::IsOpen(id))
            PopUpMenu::Open(id, ImRect(x, slot.GetCenter().y - line * 0.5f, x + title_width, slot.GetCenter().y + line * 0.5f), *selection, PopUpMenu::Placement::OverLabel);
        const int picked = PopUpMenu::Show(id, items, options.itemImage, options.itemImageSize);
        if (picked < 0 || picked == *selection)
            return false;
        *selection = picked;
        return true;
    }

    // The options as checkable items of the menu being collected, in a submenu or inline in their own section.
    static bool MenuPicker(const char* label, int* selection, std::span<const char* const> items, PickerStyle style) {
        const std::string title(Interaction::VisibleLabel(label));
        if (style == PickerStyle::Inline)
            MenuContent::BeginSection(title.empty() ? nullptr : title.c_str());
        else
            MenuContent::BeginSubmenu(title, 0);
        int chosen = -1;
        for (int i = 0; i < int(items.size()); ++i) {
            if (MenuContent::AddItem({.title = items[size_t(i)], .checkable = true, .checked = i == *selection}))
                chosen = i;
        }
        if (style == PickerStyle::Inline)
            MenuContent::EndSection();
        else
            MenuContent::EndSubmenu();
        if (chosen < 0 || chosen == *selection)
            return false;
        *selection = chosen;
        return true;
    }

    bool Picker(const char* label, int* selection, std::span<const char* const> items, const PickerOptions& options) {
        if (MenuContent::IsCollecting())
            return MenuPicker(label, selection, items, options.style);
        if ((options.style == PickerStyle::Automatic || options.style == PickerStyle::Menu) && Layout::ParentRole() == Layout::Role::Toolbar)
            return ToolbarPopUp(label, selection, items, options);
        if (options.style == PickerStyle::Segmented)
            return Segmented(label, selection, items, options.width);
        if (options.style == PickerStyle::Arrows)
            return ArrowsPopUp(label, selection, items, options);
        const bool in_section = Layout::ParentRole() == Layout::Role::FormSection;
        const bool radio = options.style == PickerStyle::RadioGroup || options.style == PickerStyle::Inline;
        if (radio && in_section && options.horizontal)
            return HorizontalRadioRow(label, selection, items, options);
        if (radio)
            return in_section ? RadioGroupRow(label, selection, items) : RadioGroup(label, selection, items);
        if (in_section)
            return FormPopUpRow(label, selection, items, options);
        if (Layout::ParentRole() == Layout::Role::ListRow || Environment().insideForm)
            return InlinePopUp(label, selection, items, options);
        return BezelPopUp(label, selection, items, options);
    }

    bool Picker(const char* label, int* selection, std::initializer_list<const char*> items, const PickerOptions& options) {
        return Picker(label, selection, std::span<const char* const>(items.begin(), items.size()), options);
    }
} // namespace Cupertino
