#include "ScopeBar.h"

#include "controls/Button.h"
#include "controls/Text.h"
#include "core/Draw.h"
#include "core/Metrics.h"
#include "core/Theme.h"
#include "layout/Layout.h"
#include "layout/Stacks.h"

namespace Cupertino {
    bool ScopeBar(const char* title, int* selection, std::span<const char* const> scopes, const std::function<void()>& actions) {
        const Metrics::AccessoryBarMetrics& metrics = Metrics::AccessoryBar();
        const ImRect bar = Layout::Place(Layout::Placement{.size = ImVec2(Layout::Proposal().x, Px(metrics.height + 1.0f)), .flexibleWidth = true});
        if (Layout::IsMeasuring())
            return false;
        Draw::FillRect(ImGui::GetWindowDrawList(), ImRect(bar.Min.x, bar.Max.y - Px(1.0f), bar.Max.x, bar.Max.y), Theme::Colors().separator);
        const int previous = *selection;
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Overlay;
        spec.alignment = Alignment{HorizontalAlignment::Leading, VerticalAlignment::Center};
        ImGui::PushID(selection);
        Layout::Region(ImGui::GetID("bar"), ImRect(bar.Min.x + Px(metrics.leading), bar.Min.y, bar.Max.x - Px(metrics.trailing), bar.Max.y - Px(1.0f)), spec, [&] {
            HStack({.alignment = VerticalAlignment::Center, .spacing = metrics.spacing}, [&] {
                if (title)
                    Padding(EdgeInsets{0.0f, 0.0f, 0.0f, metrics.titleSpacing - metrics.spacing}, [&] {
                        Text(title, {.font = Font::System(metrics.fontSize, FontWeight::Bold), .foreground = Foreground::Secondary});
                    });
                for (size_t i = 0; i < scopes.size(); ++i) {
                    bool on = *selection == int(i);
                    ImGui::PushID(int(i));
                    if (AccessoryBarToggle(scopes[i], &on))
                        *selection = int(i);
                    ImGui::PopID();
                }
                if (actions) {
                    Spacer();
                    HStack({.alignment = VerticalAlignment::Center, .spacing = metrics.spacing}, actions);
                }
            });
        });
        ImGui::PopID();
        return *selection != previous;
    }

    bool ScopeBar(const char* title, int* selection, std::initializer_list<const char*> scopes, const std::function<void()>& actions) {
        return ScopeBar(title, selection, std::span<const char* const>(scopes.begin(), scopes.size()), actions);
    }
} // namespace Cupertino
