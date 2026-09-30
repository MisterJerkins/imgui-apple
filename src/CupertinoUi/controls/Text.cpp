#include "Text.h"

#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Theme.h"
#include "layout/Layout.h"

namespace Cupertino {
    Rgba ForegroundColor(Foreground foreground, Rgba custom) {
        const Palette& colors = Theme::Colors();
        const bool prominent = Environment().backgroundProminence == BackgroundProminence::Increased;
        // On an accented selection the tint turns white with the text, as a checkmark in a selected row does.
        if (prominent && foreground != Foreground::Custom)
            return foreground == Foreground::Primary || foreground == Foreground::White || foreground == Foreground::Accent ? colors.selectedContent : colors.selectedSecondaryContent;
        switch (foreground) {
            case Foreground::Secondary:
                return colors.secondaryLabel;
            case Foreground::Tertiary:
                return colors.tertiaryLabel;
            case Foreground::Quaternary:
                return colors.quaternaryLabel;
            case Foreground::Accent:
                return colors.accent;
            case Foreground::White:
                return Rgba::White(1.0f);
            case Foreground::Custom:
                return custom;
            case Foreground::Primary:
                break;
        }
        return colors.label;
    }

    void Text(std::string_view text, const TextOptions& options) {
        const float offered = Layout::Proposal().x;
        const ImVec2 natural = Typography::Measure(options.font, text, 0.0f);
        ImVec2 size = natural;
        if (options.wraps && offered > 0.0f && natural.x > offered)
            size = Typography::Measure(options.font, text, offered);
        else if (!options.wraps && offered > 0.0f)
            size.x = ImMin(size.x, offered);

        const ImRect rect = Layout::Place(Layout::Placement{.size = size, .baseline = Typography::Baseline(options.font), .snapsToPixels = false});
        if (Layout::IsMeasuring())
            return;
        ImDrawList* draw = ImGui::GetWindowDrawList();
        Rgba color = ForegroundColor(options.foreground, options.color);
        // Disabled, primary and secondary text turn tertiary like the labels and descriptions of disabled form rows.
        if (!Environment().enabled && (options.foreground == Foreground::Primary || options.foreground == Foreground::Secondary))
            color = Theme::Colors().tertiaryLabel;
        if (options.wraps && size.y > options.font.lineHeight * Environment().Scale() + 0.5f) {
            Typography::DrawWrapped(draw, options.font, rect, color, text, options.alignment);
        } else if (natural.x > rect.GetWidth() + 1.0f) {
            Typography::Draw(draw, options.font, rect, color, Typography::Truncate(options.font, text, rect.GetWidth(), options.truncation), options.alignment);
        } else {
            Typography::Draw(draw, options.font, rect, color, text, options.alignment);
        }
    }

    void Image(unsigned symbol, const ImageOptions& options) {
        const float width = Typography::SymbolWidth(symbol, options.font);
        const ImRect rect = Layout::Place(Layout::Placement{.size = ImVec2(width, options.font.lineHeight * Environment().Scale()), .baseline = Typography::Baseline(options.font)});
        if (Layout::IsMeasuring())
            return;
        Typography::DrawSymbol(ImGui::GetWindowDrawList(), symbol, options.font, rect, ForegroundColor(options.foreground, options.color));
    }

    void Image(const Bitmap& bitmap, ImVec2 size) {
        const ImRect rect = Layout::Place(Px(size.x > 0.0f && size.y > 0.0f ? size : bitmap.size));
        if (Layout::IsMeasuring())
            return;
        Draw::Image(ImGui::GetWindowDrawList(), rect, bitmap);
    }

    void Image(const Icon& icon, ImVec2 size) {
        const ImRect rect = Layout::Place(Px(size));
        if (Layout::IsMeasuring())
            return;
        DrawIcon(ImGui::GetWindowDrawList(), rect, icon);
    }
} // namespace Cupertino
