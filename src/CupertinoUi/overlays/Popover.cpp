#include "Popover.h"

#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/Motion.h"
#include "core/State.h"
#include "core/Theme.h"
#include "layout/Layout.h"

namespace Cupertino::PopoverPanel {
    struct PopoverState {
        bool open = false;
        bool closing = false;
        ImRect anchor;
        ArrowEdge edge = ArrowEdge::Automatic;
        AnimatedFloat opacity;
        int openedFrame = 0;
    };

    void Open(ImGuiID id, const ImRect& anchor, ArrowEdge edge) {
        PopoverState& state = State::Get<PopoverState>(id);
        state = PopoverState{};
        state.open = true;
        state.anchor = anchor;
        state.edge = edge;
        state.opacity.Snap(0.0f);
        state.openedFrame = ImGui::GetFrameCount();
    }

    void Close(ImGuiID id) {
        PopoverState& state = State::Get<PopoverState>(id);
        if (state.open && !state.closing)
            state.closing = true;
    }

    bool IsOpen(ImGuiID id) {
        const PopoverState& state = State::Get<PopoverState>(id);
        return state.open && !state.closing;
    }

    // The arrow along an edge running in direction along, its sharp tip at tip: a triangle whose corners are quadratic
    // curves through them, the tip's short, the shoulders' long enough to ease into the edge.
    static void Arrow(ImVector<ImVec2>& points, ImVec2 tip, ImVec2 along, ImVec2 out) {
        const Metrics::PopoverMetrics& metrics = Metrics::Popover();
        const ImVec2 base = tip - out * Px(metrics.arrowHeight);
        const float half_width = Px(metrics.arrowWidth) * 0.5f;
        const ImVec2 corners[] = {base - along * half_width, tip, base + along * half_width};
        const ImVec2 edges[] = {along, Draw::Normalized(tip - corners[0]), Draw::Normalized(corners[2] - tip), along};
        const float reach[] = {Px(metrics.arrowShoulder), Px(metrics.arrowTip), Px(metrics.arrowShoulder)};
        const int steps = 6;
        for (int c = 0; c < 3; ++c) {
            const ImVec2 from = corners[c] - edges[c] * reach[c];
            const ImVec2 to = corners[c] + edges[c + 1] * reach[c];
            for (int s = 0; s <= steps; ++s)
                points.push_back(ImBezierQuadraticCalc(from, corners[c], to, float(s) / float(steps)));
        }
    }

    // The panel's outline with the arrow let into its edge, clockwise from the top-left corner: each corner's points
    // are followed by the straight edge the arrow may sit on (top, trailing, bottom, leading).
    static void Outline(ImVector<ImVec2>& points, const ImRect& panel, float radius, ImVec2 tip, ArrowEdge edge) {
        const int corner = 7;
        points.resize(0);
        Draw::RoundedRectContour(points, panel, CornerRadii(radius), CornerStyle::Continuous, corner);
        int at = corner;
        ImVec2 along(1.0f, 0.0f);
        switch (edge) {
            case ArrowEdge::Trailing:
                at = 2 * corner;
                along = ImVec2(0.0f, 1.0f);
                break;
            case ArrowEdge::Bottom:
                at = 3 * corner;
                along = ImVec2(-1.0f, 0.0f);
                break;
            case ArrowEdge::Leading:
                at = 4 * corner;
                along = ImVec2(0.0f, -1.0f);
                break;
            case ArrowEdge::Top:
            case ArrowEdge::Automatic:
                break;
        }
        // Clockwise, the outside is to the left of the edge's direction.
        ImVector<ImVec2> contour;
        contour.swap(points);
        for (int i = 0; i < at; ++i)
            points.push_back(contour[i]);
        Arrow(points, tip, along, ImVec2(along.y, -along.x));
        for (int i = at; i < contour.Size; ++i)
            points.push_back(contour[i]);
    }

    void Show(ImGuiID id, ImVec2 size, const std::function<void()>& content) {
        PopoverState& state = State::Get<PopoverState>(id);
        if (!state.open)
            return;
        const Metrics::PopoverMetrics& metrics = Metrics::Popover();
        const Palette& colors = Theme::Colors();
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImGuiIO& io = ImGui::GetIO();

        // The panel beside the anchor, centered on it and kept on screen; the arrow points at the anchor's middle from the
        // straight part of its edge. Without an edge the panel goes under the anchor when it fits, above otherwise.
        const ImVec2 panel_size = Px(size);
        const float arrow = Px(metrics.arrowHeight);
        const float gap = Px(metrics.gap);
        const float margin = Px(metrics.margin);
        const float radius = Px(metrics.radius);
        const float half_base = Px(metrics.arrowWidth * 0.5f + metrics.arrowShoulder);
        const ImVec2 anchor = state.anchor.GetCenter();
        const ImVec2 screen_min = viewport->Pos + ImVec2(margin, margin);
        const ImVec2 screen_max = ImMax(screen_min, viewport->Pos + viewport->Size - ImVec2(margin, margin) - panel_size);
        ArrowEdge edge = state.edge;
        if (edge == ArrowEdge::Automatic) {
            const float room_below = viewport->Pos.y + viewport->Size.y - margin - (state.anchor.Max.y + gap + arrow);
            edge = room_below >= panel_size.y || state.anchor.Min.y - gap - arrow - panel_size.y < screen_min.y ? ArrowEdge::Top : ArrowEdge::Bottom;
        }
        const bool vertical = edge == ArrowEdge::Top || edge == ArrowEdge::Bottom;
        ImVec2 origin;
        if (vertical) {
            origin.x = ImClamp(anchor.x - panel_size.x * 0.5f, screen_min.x, screen_max.x);
            origin.y = edge == ArrowEdge::Top ? state.anchor.Max.y + gap + arrow : state.anchor.Min.y - gap - arrow - panel_size.y;
        } else {
            origin.x = edge == ArrowEdge::Leading ? state.anchor.Max.x + gap + arrow : state.anchor.Min.x - gap - arrow - panel_size.x;
            origin.y = ImClamp(anchor.y - panel_size.y * 0.5f, screen_min.y, screen_max.y);
        }
        const ImRect panel(origin, origin + panel_size);
        ImVec2 tip;
        if (vertical)
            tip = ImVec2(ImClamp(anchor.x, panel.Min.x + radius + half_base, panel.Max.x - radius - half_base), edge == ArrowEdge::Top ? panel.Min.y - arrow : panel.Max.y + arrow);
        else
            tip = ImVec2(edge == ArrowEdge::Leading ? panel.Min.x - arrow : panel.Max.x + arrow, ImClamp(anchor.y, panel.Min.y + radius + half_base, panel.Max.y - radius - half_base));

        if (!state.closing && ImGui::GetFrameCount() > state.openedFrame && (ImGui::IsKeyPressed(ImGuiKey_Escape) || (io.MouseClicked[ImGuiMouseButton_Left] && !panel.Contains(io.MousePos))))
            Close(id);
        const float opacity = state.opacity.Update(state.closing ? 0.0f : 1.0f, Animation::Linear(metrics.fade));
        if (state.closing && opacity <= 0.0f) {
            state.open = false;
            return;
        }

        Interaction::Backdrop("##CupertinoPopoverBackdrop");
        char name[32];
        ImFormatString(name, IM_ARRAYSIZE(name), "##CupertinoPopover%08X", id);
        Interaction::BeginOverlay(name, ImRect(ImMin(panel.Min, tip), ImMax(panel.Max, tip)));
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * opacity);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        draw->PushClipRectFullScreen();
        Draw::DropShadows(draw, panel, CornerRadii(radius), Theme::MenuShadows());
        ImVector<ImVec2> outline;
        Outline(outline, panel, radius, tip, edge);
        draw->AddConcavePolyFilled(outline.Data, outline.Size, colors.popoverBackground.Packed());
        draw->AddPolyline(outline.Data, outline.Size, colors.popoverBorder.Packed(), ImDrawFlags_Closed, Px(0.5f));
        draw->PopClipRect();

        Layout::ContainerSpec spec;
        spec.alignment = Alignment{HorizontalAlignment::Leading, VerticalAlignment::Top};
        Layout::Region(id, panel, spec, content);
        ImGui::PopStyleVar();
        ImGui::End();
    }
} // namespace Cupertino::PopoverPanel

namespace Cupertino {
    void Popover(bool* is_presented, const PopoverOptions& options, const std::function<void()>& anchor, const std::function<void()>& content) {
        // The anchor's container takes the view's own id; the panel lays its content out under one of its own.
        const ImGuiID id = ImHashStr("##Popover", 0, Layout::NextViewId());
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Overlay;
        Layout::Container(spec, anchor, [&](const Layout::ContainerFrame& frame) {
            if (*is_presented && !PopoverPanel::IsOpen(id))
                PopoverPanel::Open(id, frame.rect, options.arrowEdge);
            else if (!*is_presented)
                PopoverPanel::Close(id);
        });
        const bool shown = PopoverPanel::IsOpen(id);
        PopoverPanel::Show(id, options.size, content);
        if (shown && !PopoverPanel::IsOpen(id))
            *is_presented = false;
    }
} // namespace Cupertino
