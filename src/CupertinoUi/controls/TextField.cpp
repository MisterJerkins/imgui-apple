#include "TextField.h"

#include "Bezel.h"
#include "LabeledContent.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/State.h"
#include "core/Symbols.h"
#include "core/TextInput.h"
#include "core/Theme.h"
#include "layout/Layout.h"
#include "layout/Stacks.h"
#include "overlays/PopUpMenu.h"

namespace Cupertino {
    static TextInput::Style FieldStyle(TextAlignment alignment, std::string_view placeholder, bool secure) {
        const Palette& colors = Theme::Colors();
        return TextInput::Style{
            .color = colors.LabelColor(Environment().enabled),
            .alignment = alignment,
            .placeholder = placeholder,
            .secure = secure,
        };
    }

    // Where a bezeled or search field's text line has its baseline: centered on the white field.
    static float FieldBaseline() {
        const Metrics::TextFieldMetrics& metrics = Metrics::TextField();
        return Px(metrics.fieldTop) + Typography::CenteredBaseline(Font::Style(TextStyle::Body), Px(metrics.fieldHeight));
    }

    // A form row: the row label, and the text on the value's line aligned to the trailing edge like a value. At rest the
    // field has no outline (Game Center's Nickname and the Users sheet @2x; the kit outlines it at 3 %); focused, the
    // ring surrounds a 24 pt box around the text.
    static bool FormField(const char* label, std::string* text, const TextFieldOptions& options, bool secure) {
        const Metrics::TextFieldMetrics& metrics = Metrics::TextField();
        const Metrics::FormMetrics& form = Metrics::Form();
        const Font font = Font::Style(TextStyle::Body);
        const float width = options.width > 0.0f ? options.width : ImMax(metrics.minWidth, Pt(Layout::FullProposal().x) * 0.5f - form.valueTrailing);
        const FormRowSpec spec = {.label = Interaction::VisibleLabel(label), .accessorySize = ImVec2(width, font.lineHeight), .trailingInset = form.valueTrailing, .accessoryTop = form.rowVerticalInset};
        const FormRowFrame row = PlaceFormRow(spec);
        if (Layout::IsMeasuring())
            return false;
        DrawFormRowLabel(row, spec);

        const TextInput::Result result = TextInput::Edit(ImGui::GetID(label), row.accessory, text, FieldStyle(TextAlignment::Trailing, options.placeholder ? options.placeholder : "", secure));
        if (result.submitted && options.onSubmit)
            options.onSubmit();
        if (result.focused) {
            const float half = Px(metrics.formHeight) * 0.5f;
            const ImRect box(row.accessory.Min.x - Px(metrics.formTextInset), row.accessory.GetCenter().y - half, row.accessory.Max.x + Px(metrics.formTextInset), row.accessory.GetCenter().y + half);
            Draw::FocusRing(ImGui::GetWindowDrawList(), box, CornerRadii(Px(metrics.formRadius)), Theme::Colors().accent);
        }
        return result.changed;
    }

    ImRect FieldInFrame(const ImRect& frame) {
        const Metrics::TextFieldMetrics& metrics = Metrics::TextField();
        return ImRect(frame.Min.x, frame.Min.y + Px(metrics.fieldTop), frame.Max.x, frame.Min.y + Px(metrics.fieldTop + metrics.fieldHeight));
    }

    ImRect FieldClearButtonFrame(const ImRect& field) {
        const Metrics::TextFieldMetrics& metrics = Metrics::TextField();
        const float left = field.Max.x - Px(metrics.clearInset + metrics.clearButton);
        const float half = Px(metrics.clearButton) * 0.5f;
        return ImRect(left, field.GetCenter().y - half, left + 2.0f * half, field.GetCenter().y + half);
    }

    bool FieldClearButton(ImGuiID id, const ImRect& frame, Rgba color) {
        const bool clicked = Interaction::Button(id, frame).pressed;
        Typography::DrawSymbol(ImGui::GetWindowDrawList(), Symbols::XmarkCircleFill, Font::Style(TextStyle::Body), frame, color);
        return clicked;
    }

    // AppKit's bezeled field: square corners, the white field 19 pt tall in a 22 pt frame; a plain field keeps the frame
    // and its text alone.
    static bool BezeledField(const char* label, std::string* text, const TextFieldOptions& options, bool secure) {
        const Metrics::TextFieldMetrics& metrics = Metrics::TextField();
        const float width = options.width > 0.0f ? options.width : ImMax(metrics.minWidth, Pt(Layout::Proposal().x));
        const ImRect frame = Layout::Place(Layout::Placement{.size = Px(ImVec2(width, metrics.height)), .baseline = FieldBaseline()});
        if (Layout::IsMeasuring())
            return false;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const ImRect field = FieldInFrame(frame);
        const bool plain = options.style == TextFieldStyle::Plain;
        if (!plain) {
            const Interaction::DisabledFade fade;
            Bezel::Field(draw, field, CornerRadii(0.0f));
        }
        ImGui::PushID(label);
        const bool clears = options.clearButton && !text->empty() && Environment().enabled;
        const ImRect clear = FieldClearButtonFrame(field);
        const bool cleared = clears && FieldClearButton(ImGui::GetID("clear"), clear, Theme::Colors().fieldClearButton);
        if (cleared)
            text->clear();
        const float inset = plain ? 0.0f : Px(metrics.textInset);
        const float text_end = clears ? clear.Min.x - Px(metrics.clearInset) : frame.Max.x - inset;
        const ImRect area(frame.Min.x + inset, field.Min.y, text_end, field.Max.y);
        const std::string_view placeholder = options.placeholder ? std::string_view(options.placeholder) : Interaction::VisibleLabel(label);
        const TextInput::Result result = TextInput::Edit(ImGui::GetID("text"), area, text, FieldStyle(options.alignment, placeholder, secure));
        ImGui::PopID();
        if (result.submitted && options.onSubmit)
            options.onSubmit();
        if (result.focused && options.focusRing && !plain)
            Draw::FocusRing(draw, field, CornerRadii(0.0f), Theme::Colors().accent);
        return result.changed || cleared;
    }

    bool TextField(const char* label, std::string* text, const TextFieldOptions& options) {
        if (Layout::ParentRole() == Layout::Role::FormSection)
            return FormField(label, text, options, false);
        return BezeledField(label, text, options, false);
    }

    bool SecureField(const char* label, std::string* text, const TextFieldOptions& options) {
        if (Layout::ParentRole() == Layout::Role::FormSection)
            return FormField(label, text, options, true);
        return BezeledField(label, text, options, true);
    }

    // The text a search field's suggestions were closed at, so they stay away until the text changes.
    struct SuggestionsState {
        std::string dismissed;
    };

    // The white search field, or in a toolbar an outline on the bar 28 pt tall with the text a point below the middle.
    bool SearchField(const char* label, std::string* text, const SearchFieldOptions& options) {
        const Metrics::TextFieldMetrics& metrics = Metrics::TextField();
        const bool in_toolbar = Layout::ParentRole() == Layout::Role::Toolbar;
        const Font font = Font::Style(TextStyle::Body);
        const float width = options.width > 0.0f ? options.width : ImMax(metrics.minWidth, Pt(Layout::Proposal().x));
        const float height = in_toolbar ? Metrics::Window().toolbarControlHeight : metrics.height;
        const float text_offset = in_toolbar ? Px(metrics.toolbarSearchTextOffset) : 0.0f;
        const float baseline = in_toolbar ? Typography::CenteredBaseline(font, Px(height)) + text_offset : FieldBaseline();
        const ImRect frame = Layout::Place(Layout::Placement{.size = Px(ImVec2(width, height)), .baseline = baseline});
        if (Layout::IsMeasuring())
            return false;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const bool enabled = Environment().enabled;
        const ImRect field = in_toolbar ? frame : FieldInFrame(frame);
        const CornerRadii radii(Px(in_toolbar ? metrics.toolbarSearchRadius : metrics.searchRadius));
        {
            const Interaction::DisabledFade fade;
            if (in_toolbar)
                Draw::StrokeRoundedRect(draw, field, radii, colors.tertiaryFill, Px(1.0f));
            else
                Bezel::Field(draw, field, radii);
        }
        const float symbol_x = in_toolbar ? metrics.toolbarSearchSymbolX : metrics.searchSymbolX;
        const float text_x = in_toolbar ? metrics.toolbarSearchTextX : metrics.searchTextX;
        const ImRect symbol(field.Min.x + Px(symbol_x), field.Min.y, field.Min.x + Px(text_x), field.Max.y);
        // Disabled, the magnifier fades to half (kit); on a toolbar it takes the toolbar's disabled symbol color.
        const Rgba symbol_color = in_toolbar ? (enabled ? colors.toolbarSymbol : colors.toolbarSymbolDisabled) : colors.fieldSymbol.Opacity(enabled ? 1.0f : 0.5f);
        Typography::DrawSymbol(draw, Symbols::Magnifyingglass, font.Weight(FontWeight::Medium), symbol, symbol_color, TextAlignment::Leading);

        // The clear button replaces the end of the text area while there is text to clear and the field is enabled.
        ImGui::PushID(label);
        const bool clears = !text->empty() && enabled;
        const ImRect clear = FieldClearButtonFrame(field);
        const bool cleared = clears && FieldClearButton(ImGui::GetID("clear"), clear, Theme::Colors().fieldClearButton);
        if (cleared)
            text->clear();
        const float text_end = clears ? clear.Min.x - Px(metrics.clearInset) : field.Max.x - Px(metrics.textInset);
        const ImRect area(field.Min.x + Px(text_x), field.Min.y + text_offset, text_end, field.Max.y + text_offset);
        const std::string_view visible = Interaction::VisibleLabel(label);
        const std::string_view placeholder = options.placeholder ? std::string_view(options.placeholder) : visible.empty() ? std::string_view("Search") : visible;
        const TextInput::Result result = TextInput::Edit(ImGui::GetID("text"), area, text, FieldStyle(TextAlignment::Leading, placeholder, false));
        // Suggestions show while the field is edited with text they were not dismissed at; choosing one fills the field.
        const ImGuiID menu = ImGui::GetID("suggestions");
        std::string& dismissed = State::Get<SuggestionsState>(menu).dismissed;
        const bool open = PopUpMenu::IsOpen(menu);
        const bool wanted = result.focused && !text->empty() && !options.suggestions.empty() && *text != dismissed;
        if (wanted && !open)
            PopUpMenu::Open(menu, field, -1, PopUpMenu::Placement::Suggestions, FLT_MAX, Pt(field.GetWidth()));
        else if (open && (text->empty() || options.suggestions.empty()))
            PopUpMenu::Close(menu);
        bool suggested = false;
        const int chosen = PopUpMenu::Show(menu, options.suggestions);
        if (chosen >= 0 && *text != options.suggestions[size_t(chosen)]) {
            *text = options.suggestions[size_t(chosen)];
            suggested = true;
        }
        if (open && !PopUpMenu::IsOpen(menu))
            dismissed = *text;
        ImGui::PopID();
        if (result.focused)
            Draw::FocusRing(draw, field, radii, colors.accent);
        return result.changed || cleared || suggested;
    }

    bool TextEditor(const char* label, std::string* text, const TextEditorOptions& options) {
        const ImGuiID id = ImGui::GetID(label);
        bool changed = false;
        Background([](ImDrawList* draw, const ImRect& rect) { Draw::FillRect(draw, rect, Theme::Colors().fieldBackground); }, [&] {
            ScrollView({.border = options.border}, [&] {
                // The text area covers the visible height at least, so a click under the last line still edits.
                const float padding = Px(Metrics::TextField().editorPadding);
                const float width = Layout::Proposal().x;
                const float height = ImMax(TextInput::TextHeight(options.font, *text, width - 2.0f * padding), ImGui::GetCurrentWindow()->InnerRect.GetHeight());
                const ImRect rect = Layout::Place(Layout::Placement{.size = ImVec2(width, height), .flexibleWidth = true});
                if (Layout::IsMeasuring())
                    return;
                const Palette& colors = Theme::Colors();
                const TextInput::Style style = {.font = options.font, .color = colors.LabelColor(Environment().enabled), .multiline = true};
                changed = TextInput::Edit(id, ImRect(rect.Min.x + padding, rect.Min.y, rect.Max.x - padding, rect.Max.y), text, style).changed;
            });
        });
        return changed;
    }
} // namespace Cupertino
