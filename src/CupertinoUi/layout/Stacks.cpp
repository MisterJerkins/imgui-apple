#include "Stacks.h"

#include "core/Draw.h"
#include "core/Environment.h"
#include "core/MenuContent.h"
#include "core/State.h"
#include "core/Theme.h"

namespace Cupertino {
    void VStack(const VStackOptions& options, const std::function<void()>& content) {
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Vertical;
        spec.spacing = options.spacing;
        spec.alignment.horizontal = options.alignment;
        Layout::Container(spec, content);
    }

    void VStack(const std::function<void()>& content) {
        VStack(VStackOptions{}, content);
    }

    void HStack(const HStackOptions& options, const std::function<void()>& content) {
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Horizontal;
        spec.spacing = options.spacing;
        spec.alignment.vertical = options.alignment;
        Layout::Container(spec, content);
    }

    void HStack(const std::function<void()>& content) {
        HStack(HStackOptions{}, content);
    }

    void ZStack(const ZStackOptions& options, const std::function<void()>& content) {
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Overlay;
        spec.alignment = options.alignment;
        Layout::Container(spec, content);
    }

    void ZStack(const std::function<void()>& content) {
        ZStack(ZStackOptions{}, content);
    }

    void Spacer(float min_length) {
        const bool horizontal = Layout::ParentArrangement() == Layout::Arrangement::Horizontal;
        Layout::Placement placement;
        placement.flexibleWidth = horizontal;
        placement.flexibleHeight = !horizontal;
        placement.minLength = Px(min_length);
        Layout::Place(placement);
    }

    void Divider() {
        Divider(Theme::Colors().separator);
    }

    void Divider(Rgba color) {
        if (MenuContent::IsCollecting()) {
            MenuContent::AddSeparator();
            return;
        }
        const bool horizontal = Layout::ParentArrangement() == Layout::Arrangement::Horizontal;
        const ImVec2 proposal = Layout::Proposal();
        const float thickness = Px(1.0f);
        Layout::Placement placement;
        placement.size = horizontal ? ImVec2(thickness, proposal.y) : ImVec2(proposal.x, thickness);
        placement.flexibleWidth = !horizontal;
        placement.flexibleHeight = horizontal;
        const ImRect rect = Layout::Place(placement);
        if (Layout::IsMeasuring())
            return;
        Draw::FillRect(ImGui::GetWindowDrawList(), rect, color);
    }

    void Frame(const FrameOptions& options, const std::function<void()>& content) {
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Overlay;
        spec.alignment = options.alignment;
        spec.width = options.width;
        spec.height = options.height;
        spec.minWidth = options.minWidth;
        spec.minHeight = options.minHeight;
        spec.fillWidth = options.maxWidth == Infinity;
        spec.fillHeight = options.maxHeight == Infinity;
        Layout::Container(spec, content);
    }

    void Padding(const EdgeInsets& insets, const std::function<void()>& content) {
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Overlay;
        spec.alignment = Alignment{HorizontalAlignment::Leading, VerticalAlignment::Top};
        spec.padding = insets;
        Layout::Container(spec, content);
    }

    void Padding(float length, const std::function<void()>& content) {
        Padding(EdgeInsets::All(length), content);
    }

    void Canvas(ImVec2 size, const std::function<void(ImDrawList* draw, const ImRect& rect)>& paint, float baseline) {
        const ImRect rect = baseline >= 0.0f ? Layout::Place(Layout::Placement{.size = Px(size), .baseline = Px(baseline)}) : Layout::Place(Px(size));
        if (Layout::IsMeasuring())
            return;
        paint(ImGui::GetWindowDrawList(), rect);
    }

    void Background(const std::function<void(ImDrawList* draw, const ImRect& rect)>& paint, const std::function<void()>& content) {
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Overlay;
        Layout::ContainerBehind(spec, content, [&](const Layout::ContainerFrame& frame) { paint(ImGui::GetWindowDrawList(), frame.rect); });
    }

    void Overlay(const std::function<void(ImDrawList* draw, const ImRect& rect)>& paint, const std::function<void()>& content) {
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Overlay;
        Layout::Container(spec, content, [&](const Layout::ContainerFrame& frame) { paint(ImGui::GetWindowDrawList(), frame.rect); });
    }

    // The value an Id had when it was last drawn, in which frame, and the frame its content last became new.
    struct IdentityState {
        ImGuiID value = 0;
        int frame = -1;
        int changed = -1;
    };

    // The content is new in a frame where the value differs from the last one, or after a frame the Id was not drawn in;
    // a new container measuring the content draws it twice in that frame.
    static void Identified(ImGuiID value, const std::function<void()>& content) {
        IdentityState& state = State::Get<IdentityState>(Layout::NextViewId());
        const int frame = ImGui::GetFrameCount();
        if (state.value != value || state.frame < frame - 1)
            state.changed = frame;
        state.value = value;
        state.frame = frame;
        const bool changed = state.changed == frame;
        ImGui::PushID(int(value));
        WithEnvironment([&](EnvironmentValues& environment) { environment.identityChanged = environment.identityChanged || changed; }, content);
        ImGui::PopID();
    }

    void Id(int value, const std::function<void()>& content) {
        Identified(ImGuiID(value), content);
    }

    void Id(std::string_view value, const std::function<void()>& content) {
        Identified(ImHashStr(value.data(), value.size()), content);
    }
} // namespace Cupertino
