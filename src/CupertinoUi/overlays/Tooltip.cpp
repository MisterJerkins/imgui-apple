#include "Tooltip.h"

#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/Motion.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"

#include <string>

namespace Cupertino {
    // One tooltip at a time: the view the pointer rests on, when it came to rest there, and what shows.
    struct TooltipState {
        ImGuiID owner = 0;
        int ownerFrame = -1;
        double restingSince = 0.0;
        ImVec2 restingAt;
        bool shown = false;
        // A click or a key hides the tooltip until the pointer leaves its view.
        bool dismissed = false;
        std::string text;
        ImVec2 anchor;
        AnimatedFloat presence;
    };

    static TooltipState& Current() {
        static TooltipState state;
        return state;
    }

    static Font TooltipFont() {
        const Metrics::TooltipMetrics& metrics = Metrics::Tooltip();
        Font font = Font::System(metrics.textSize, FontWeight::Medium);
        font.lineHeight = metrics.lineHeight;
        return font;
    }

    static bool AnyInput() {
        const ImGuiIO& io = ImGui::GetIO();
        if (ImGui::IsAnyMouseDown() || io.MouseWheel != 0.0f || io.InputQueueCharacters.Size > 0)
            return true;
        for (int key = ImGuiKey_Keyboard_BEGIN; key < ImGuiKey_Keyboard_END; ++key) {
            if (ImGui::IsKeyPressed(ImGuiKey(key), false))
                return true;
        }
        return false;
    }

    // Follows the pointer over a view with help: the tooltip shows once the pointer has rested for the delay, or at once
    // when another one is showing.
    static void Track(ImGuiID id, const ImRect& rect, std::string_view text) {
        const Metrics::TooltipMetrics& metrics = Metrics::Tooltip();
        TooltipState& state = Current();
        const ImVec2 pointer = ImGui::GetIO().MousePos;
        if (!Interaction::PointerOver(rect))
            return;
        const double now = ImGui::GetTime();
        const bool showing = state.shown && state.ownerFrame >= ImGui::GetFrameCount() - 1;
        if (state.owner != id || state.ownerFrame < ImGui::GetFrameCount() - 1) {
            state.owner = id;
            state.dismissed = false;
            state.restingSince = now;
            state.restingAt = pointer;
            state.shown = showing;
            if (showing) {
                state.text = std::string(text);
                state.anchor = pointer;
            }
        }
        state.ownerFrame = ImGui::GetFrameCount();
        if (AnyInput()) {
            state.dismissed = true;
            state.shown = false;
        }
        if (state.shown || state.dismissed)
            return;
        if (pointer.x != state.restingAt.x || pointer.y != state.restingAt.y) {
            state.restingSince = now;
            state.restingAt = pointer;
        }
        if (now - state.restingSince >= metrics.delay) {
            state.shown = true;
            state.text = std::string(text);
            state.anchor = pointer;
        }
    }

    void Help(std::string_view text, const std::function<void()>& content) {
        const ImGuiID id = Layout::NextViewId();
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Overlay;
        Layout::Container(spec, content, [&](const Layout::ContainerFrame& frame) { Track(id, frame.rect, text); });
    }
} // namespace Cupertino

namespace Cupertino::Tooltip {
    ImVec2 Measure(std::string_view text) {
        const Metrics::TooltipMetrics& metrics = Metrics::Tooltip();
        const ImVec2 size = Typography::Measure(TooltipFont(), text, Px(metrics.maxWidth));
        return ImVec2(ImCeil(size.x) + Px(2.0f * metrics.horizontalInset), size.y + Px(metrics.top + metrics.bottom));
    }

    void Draw(ImDrawList* draw, ImVec2 origin, std::string_view text) {
        const Metrics::TooltipMetrics& metrics = Metrics::Tooltip();
        const Palette& colors = Theme::Colors();
        const ImRect panel(origin, origin + Measure(text));
        const CornerRadii radii(Px(metrics.radius));
        Draw::DropShadows(draw, panel, radii, Theme::TooltipShadows());
        Draw::StrokeRoundedRect(draw, panel, radii, colors.tooltipBorder, Px(0.5f), StrokeAlignment::Outside);
        Draw::FillRoundedRect(draw, panel, radii, colors.tooltipBackground);
        const ImRect line(panel.Min.x + Px(metrics.horizontalInset), panel.Min.y + Px(metrics.top), panel.Max.x - Px(metrics.horizontalInset), panel.Max.y - Px(metrics.bottom));
        Typography::DrawWrapped(draw, TooltipFont(), line, colors.tooltipText, text);
    }

    // Draws the tooltip over everything once all views have been laid out: it fades in while its view keeps the pointer
    // and fades out when the pointer leaves, and stays on the screen.
    void Present() {
        const Metrics::TooltipMetrics& metrics = Metrics::Tooltip();
        TooltipState& state = Current();
        const bool visible = state.shown && state.ownerFrame == ImGui::GetFrameCount();
        if (!visible && state.ownerFrame < ImGui::GetFrameCount())
            state.shown = false;
        const float presence = state.presence.Update(visible ? 1.0f : 0.0f, Animation::EaseOut(visible ? metrics.fadeIn : metrics.fadeOut));
        if (presence <= 0.0f || state.text.empty())
            return;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 size = Measure(state.text);
        ImVec2 origin = state.anchor + ImVec2(0.0f, Px(metrics.pointerOffset));
        origin.x = ImClamp(origin.x, viewport->Pos.x, viewport->Pos.x + viewport->Size.x - size.x);
        if (origin.y + size.y > viewport->Pos.y + viewport->Size.y)
            origin.y = state.anchor.y - size.y - Px(metrics.flippedGap);
        const Draw::Opacity fade(presence);
        Draw(ImGui::GetForegroundDrawList(), Draw::Snap(origin), state.text);
    }
} // namespace Cupertino::Tooltip
