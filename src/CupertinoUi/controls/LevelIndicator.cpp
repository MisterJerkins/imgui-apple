#include "LevelIndicator.h"

#include "LabeledContent.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/Symbols.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"

#include <cmath>

namespace Cupertino {
    // The tier a value falls in: green, yellow from warning, red from critical, or the other way round when critical is
    // the lower one.
    static Rgba TierColor(float value, const LevelIndicatorOptions& options) {
        if (options.critical == options.warning)
            return Theme::SystemGreen();
        const bool high_is_bad = options.critical > options.warning;
        const bool critical = high_is_bad ? value >= options.critical : value <= options.critical;
        const bool warning = high_is_bad ? value >= options.warning : value <= options.warning;
        return critical ? Theme::SystemRed() : warning ? Theme::SystemYellow() : Theme::SystemGreen();
    }

    static int Units(const LevelIndicatorOptions& options) {
        return ImMax(1, int(std::lround(options.maximum - options.minimum)));
    }

    static ImVec2 IndicatorSize(const LevelIndicatorOptions& options) {
        const Metrics::LevelIndicatorMetrics& metrics = Metrics::LevelIndicator();
        if (options.style == LevelIndicatorStyle::Rating)
            return ImVec2(float(Units(options)) * metrics.starPitch, metrics.starHeight);
        const float width = options.width > 0.0f ? options.width : ImMax(metrics.minWidth, Pt(Layout::Proposal().x));
        return ImVec2(width, metrics.height);
    }

    // The bar, the cells or the stars in rect for value.
    static void DrawIndicator(ImDrawList* draw, const ImRect& rect, float value, const LevelIndicatorOptions& options) {
        const Metrics::LevelIndicatorMetrics& metrics = Metrics::LevelIndicator();
        const Palette& colors = Theme::Colors();
        const Interaction::DisabledFade fade;
        const float span = ImMax(options.maximum - options.minimum, 1e-6f);
        const float fraction = ImSaturate((value - options.minimum) / span);
        const CornerRadii radii(Px(metrics.radius));
        if (options.style == LevelIndicatorStyle::Rating) {
            const Font font = Font::System(metrics.starSize);
            for (int i = 0; i < Units(options); ++i) {
                const float left = rect.Min.x + Px(metrics.starPitch) * float(i);
                const bool lit = options.minimum + float(i) + 0.5f <= value;
                Typography::DrawSymbol(draw, Symbols::StarFill, font, ImRect(left, rect.Min.y, left + Px(metrics.starPitch), rect.Max.y), lit ? colors.label : colors.quaternaryLabel);
            }
            return;
        }
        if (options.style == LevelIndicatorStyle::DiscreteCapacity) {
            const int units = Units(options);
            const float gap = Px(metrics.cellGap);
            const float cell = (rect.GetWidth() - gap * float(units - 1)) / float(units);
            for (int i = 0; i < units; ++i) {
                const float left = rect.Min.x + (cell + gap) * float(i);
                const float level = options.minimum + float(i) + 1.0f;
                const bool lit = level <= value + 1e-4f;
                const Rgba color = !lit ? colors.levelIndicatorTrack : TierColor(options.tiered ? level : value, options);
                Draw::FillRoundedRect(draw, ImRect(left, rect.Min.y, left + cell, rect.Max.y), radii, color);
            }
            return;
        }
        Draw::FillRoundedRect(draw, rect, radii, colors.levelIndicatorTrack);
        if (fraction <= 0.0f)
            return;
        const float end = rect.Min.x + rect.GetWidth() * fraction;
        draw->PushClipRect(rect.Min, ImVec2(end, rect.Max.y), true);
        if (!options.tiered) {
            Draw::FillRoundedRect(draw, rect, radii, TierColor(value, options));
        } else {
            // Each tier's stretch of the bar in its color, cut at the tier limits.
            const float limits[] = {options.minimum, ImMin(options.warning, options.critical), ImMax(options.warning, options.critical), options.maximum};
            for (int t = 0; t < 3; ++t) {
                const float from = rect.Min.x + rect.GetWidth() * ImSaturate((limits[t] - options.minimum) / span);
                const float to = rect.Min.x + rect.GetWidth() * ImSaturate((limits[t + 1] - options.minimum) / span);
                if (to <= from)
                    continue;
                draw->PushClipRect(ImVec2(from, rect.Min.y), ImVec2(to, rect.Max.y), true);
                Draw::FillRoundedRect(draw, rect, radii, TierColor((limits[t] + limits[t + 1]) * 0.5f, options));
                draw->PopClipRect();
            }
        }
        draw->PopClipRect();
    }

    static std::optional<ImRect> PlaceIndicator(const char* label, const LevelIndicatorOptions& options) {
        const Metrics::FormMetrics& form = Metrics::Form();
        const ImVec2 size = IndicatorSize(options);
        return PlaceLabeledControl({.label = Interaction::VisibleLabel(label), .accessorySize = size, .trailingInset = form.valueTrailing, .accessoryTop = (form.rowHeight - size.y) * 0.5f}, Metrics::LevelIndicator().labelSpacing);
    }

    void LevelIndicator(const char* label, float value, const LevelIndicatorOptions& options) {
        const std::optional<ImRect> rect = PlaceIndicator(label, options);
        if (rect)
            DrawIndicator(ImGui::GetWindowDrawList(), *rect, value, options);
    }

    bool LevelIndicator(const char* label, float* value, const LevelIndicatorOptions& options) {
        const std::optional<ImRect> rect = PlaceIndicator(label, options);
        if (!rect)
            return false;
        const float previous = *value;
        const Interaction::Response response = Interaction::Button(ImGui::GetID(label), *rect, ImGuiButtonFlags_PressedOnClick);
        if (response.held) {
            // The unit under the pointer, rounded up: a click on the first star or cell gives one.
            const float fraction = ImSaturate((ImGui::GetIO().MousePos.x - rect->Min.x) / rect->GetWidth());
            const float units = options.maximum - options.minimum;
            *value = options.style == LevelIndicatorStyle::ContinuousCapacity ? options.minimum + units * fraction : options.minimum + std::ceil(units * fraction);
        }
        DrawIndicator(ImGui::GetWindowDrawList(), *rect, *value, options);
        return *value != previous;
    }
} // namespace Cupertino
