#include "ColorPicker.h"

#include "Bezel.h"
#include "LabeledContent.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"
#include "layout/Stacks.h"
#include "overlays/Popover.h"
#include "window/ColorPanel.h"

namespace Cupertino {
    // A row of grays from white to black, then ten hues from light to dark.
    static Rgba Preset(int row, int column) {
        static const unsigned Hues[] = {0xFF3B30, 0xFF9500, 0xFFCC00, 0x34C759, 0x00C7BE, 0x30B0C7, 0x007AFF, 0x5856D6, 0xAF52DE, 0xFF2D55};
        if (row == 0) {
            const float level = 1.0f - float(column) / float(IM_ARRAYSIZE(Hues) - 1);
            return Rgba(level, level, level);
        }
        const Rgba hue = Rgba::Hex(Hues[column]);
        static const float Tints[] = {0.6f, 0.3f, 0.0f};
        static const float Shades[] = {0.25f, 0.5f};
        return row <= 3 ? Blend::Mix(hue, Rgba::White(1.0f), Tints[row - 1]) : Blend::Mix(hue, Rgba::Black(1.0f), Shades[row - 4]);
    }

    // The swatch grid in points, without the popover's padding.
    static ImVec2 GridSize() {
        const Metrics::ColorWellMetrics& metrics = Metrics::ColorWell();
        const float pitch = metrics.swatch + metrics.swatchSpacing;
        return ImVec2(float(metrics.columns) * pitch - metrics.swatchSpacing, float(metrics.rows) * pitch - metrics.swatchSpacing);
    }

    // The popover's grid: a click picks a color and closes the popover; the current color wears an accent ring.
    static bool PresetGrid(ImGuiID popover, Rgba* color) {
        const Metrics::ColorWellMetrics& metrics = Metrics::ColorWell();
        bool changed = false;
        Padding(metrics.gridPadding, [&] {
            Canvas(GridSize(), [&](ImDrawList* draw, const ImRect& rect) {
                const CornerRadii radii(Px(metrics.swatchRadius));
                for (int row = 0; row < metrics.rows; ++row) {
                    for (int column = 0; column < metrics.columns; ++column) {
                        const ImVec2 min = rect.Min + Px(ImVec2(float(column), float(row)) * (metrics.swatch + metrics.swatchSpacing));
                        const ImRect swatch(min, min + Px(ImVec2(metrics.swatch, metrics.swatch)));
                        const Rgba preset = Preset(row, column);
                        ImGui::PushID(row * metrics.columns + column);
                        if (Interaction::Button(ImGui::GetID("preset"), swatch).pressed) {
                            *color = preset;
                            changed = true;
                            PopoverPanel::Close(popover);
                        }
                        ImGui::PopID();
                        Draw::FillRoundedRect(draw, swatch, radii, preset);
                        Draw::StrokeRoundedRect(draw, swatch, radii, Rgba::Black(0.1f), Px(0.5f), StrokeAlignment::Inside);
                        if (SameColor(preset, *color))
                            Draw::StrokeRoundedRect(draw, swatch, radii, Theme::Colors().accent, Px(2.0f), StrokeAlignment::Outside);
                    }
                }
            });
        });
        return changed;
    }

    // The bezel of a push button with the color inset 4 pt; a hairline keeps light colors apart from the bezel.
    static void DrawWell(ImDrawList* draw, const ImRect& frame, Rgba color, bool pressed) {
        const Metrics::ColorWellMetrics& metrics = Metrics::ColorWell();
        const Interaction::DisabledFade fade;
        Bezel::Pill(draw, frame, CornerRadii(Px(metrics.radius)), pressed);
        const ImRect swatch(frame.Min + Px(ImVec2(metrics.inset, metrics.inset)), frame.Max - Px(ImVec2(metrics.inset, metrics.inset)));
        const CornerRadii radii(Px(metrics.swatchRadius));
        Draw::FillRoundedRect(draw, swatch, radii, color);
        Draw::StrokeRoundedRect(draw, swatch, radii, Rgba::Black(0.12f), Px(0.5f), StrokeAlignment::Inside);
    }

    // The bordered well: a gradient frame with a line inside it and the color inset 4 pt, the frame darkened while the well
    // is active. Disabled, only the line fades to half over a white one; the color stays.
    static void DrawBorderedWell(ImDrawList* draw, const ImRect& frame, Rgba color, bool active) {
        const Metrics::ColorWellMetrics& metrics = Metrics::ColorWell();
        const Palette& colors = Theme::Colors();
        const bool enabled = Environment().enabled;
        Draw::FillVerticalGradient(draw, frame, CornerRadii(0.0f), colors.colorWellTop, colors.colorWellBottom);
        if (active)
            Draw::FillRect(draw, frame, colors.controlPressed);
        if (!enabled)
            Draw::StrokeRoundedRect(draw, frame, CornerRadii(0.0f), colors.controlBackground, Px(1.0f));
        Draw::StrokeRoundedRect(draw, frame, CornerRadii(0.0f), colors.colorWellLine.Opacity(enabled ? 1.0f : 0.5f), Px(1.0f));
        const ImRect swatch(frame.Min + Px(ImVec2(metrics.borderedInset, metrics.borderedInset)), frame.Max - Px(ImVec2(metrics.borderedInset, metrics.borderedInset)));
        Draw::FillRect(draw, swatch, color);
    }

    bool ColorPicker(const char* label, Rgba* color, const ColorPickerOptions& options) {
        const Metrics::ColorWellMetrics& metrics = Metrics::ColorWell();
        const Font font = Font::Style(TextStyle::Body);
        const std::string_view text = Interaction::VisibleLabel(label);
        const bool bordered = options.style == ColorWellStyle::Bordered;
        const ImVec2 size = bordered ? metrics.borderedSize : metrics.size;
        // Accessibility > Display: in a form the well sits 8 pt from the row top with the label centered on it; rows are 42 pt.
        const Metrics::FormMetrics& form = Metrics::Form();
        const std::optional<ImRect> placed = PlaceLabeledControl({.label = text, .accessorySize = size, .trailingInset = form.valueTrailing, .accessoryTop = metrics.formTop, .labelTop = metrics.formTop + (size.y - font.lineHeight) * 0.5f}, metrics.labelSpacing);
        if (!placed)
            return false;
        const ImRect well = *placed;

        const ImGuiID id = ImGui::GetID(label);
        const Interaction::Response response = Interaction::Button(id, well, ImGuiButtonFlags_PressedOnClick);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        bool changed = false;
        if (bordered) {
            changed = SharedColorPanel::Bind(id, color, response.pressed);
            DrawBorderedWell(draw, well, *color, SharedColorPanel::IsActive(id));
        } else {
            if (response.pressed && !PopoverPanel::IsOpen(id))
                PopoverPanel::Open(id, well);
            PopoverPanel::Show(id, GridSize() + ImVec2(2.0f, 2.0f) * metrics.gridPadding, [&] { changed = PresetGrid(id, color); });
            DrawWell(draw, well, *color, PopoverPanel::IsOpen(id));
        }
        if (response.focused)
            Draw::FocusRing(draw, well, CornerRadii(bordered ? 0.0f : Px(metrics.radius)), Theme::Colors().accent);
        return changed;
    }
} // namespace Cupertino
