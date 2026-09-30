#include "Interaction.h"

#include "Environment.h"
#include "State.h"

#include <algorithm>

namespace Cupertino::Interaction {
    Response Button(ImGuiID id, const ImRect& rect, ImGuiButtonFlags flags, ImGuiItemFlags item_flags) {
        Response response;
        const bool enabled = Environment().enabled;
        if (!ImGui::ItemAdd(rect, id, nullptr, (enabled ? ImGuiItemFlags_None : ImGuiItemFlags_Disabled) | item_flags))
            return response;
        response.pressed = ImGui::ButtonBehavior(rect, id, &response.hovered, &response.held, flags);
        response.focused = FocusVisible(id);
        const InteractionPreview preview = Environment().interactionPreview;
        if (enabled && (preview == InteractionPreview::Hovered || preview == InteractionPreview::Pressed))
            response.hovered = true;
        if (enabled && preview == InteractionPreview::Pressed)
            response.held = true;
        if (enabled && preview == InteractionPreview::Focused)
            response.focused = true;
        return response;
    }

    void BeginOverlay(const char* name, const ImRect& rect, ImGuiWindowFlags flags) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::SetNextWindowPos(rect.Min);
        ImGui::SetNextWindowSize(rect.GetSize());
        ImGui::Begin(name, nullptr, OverlayWindowFlags | flags);
        ImGui::PopStyleVar(2);
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
    }

    void Backdrop(const char* id) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        BeginOverlay(id, ImRect(viewport->Pos, viewport->Pos + viewport->Size));
        ImGui::End();
    }

    bool PointerOver(const ImRect& rect) {
        return ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && rect.Contains(ImGui::GetIO().MousePos);
    }

    bool Enabled() {
        return Environment().enabled && (GImGui->CurrentItemFlags & ImGuiItemFlags_Disabled) == 0;
    }

    bool FocusVisible(ImGuiID id) {
        const ImGuiContext& g = *GImGui;
        return g.NavId == id && g.NavCursorVisible;
    }

    int GroupArrows(int index, int count) {
        const int step = ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_DownArrow) ? 1 : ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || ImGui::IsKeyPressed(ImGuiKey_UpArrow) ? -1 : 0;
        const int target = ImClamp(index + step, 0, count - 1);
        if (target != index)
            ImGui::SetFocusID(ImGui::GetID(target), ImGui::GetCurrentWindow());
        return target;
    }

    std::string_view VisibleLabel(const char* label) {
        return std::string_view(label, size_t(ImGui::FindRenderedTextEnd(label) - label));
    }

    struct ListFocus {
        ImGuiID list = 0;
    };

    ImGuiID& FocusedList() {
        return State::Get<ListFocus>(ImHashStr("##CupertinoListFocus")).list;
    }

    // Selects the rows from one end of a range to the other, and only them.
    static void SelectRange(std::vector<bool>& selection, int from, int to) {
        std::fill(selection.begin(), selection.end(), false);
        for (int row = ImMin(from, to); row <= ImMax(from, to); ++row)
            selection[size_t(row)] = true;
    }

    // The row the selection last moved to by keyboard; the anchor stays where a range began.
    struct ListLead {
        int row = -1;
        int movedFrame = -1;
    };

    bool ListKeys(ImGuiID id, const ImRect& rect, std::vector<bool>* selection, int& anchor) {
        ImGuiContext& g = *GImGui;
        ImGui::ItemAdd(rect, id, nullptr, Environment().enabled ? ImGuiItemFlags_None : ImGuiItemFlags_Disabled);
        // ImGui puts the navigation on a window's first item by itself; only Tab makes the list the focused one.
        if (FocusVisible(id))
            FocusedList() = id;
        // The keys go to the list while it holds the keyboard focus, or was clicked last and no other control shows it.
        const bool active = FocusedList() == id && g.NavWindow == ImGui::GetCurrentWindow() && (g.NavId == id || !g.NavCursorVisible) && !g.IO.WantTextInput;
        if (!active || !selection || selection->empty())
            return FocusVisible(id);

        const int count = int(selection->size());
        ListLead& lead = State::Get<ListLead>(id);
        const bool any = std::find(selection->begin(), selection->end(), true) != selection->end();
        const int from = lead.row >= 0 && lead.row < count && (*selection)[size_t(lead.row)] ? lead.row : anchor;
        int target = -1;
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
            target = any ? ImMin(from + 1, count - 1) : 0;
        else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
            target = any ? ImMax(from - 1, 0) : count - 1;
        else if (ImGui::IsKeyPressed(ImGuiKey_Home))
            target = 0;
        else if (ImGui::IsKeyPressed(ImGuiKey_End))
            target = count - 1;
        if ((g.IO.KeyCtrl || g.IO.KeySuper) && ImGui::IsKeyPressed(ImGuiKey_A, false))
            std::fill(selection->begin(), selection->end(), true);
        if (target >= 0) {
            if (g.IO.KeyShift && any) {
                SelectRange(*selection, anchor, target);
            } else {
                std::fill(selection->begin(), selection->end(), false);
                (*selection)[size_t(target)] = true;
                anchor = target;
            }
            lead.row = target;
            lead.movedFrame = ImGui::GetFrameCount();
        }
        return FocusVisible(id);
    }

    int ListKeyRow(ImGuiID id) {
        const ListLead& lead = State::Get<ListLead>(id);
        return lead.movedFrame == ImGui::GetFrameCount() ? lead.row : -1;
    }

    void FocusList(ImGuiID id) {
        FocusedList() = id;
        ImGui::SetFocusID(id, ImGui::GetCurrentWindow());
        GImGui->NavCursorVisible = false;
    }

    float ListFocus(ImGuiID id, bool initial) {
        const ImGuiID focused = FocusedList();
        return focused == id || (initial && focused == 0) ? Environment().KeyAmount() : 0.0f;
    }

    void ClickSelection(std::vector<bool>& selection, int& anchor, int row) {
        const ImGuiIO& io = ImGui::GetIO();
        if (io.KeyShift) {
            SelectRange(selection, anchor, row);
            return;
        }
        if (io.KeyCtrl || io.KeySuper) {
            selection[size_t(row)] = !selection[size_t(row)];
        } else {
            std::fill(selection.begin(), selection.end(), false);
            selection[size_t(row)] = true;
        }
        anchor = row;
    }

    DisabledFade::DisabledFade(float opacity) {
        if (!Environment().enabled) {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * opacity);
            pushed = true;
        }
    }

    DisabledFade::~DisabledFade() {
        if (pushed)
            ImGui::PopStyleVar();
    }
} // namespace Cupertino::Interaction
