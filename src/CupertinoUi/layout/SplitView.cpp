#include "SplitView.h"

#include "Layout.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/State.h"
#include "core/Theme.h"

namespace Cupertino {
    struct DividerDrag {
        // Where the pointer took the divider, in points from the divider's start.
        float grab = 0.0f;
    };

    bool SplitDivider(ImGuiID id, const ImRect& line, ImGuiAxis axis, float start, float* position, float minimum, float maximum) {
        const float grip = Px(Metrics::SplitView().grip);
        const bool horizontal = axis == ImGuiAxis_X;
        const ImRect band = horizontal ? ImRect(line.Min.x - grip, line.Min.y, line.Max.x + grip, line.Max.y) : ImRect(line.Min.x, line.Min.y - grip, line.Max.x, line.Max.y + grip);
        const Interaction::Response response = Interaction::Button(id, band, ImGuiButtonFlags_PressedOnClick, ImGuiItemFlags_NoNav);
        if (response.hovered || response.held)
            ImGui::SetMouseCursor(horizontal ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS);
        DividerDrag& drag = State::Get<DividerDrag>(id);
        const float pointer = horizontal ? ImGui::GetIO().MousePos.x : ImGui::GetIO().MousePos.y;
        if (response.pressed)
            drag.grab = Pt(pointer - (horizontal ? line.Min.x : line.Min.y));
        if (!response.held)
            return false;
        *position = ImClamp(Pt(pointer - start) - drag.grab, minimum, ImMax(minimum, maximum));
        return true;
    }

    static void Split(ImGuiAxis axis, const SplitViewOptions& options, const std::function<void()>& first, const std::function<void()>& second) {
        const ImGuiID id = Layout::NextViewId();
        Layout::Placement placement;
        placement.size = Layout::Proposal();
        placement.flexibleWidth = true;
        placement.flexibleHeight = true;
        const ImRect rect = Layout::Place(placement);
        if (Layout::IsMeasuring() || !options.position)
            return;

        const bool horizontal = axis == ImGuiAxis_X;
        const float start = horizontal ? rect.Min.x : rect.Min.y;
        const float divider = Metrics::SplitView().divider;
        const float maximum = ImMin(options.maximum, Pt(horizontal ? rect.GetWidth() : rect.GetHeight()) - divider - options.secondMinimum);
        float& position = *options.position;
        position = ImClamp(position, options.minimum, ImMax(options.minimum, maximum));
        const auto line_at = [&](float at) { return horizontal ? ImRect(at, rect.Min.y, at + Px(divider), rect.Max.y) : ImRect(rect.Min.x, at, rect.Max.x, at + Px(divider)); };
        SplitDivider(ImHashStr("##Divider", 0, id), line_at(Draw::Snap(start + Px(position))), axis, start, &position, options.minimum, maximum);
        const ImRect line = line_at(Draw::Snap(start + Px(position)));
        Draw::FillRect(ImGui::GetWindowDrawList(), line, Theme::Colors().splitDivider);

        Layout::ContainerSpec pane;
        pane.arrangement = Layout::Arrangement::Vertical;
        pane.fillWidth = true;
        pane.fillHeight = true;
        const ImRect first_rect = horizontal ? ImRect(rect.Min, ImVec2(line.Min.x, rect.Max.y)) : ImRect(rect.Min, ImVec2(rect.Max.x, line.Min.y));
        const ImRect second_rect = horizontal ? ImRect(ImVec2(line.Max.x, rect.Min.y), rect.Max) : ImRect(ImVec2(rect.Min.x, line.Max.y), rect.Max);
        Layout::Region(ImHashStr("##First", 0, id), first_rect, pane, first);
        Layout::Region(ImHashStr("##Second", 0, id), second_rect, pane, second);
    }

    void HSplitView(const SplitViewOptions& options, const std::function<void()>& first, const std::function<void()>& second) {
        Split(ImGuiAxis_X, options, first, second);
    }

    void VSplitView(const SplitViewOptions& options, const std::function<void()>& first, const std::function<void()>& second) {
        Split(ImGuiAxis_Y, options, first, second);
    }
} // namespace Cupertino
