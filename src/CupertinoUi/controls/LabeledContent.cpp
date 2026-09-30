#include "LabeledContent.h"

#include "Text.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/Symbols.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"
#include "layout/Stacks.h"

namespace Cupertino {
    // Where the description's first line starts after its inline symbol, in points.
    static float DescriptionIndent(const FormRowSpec& spec) {
        if (!spec.descriptionSymbol)
            return 0.0f;
        return Pt(Typography::SymbolWidth(spec.descriptionSymbol, Font::Style(spec.descriptionStyle))) + Metrics::Form().descriptionSymbolSpacing;
    }

    FormRowFrame PlaceFormRow(const FormRowSpec& spec) {
        const Metrics::FormMetrics& metrics = Metrics::Form();
        const float top_inset = Environment().rowTopInset >= 0.0f ? Environment().rowTopInset : metrics.rowVerticalInset;
        const float bottom_inset = Environment().rowBottomInset >= 0.0f ? Environment().rowBottomInset : metrics.rowVerticalInset;
        const bool has_icon = !spec.icon.IsEmpty();
        const bool small_icon = has_icon && spec.iconSize == RowIconSize::Small;
        // A leading inset set by ListRowInsets moves the row's icon and text by as much as it differs from the form's.
        const float shift = Environment().rowLeadingInset >= 0.0f ? Environment().rowLeadingInset - metrics.rowInset : 0.0f;
        // A small icon's row is as tall as a navigation row, with its text and control centered like the icon.
        const float centering = small_icon ? (metrics.navigationRowHeight - metrics.rowHeight) * 0.5f : 0.0f;
        const float label_inset = (spec.labelTop >= 0.0f ? spec.labelTop : top_inset) + centering;
        const float icon_text_x = !has_icon ? metrics.rowInset : small_icon ? metrics.smallIconTextX : metrics.largeIconTextX;
        const float text_x = (spec.labelLeading >= 0.0f ? spec.labelLeading : icon_text_x) + shift;
        const float offered = Layout::FullProposal().x;
        const float width = offered > 0.0f ? offered : Px(spec.accessorySize.x + 200.0f);
        // Label and description wrap from the leading inset to a little before the accessory. A row without one wraps at
        // the row inset and keeps its description half a point closer, like a section header (Spotlight @2x).
        const bool accessory = spec.accessorySize.x > 0.0f || spec.trailingInset > 0.0f;
        const float spacing = spec.accessorySpacing >= 0.0f ? spec.accessorySpacing : metrics.descriptionTrailing;
        const float trailing = accessory ? spacing + spec.trailingInset + spec.accessorySize.x : metrics.rowInset;
        const float description_spacing = accessory ? metrics.descriptionSpacing : metrics.headerDescriptionSpacing;
        const float column = ImMax(width - Px(text_x + trailing), Px(1.0f));
        const float label_height = Pt(Typography::Measure(Font::Style(TextStyle::Body), spec.label, column).y);
        const float description_indent = DescriptionIndent(spec);
        const float description_height = spec.description.empty() ? 0.0f : description_spacing + Pt(Typography::Measure(Font::Style(spec.descriptionStyle), spec.description, column, Px(description_indent)).y);
        const float text_bottom = label_inset + label_height + description_height;
        const float icon_bottom = has_icon && !small_icon ? metrics.largeIconTop + metrics.largeIcon : 0.0f;
        const float min_height = small_icon ? metrics.navigationRowHeight : metrics.rowHeight;
        const float content_height = ImMax(min_height, ImMax(ImMax(spec.accessoryTop + centering + spec.accessorySize.y, text_bottom), icon_bottom) + bottom_inset);

        Layout::Placement placement;
        placement.size = ImVec2(width, Px(content_height));
        placement.ignoresChildInsets = true;
        FormRowFrame row;
        row.slot = Layout::Place(placement);
        row.content = row.slot;

        if (small_icon) {
            const float half = Px(metrics.smallIcon) * 0.5f;
            const float left = row.content.Min.x + Px(metrics.rowInset + shift);
            row.icon = ImRect(left, row.content.GetCenter().y - half, left + 2.0f * half, row.content.GetCenter().y + half);
        } else if (has_icon) {
            const ImVec2 icon_min = row.content.Min + Px(ImVec2(metrics.largeIconX + shift, metrics.largeIconTop));
            row.icon = ImRect(icon_min, icon_min + Px(ImVec2(metrics.largeIcon, metrics.largeIcon)));
        }
        const ImVec2 accessory_min(row.content.Max.x - Px(spec.trailingInset + spec.accessorySize.x), row.content.Min.y + Px(spec.accessoryTop + centering));
        row.accessory = ImRect(accessory_min, accessory_min + Px(spec.accessorySize));
        const float line_top = row.content.Min.y + Px(label_inset);
        const float left = row.content.Min.x + Px(text_x);
        row.label = ImRect(left, line_top, left + column, line_top + Px(label_height));
        if (!spec.description.empty())
            row.description = ImRect(row.label.Min.x, row.label.Max.y + Px(description_spacing), row.label.Max.x, row.content.Min.y + Px(text_bottom));
        return row;
    }

    void DrawFormRowLabel(const FormRowFrame& row, const FormRowSpec& spec) {
        const Palette& colors = Theme::Colors();
        const bool enabled = Environment().enabled;
        ImDrawList* draw = ImGui::GetWindowDrawList();
        if (!spec.icon.IsEmpty())
            DrawIcon(draw, row.icon, spec.icon);
        Typography::DrawWrapped(draw, Font::Style(TextStyle::Body), row.label, colors.LabelColor(enabled), spec.label);
        if (spec.description.empty())
            return;
        const Font font = Font::Style(spec.descriptionStyle);
        const float indent = Px(DescriptionIndent(spec));
        if (spec.descriptionSymbol) {
            const ImRect symbol(row.description.Min, ImVec2(row.description.Min.x + indent, row.description.Min.y + Px(font.lineHeight)));
            Typography::DrawSymbol(draw, spec.descriptionSymbol, font, symbol, enabled ? spec.descriptionSymbolColor : colors.tertiaryLabel, TextAlignment::Leading);
        }
        // The symbol stands inline: the lines after the first start at the column's edge (Lock Screen dark @2x).
        Typography::DrawWrapped(draw, font, row.description, enabled ? colors.secondaryLabel : colors.tertiaryLabel, spec.description, TextAlignment::Leading, indent);
    }

    // The font of a columns form's rows.
    static Font ColumnsFont() {
        const ColumnsFormStyle& style = Environment().columnsForm;
        return Font::System(style.textSize).WithLineHeight(style.lineHeight);
    }

    // A columns form's row: the label right-aligned in the first column on content's first baseline. The content lays
    // out its own labeled views as usual.
    static void ColumnsRow(std::string_view label, const std::function<void()>& content) {
        const ColumnsFormStyle style = Environment().columnsForm;
        HStack({.alignment = VerticalAlignment::FirstTextBaseline, .spacing = style.spacing}, [&] {
            Frame({.width = style.labelWidth, .alignment = {HorizontalAlignment::Trailing, VerticalAlignment::Top}}, [&] {
                Text(label, {.font = ColumnsFont(), .alignment = TextAlignment::Trailing, .wraps = true});
            });
            WithEnvironment([](EnvironmentValues& environment) { environment.columnsForm = ColumnsFormStyle(); }, content);
        });
    }

    void ColumnsForm(const ColumnsFormStyle& style, const std::function<void()>& content) {
        WithEnvironment([&](EnvironmentValues& environment) { environment.columnsForm = style; }, [&] {
            VStack({.alignment = HorizontalAlignment::Leading, .spacing = 0.0f}, content);
        });
    }

    std::optional<ImRect> PlaceLabeledControl(const FormRowSpec& spec, float label_spacing) {
        if (Layout::ParentRole() == Layout::Role::FormSection) {
            const FormRowFrame row = PlaceFormRow(spec);
            if (Layout::IsMeasuring())
                return std::nullopt;
            DrawFormRowLabel(row, spec);
            return row.accessory;
        }
        // In a columns form the label ends at its column and the control starts in the next; elsewhere the control follows
        // the label.
        const ColumnsFormStyle& columns = Environment().columnsForm;
        const bool in_columns = columns.labelWidth > 0.0f && !spec.label.empty();
        const Font font = in_columns ? ColumnsFont() : Font::Style(TextStyle::Body);
        const float lead = in_columns ? columns.labelWidth + columns.spacing : spec.label.empty() ? 0.0f : Pt(Typography::Width(font, spec.label)) + label_spacing;
        const float height = spec.label.empty() ? spec.accessorySize.y : ImMax(spec.accessorySize.y, font.lineHeight);
        const float offered = Pt(Layout::Proposal().x);
        const float width = spec.accessoryMinWidth > 0.0f && offered > 0.0f ? ImClamp(offered - lead, spec.accessoryMinWidth, spec.accessorySize.x) : spec.accessorySize.x;
        // Without a label the control still reports where a centered label's baseline would be, so a label beside it in a
        // baseline-aligned stack (LabeledContent) centers on it.
        const ImRect rect = Layout::Place(Layout::Placement{.size = Px(ImVec2(lead + width, height)), .baseline = Typography::CenteredBaseline(font, Px(height))});
        if (Layout::IsMeasuring())
            return std::nullopt;
        if (!spec.label.empty())
            Typography::Draw(ImGui::GetWindowDrawList(), font, ImRect(rect.Min, ImVec2(rect.Min.x + Px(in_columns ? columns.labelWidth : lead), rect.Max.y)), Theme::Colors().LabelColor(Environment().enabled), spec.label, in_columns ? TextAlignment::Trailing : TextAlignment::Leading);
        const float top = rect.GetCenter().y - Px(spec.accessorySize.y) * 0.5f;
        return ImRect(rect.Max.x - Px(width), top, rect.Max.x, top + Px(spec.accessorySize.y));
    }

    void Label(std::string_view title, const LabelOptions& options) {
        if (Layout::ParentRole() == Layout::Role::FormSection) {
            const FormRowSpec spec = {.label = title, .description = options.description ? options.description : "", .icon = options.icon, .iconSize = options.description ? RowIconSize::Large : RowIconSize::Small};
            const FormRowFrame row = PlaceFormRow(spec);
            if (!Layout::IsMeasuring())
                DrawFormRowLabel(row, spec);
            return;
        }
        HStack({.spacing = 6.0f}, [&] {
            if (!options.icon.IsEmpty())
                Image(options.icon, ImVec2(Metrics::Form().smallIcon, Metrics::Form().smallIcon));
            Text(title);
        });
    }

    void LabeledContent(std::string_view label, std::string_view value) {
        if (Environment().columnsForm.labelWidth > 0.0f && Layout::ParentRole() != Layout::Role::FormSection) {
            // The row's content lays out in a plain environment, so the value's look is taken here.
            const TextOptions options = {.font = ColumnsFont(), .wraps = true};
            ColumnsRow(label, [&] { Text(value, options); });
            return;
        }
        if (Layout::ParentRole() != Layout::Role::FormSection) {
            HStack([&] {
                Text(label);
                Spacer();
                Text(value, {.foreground = Foreground::Secondary});
            });
            return;
        }
        const Font font = Font::Style(TextStyle::Body);
        const float value_width = Pt(Typography::Width(font, value));
        const FormRowSpec spec = {.label = label, .accessorySize = ImVec2(value_width, font.lineHeight), .trailingInset = Metrics::Form().valueTrailing, .accessoryTop = Metrics::Form().rowVerticalInset};
        const FormRowFrame row = PlaceFormRow(spec);
        if (Layout::IsMeasuring())
            return;
        DrawFormRowLabel(row, spec);
        Typography::Draw(ImGui::GetWindowDrawList(), font, row.accessory, Theme::Colors().secondaryLabel, value, TextAlignment::Trailing);
    }

    void LabeledContent(std::string_view label, const std::function<void()>& content) {
        LabeledContent(label, LabeledContentOptions{}, content);
    }

    void LabeledContent(std::string_view label, const LabeledContentOptions& options, const std::function<void()>& content) {
        if (Environment().columnsForm.labelWidth > 0.0f && Layout::ParentRole() != Layout::Role::FormSection) {
            ColumnsRow(label, content);
            return;
        }
        HStack({.alignment = VerticalAlignment::FirstTextBaseline, .spacing = 8.0f}, [&] {
            if (options.description) {
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = Metrics::Form().descriptionSpacing}, [&] {
                    Text(label);
                    Text(options.description, {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary, .wraps = true});
                });
            } else {
                Text(label);
            }
            Spacer();
            content();
        });
    }

    void LabeledContent(std::string_view label, const Icon& image, const std::function<void()>& content) {
        const Metrics::FormMetrics& metrics = Metrics::Form();
        // The image column ends where the row inset starts the text column.
        const ImVec2 column(metrics.largeIconTextX - 2.0f * metrics.rowInset, metrics.imageRowHeight - 2.0f * metrics.rowVerticalInset);
        // The image is centered on the row; the label shares the baseline of the content's first text, like SwiftUI's
        // LabeledContent (Bluetooth @2x: Connect centered, the name on its baseline).
        HStack({.spacing = metrics.rowInset}, [&] {
            // A plate (or a plate's picture) is a form row's large icon, 26 pt a point past the row inset (Sharing @2x); a
            // free picture takes the column.
            Canvas(column, [&](ImDrawList* draw, const ImRect& rect) {
                const bool plate = image.plate != IconPlate::None || !image.image.IsEmpty();
                const float side = plate ? Px(metrics.largeIcon) : rect.GetWidth();
                const ImVec2 min(plate ? rect.Min.x + Px(metrics.largeIconX - metrics.rowInset) : rect.Min.x, rect.GetCenter().y - side * 0.5f);
                DrawIcon(draw, ImRect(min, min + ImVec2(side, side)), image);
            });
            HStack({.alignment = VerticalAlignment::FirstTextBaseline, .spacing = 8.0f}, [&] {
                Text(label);
                Spacer();
                content();
            });
        });
    }

    // A navigation row with a description: the text and a large icon placed like any form row, the value on the title's
    // line 26 pt from the row's end (Focus @2x), the chevron centered at the row's end.
    static bool DescribedNavigationLink(std::string_view title, const NavigationLinkOptions& options) {
        const Metrics::FormMetrics& metrics = Metrics::Form();
        const Font font = Font::Style(TextStyle::Body);
        const float chevron = Pt(Typography::SymbolWidth(Symbols::ChevronRight, font));
        const FormRowSpec spec = {.label = title, .description = options.description, .icon = options.icon, .accessorySize = ImVec2(chevron, font.lineHeight), .trailingInset = metrics.rowInset, .accessoryTop = metrics.rowVerticalInset};
        const FormRowFrame row = PlaceFormRow(spec);
        if (Layout::IsMeasuring())
            return false;
        const Interaction::Response response = Interaction::Button(ImGui::GetID(title.data(), title.data() + title.size()), row.slot);
        DrawFormRowLabel(row, spec);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        Typography::DrawSymbol(draw, Symbols::ChevronRight, font, ImRect(row.accessory.Min.x, row.slot.Min.y, row.accessory.Max.x, row.slot.Max.y), colors.tertiaryLabel, TextAlignment::Trailing);
        if (!options.value.empty()) {
            const ImRect value(row.label.Min.x, row.label.Min.y, row.slot.Max.x - Px(metrics.navigationValueTrailing), row.label.Min.y + Px(font.lineHeight));
            Typography::Draw(draw, font, value, colors.secondaryLabel, options.value, TextAlignment::Trailing);
        }
        return response.pressed;
    }

    bool NavigationLink(std::string_view title, const NavigationLinkOptions& options) {
        if (!options.description.empty())
            return DescribedNavigationLink(title, options);
        const Metrics::FormMetrics& metrics = Metrics::Form();
        const bool has_icon = !options.icon.IsEmpty();
        Layout::Placement placement;
        placement.size = ImVec2(Layout::FullProposal().x, Px(has_icon ? metrics.navigationRowHeight : metrics.rowHeight));
        placement.ignoresChildInsets = true;
        const ImRect row = Layout::Place(placement);
        if (Layout::IsMeasuring())
            return false;

        const Interaction::Response response = Interaction::Button(ImGui::GetID(title.data(), title.data() + title.size()), row);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const Font font = Font::Style(TextStyle::Body);
        float left = row.Min.x + Px(metrics.rowInset);
        if (has_icon) {
            const float half = Px(metrics.smallIcon) * 0.5f;
            DrawIcon(draw, ImRect(left, row.GetCenter().y - half, left + 2.0f * half, row.GetCenter().y + half), options.icon);
            left = row.Min.x + Px(metrics.smallIconTextX);
        }
        const float trailing = row.Max.x - Px(metrics.rowInset);
        Typography::DrawSymbol(draw, Symbols::ChevronRight, font, ImRect(left, row.Min.y, trailing, row.Max.y), colors.tertiaryLabel, TextAlignment::Trailing);
        float title_right = trailing;
        if (!options.value.empty()) {
            const ImRect value(left, row.Min.y, row.Max.x - Px(metrics.navigationValueTrailing), row.Max.y);
            Typography::Draw(draw, font, value, colors.secondaryLabel, options.value, TextAlignment::Trailing);
            title_right = value.Max.x - Typography::Width(font, options.value);
        }
        Typography::Draw(draw, font, ImRect(left, row.Min.y, title_right, row.Max.y), colors.label, title);
        return response.pressed;
    }

    bool NavigationLink(std::string_view id, const std::function<void()>& label) {
        const ImGuiID button = ImGui::GetID(id.data(), id.data() + id.size());
        bool pressed = false;
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Horizontal;
        spec.alignment = Alignment{HorizontalAlignment::Leading, VerticalAlignment::Center};
        spec.fillWidth = true;
        Layout::Container(spec, [&] {
            label();
            Spacer();
            Image(Symbols::ChevronRight, {.foreground = Foreground::Tertiary});
        }, [&](const Layout::ContainerFrame& frame) {
            if (!Layout::IsMeasuring())
                pressed = Interaction::Button(button, frame.rect).pressed;
        });
        return pressed;
    }
} // namespace Cupertino
