#include "Sheet.h"

#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/Motion.h"
#include "core/State.h"
#include "core/Theme.h"
#include "layout/Layout.h"
#include "layout/Stacks.h"
#include "window/Window.h"

namespace Cupertino {
    struct SheetState {
        AnimatedFloat presentation;
        // Height of the content in points, known from the frame before; 0 until the sheet has been laid out once.
        float height = 0.0f;
    };

    // Battery Options @2x stands at x 123 in 715; sheets sized by their content sit 8 pt above the middle, and Spotlight and
    // Sharing sheets @2x start right under the toolbar.
    ImVec2 SheetOrigin(const ImRect& window, ImVec2 size, bool sized_by_content) {
        const float left = ImFloor((Pt(window.GetWidth()) - size.x) * 0.5f + 0.5f);
        const float lift = sized_by_content ? Metrics::Sheet().contentLift : 0.0f;
        const float top = ImMax(ToolbarHeight(), ImFloor((Pt(window.GetHeight()) - size.y) * 0.5f + 0.5f) - lift);
        return window.Min + Px(ImVec2(left, top));
    }

    void Sheet(bool* is_presented, const SheetOptions& options, const std::function<void()>& content) {
        const Metrics::SheetMetrics& metrics = Metrics::Sheet();
        const ImGuiID id = ImGui::GetID("##CupertinoSheet");
        SheetState& state = State::Get<SheetState>(id);
        // The sheet and the dim of its window fade together; a dismissed sheet draws until it has faded out.
        const float presentation = state.presentation.Update(*is_presented ? 1.0f : 0.0f, Animation::EaseOut(metrics.fade));
        if (presentation <= 0.0f)
            return;
        const SheetHost host = AttachSheet(presentation);
        if (options.height > 0.0f)
            state.height = options.height;

        // Placed with the height of the last frame.
        const ImVec2 size = Px(ImVec2(options.width, state.height));
        const ImVec2 origin = SheetOrigin(host.frame, ImVec2(options.width, state.height), options.height <= 0.0f);
        char name[32];
        ImFormatString(name, IM_ARRAYSIZE(name), "##Sheet%08X", id);

        Interaction::BeginOverlay(name, ImRect(origin, origin + ImVec2(size.x, ImMax(size.y, 1.0f))));
        // The sheet is the key window: it takes input and draws with its window's own environment, and hides the first
        // frame, which only measures.
        const auto sheet_environment = [&](EnvironmentValues& environment) {
            environment.insideSheet = true;
            environment.sheetGrows = options.height <= 0.0f;
        };
        InSheet(host, state.height > 0.0f ? host.alpha * presentation : 0.0f, sheet_environment, [&] {
            const Palette& colors = Theme::Colors();
            const CornerRadii radii(Px(Metrics::Window().radius));
            ImDrawList* draw = ImGui::GetWindowDrawList();
            // The panel is drawn under the content once its height is known.
            Layout::ContainerSpec region;
            region.alignment = Alignment{HorizontalAlignment::Leading, VerticalAlignment::Top};
            Layout::ContainerSpec panel_spec;
            panel_spec.width = options.width;
            panel_spec.height = options.height;
            Layout::Region(id, ImRect(origin, ImVec2(origin.x + size.x, host.frame.Max.y)), region, [&] {
                Layout::ContainerBehind(panel_spec, content, [&](const Layout::ContainerFrame& frame) {
                    state.height = Pt(frame.rect.GetHeight());
                    const ImRect panel(frame.rect.Min, ImVec2(frame.rect.Min.x + size.x, frame.rect.Max.y));
                    draw->PushClipRectFullScreen();
                    Draw::DropShadows(draw, panel, radii, Theme::SheetShadows());
                    Draw::StrokeRoundedRect(draw, panel, radii, colors.windowOutline, Px(0.5f), StrokeAlignment::Outside);
                    draw->PopClipRect();
                    Draw::FillRoundedRect(draw, panel, radii, colors.sheetBackground);
                });
            });
        });
        ImGui::End();
    }

    void SheetFooter(const std::function<void()>& buttons) {
        SheetFooter([] {}, buttons);
    }

    void SheetFooter(const std::function<void()>& leading, const std::function<void()>& trailing) {
        const Metrics::SheetMetrics& metrics = Metrics::Sheet();
        Layout::ContainerSpec spec;
        spec.fillWidth = true;
        Layout::Container(spec, [&] {
            const ImRect line = Layout::Place(Layout::Placement{.size = ImVec2(Layout::FullProposal().x, Px(metrics.separator))});
            if (!Layout::IsMeasuring())
                Draw::FillRect(ImGui::GetWindowDrawList(), line, Theme::Colors().separator);
            Padding(metrics.footerPadding, [&] {
                WithEnvironment([&](EnvironmentValues& environment) { environment.buttonMinWidth = metrics.footerButtonMinWidth; }, [&] {
                    HStack([&] {
                        leading();
                        Spacer();
                        trailing();
                    });
                });
            });
        });
    }
} // namespace Cupertino
