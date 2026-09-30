#include "Cupertino.h"

#include "core/State.h"
#include "window/ColorPanel.h"

namespace Cupertino {
    bool Initialize(const Configuration& configuration) {
        ImGuiStyle& style = ImGui::GetStyle();
        // Windows get their shadows from the kit's layer styles, not from the shadows branch defaults.
        style.WindowShadowSize = 0.0f;
        style.WindowBorderSize = 0.0f;
        style.WindowRounding = 0.0f;
        style.AntiAliasedFill = true;
        style.AntiAliasedLines = true;
        // Controls draw their own focus rings; ImGui's navigation cursor stays hidden.
        style.Colors[ImGuiCol_NavCursor] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        if (configuration.fullKeyboardAccess)
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        return Typography::LoadFonts(configuration.fontsDirectory);
    }

    void EndFrame() {
        // The arrows act on the focused control (a slider claims them for itself) and never move the focus between
        // controls, which only Tab does on macOS.
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard) {
            for (const ImGuiKey key : {ImGuiKey_LeftArrow, ImGuiKey_RightArrow, ImGuiKey_UpArrow, ImGuiKey_DownArrow})
                ImGui::SetKeyOwner(key, ImHashStr("##CupertinoArrows"));
        }
        SharedColorPanel::Present();
        Tooltip::Present();
        State::Collect();
    }
} // namespace Cupertino
