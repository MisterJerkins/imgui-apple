#include "PathControl.h"

#include "Bezel.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/MenuContent.h"
#include "core/Metrics.h"
#include "core/Symbols.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"
#include "overlays/PopUpMenu.h"

#include <string>
#include <vector>

namespace Cupertino {
    // A location's icon and, unless collapsed, its name after it.
    static void DrawComponent(ImDrawList* draw, const ImRect& frame, const PathComponent& component, bool named, Rgba color) {
        const Metrics::PathControlMetrics& metrics = Metrics::PathControl();
        const float icon = Px(metrics.icon);
        const ImVec2 icon_min(frame.Min.x, frame.GetCenter().y - icon * 0.5f);
        DrawIcon(draw, ImRect(icon_min, icon_min + ImVec2(icon, icon)), component.icon);
        if (named) {
            const float left = icon_min.x + icon + Px(metrics.iconSpacing);
            const float drop = Px(metrics.labelDrop);
            Typography::Draw(draw, Font::System(metrics.fontSize), ImRect(left, frame.Min.y + drop, frame.Max.x, frame.Max.y + drop), color, component.title);
        }
    }

    int PathControl(const char* id, std::span<const PathComponent> path, const PathControlOptions& options) {
        const Metrics::PathControlMetrics& metrics = Metrics::PathControl();
        const Font font = Font::System(metrics.fontSize);
        const float icon = Px(metrics.icon);
        const float named_width = [&] {
            float width = 0.0f;
            for (const PathComponent& component : path)
                width = ImMax(width, Typography::Width(font, component.title));
            return width;
        }();
        const float offered = Layout::Proposal().x;
        const ImRect frame = Layout::Place(Layout::Placement{.size = ImVec2(offered > 0.0f ? offered : icon + Px(metrics.iconSpacing) + named_width, Px(metrics.height)), .flexibleWidth = true, .baseline = Typography::CenteredBaseline(font, Px(metrics.height))});
        if (Layout::IsMeasuring() || path.empty())
            return -1;
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const Rgba text = colors.LabelColor(Environment().enabled);
        const ImGuiID control_id = ImGui::GetID(id);
        int chosen = -1;

        if (options.style == PathControlStyle::PopUp) {
            // The last location over the whole control, chevrons at its end; the menu lists the path from it up.
            const Interaction::Response response = Interaction::Button(control_id, frame, ImGuiButtonFlags_PressedOnClick);
            DrawComponent(draw, frame, path.back(), true, text);
            const ImVec2 indicator_min(frame.Max.x - Px(metrics.popUpTrailing + metrics.chevrons.frame.x), frame.GetCenter().y - Px(metrics.chevrons.frame.y) * 0.5f);
            Bezel::UpDownChevrons(draw, ImRect(indicator_min, indicator_min + Px(metrics.chevrons.frame)), text, metrics.chevrons);
            if (response.pressed && !PopUpMenu::IsOpen(control_id))
                PopUpMenu::Open(control_id, ImRect(frame.Min.x + icon + Px(metrics.iconSpacing), frame.Min.y, frame.Max.x, frame.Max.y), 0);
            std::vector<MenuContent::Entry> entries;
            for (int i = int(path.size()) - 1; i >= 0; --i)
                entries.push_back({.title = path[size_t(i)].title, .symbol = path[size_t(i)].icon.symbol, .image = path[size_t(i)].icon.image, .paint = path[size_t(i)].icon.paint});
            const int picked = PopUpMenu::Show(control_id, entries);
            return picked >= 0 ? int(path.size()) - 1 - picked : -1;
        }

        // Every location named while the path fits; otherwise the ones between the first and the last show their icons.
        const float separator = Px(metrics.chevronLeading + metrics.chevronWidth + metrics.chevronTrailing);
        const auto width_of = [&](size_t i, bool named) { return icon + (named ? Px(metrics.iconSpacing) + Typography::Width(font, path[i].title) : 0.0f); };
        float total = 0.0f;
        for (size_t i = 0; i < path.size(); ++i)
            total += width_of(i, true) + (i > 0 ? separator : 0.0f);
        const bool collapsed = total > frame.GetWidth();
        const Font chevron_font = Font::System(metrics.chevronSize, FontWeight::Semibold);
        float x = frame.Min.x;
        ImGui::PushID(control_id);
        for (size_t i = 0; i < path.size(); ++i) {
            if (i > 0) {
                const float chevron_left = x + Px(metrics.chevronLeading);
                const float drop = Px(metrics.chevronDrop);
                Typography::DrawSymbol(draw, Symbols::ChevronRight, chevron_font, ImRect(chevron_left, frame.Min.y + drop, chevron_left + Px(metrics.chevronWidth), frame.Max.y + drop), text);
                x += separator;
            }
            const bool named = !collapsed || i == 0 || i + 1 == path.size();
            const ImRect item(x, frame.Min.y, x + width_of(i, named), frame.Max.y);
            const Interaction::Response response = Interaction::Button(ImGui::GetID(int(i)), item);
            if (response.pressed)
                chosen = int(i);
            DrawComponent(draw, item, path[i], named, response.held && response.hovered ? colors.secondaryLabel : text);
            x = item.Max.x;
        }
        ImGui::PopID();
        return chosen;
    }

    int PathControl(const char* id, std::initializer_list<PathComponent> path, const PathControlOptions& options) {
        return PathControl(id, std::span<const PathComponent>(path.begin(), path.size()), options);
    }
} // namespace Cupertino
