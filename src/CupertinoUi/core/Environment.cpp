#include "Environment.h"

#include <vector>

namespace Cupertino {
    static std::vector<EnvironmentValues>& EnvironmentStack() {
        static std::vector<EnvironmentValues> stack(1);
        return stack;
    }

    EnvironmentValues& Environment() {
        return EnvironmentStack().back();
    }

    void WithEnvironment(const std::function<void(EnvironmentValues&)>& modify, const std::function<void()>& content) {
        std::vector<EnvironmentValues>& stack = EnvironmentStack();
        stack.push_back(stack.back());
        modify(stack.back());
        content();
        stack.pop_back();
    }

    void Disabled(bool disabled, const std::function<void()>& content) {
        WithEnvironment([&](EnvironmentValues& environment) { environment.enabled = environment.enabled && !disabled; }, content);
    }

    void WithInteractionPreview(InteractionPreview preview, const std::function<void()>& content) {
        WithEnvironment([&](EnvironmentValues& environment) { environment.interactionPreview = preview; }, content);
    }

    void WithControlActiveState(ControlActiveState state, const std::function<void()>& content) {
        WithEnvironment([&](EnvironmentValues& environment) { environment.controlActiveState = state; }, content);
    }

    void WithControlSize(ControlSize size, const std::function<void()>& content) {
        WithEnvironment([&](EnvironmentValues& environment) { environment.controlSize = size; }, content);
    }

    void WithBadgeProminence(BadgeProminence prominence, const std::function<void()>& content) {
        WithEnvironment([&](EnvironmentValues& environment) { environment.badgeProminence = prominence; }, content);
    }

    void ListRowInsets(float top, float bottom, const std::function<void()>& content) {
        ListRowInsets(top, -1.0f, bottom, content);
    }

    void ListRowInsets(float top, float leading, float bottom, const std::function<void()>& content) {
        WithEnvironment([&](EnvironmentValues& environment) {
            environment.rowTopInset = top;
            environment.rowLeadingInset = leading;
            environment.rowBottomInset = bottom;
        }, content);
    }

    float Px(float points) {
        return points * Environment().Scale();
    }

    ImVec2 Px(ImVec2 points) {
        const float scale = Environment().Scale();
        return ImVec2(points.x * scale, points.y * scale);
    }

    float Pt(float pixels) {
        return pixels / Environment().Scale();
    }
} // namespace Cupertino
