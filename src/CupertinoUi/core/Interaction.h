#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include <string_view>
#include <vector>

namespace Cupertino::Interaction {
    struct Response {
        bool hovered = false;
        bool held = false;
        bool pressed = false;
        bool focused = false;
    };

    // Registers an interactive rectangle and runs ImGui's button behavior. Views in a disabled environment
    // are registered as disabled, so they neither react nor take keyboard focus. Rows of lists pass
    // ImGuiItemFlags_NoTabStop (a list is one stop), the traffic lights ImGuiItemFlags_NoNav.
    Response Button(ImGuiID id, const ImRect& rect, ImGuiButtonFlags flags = 0, ImGuiItemFlags item_flags = 0);

    // Whether views drawn here take input: the environment is enabled and no sheet over the window disables its items.
    bool Enabled();

    // Whether the pointer is over rect (pixels) in the window being drawn or its child windows, with nothing in front.
    bool PointerOver(const ImRect& rect);

    // The windows of overlays (menus, popovers, alerts, sheets, banners): no chrome, no background, placed by their view.
    constexpr ImGuiWindowFlags OverlayWindowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollWithMouse;

    // Begins the window of an overlay over rect (pixels), without padding or border and in front of every window so
    // far; ImGui::End() ends it.
    void BeginOverlay(const char* name, const ImRect& rect, ImGuiWindowFlags flags = 0);

    // A transparent window over the whole viewport, in front of everything drawn so far, that takes the pointer so
    // nothing behind an overlay reacts while it is up.
    void Backdrop(const char* id);

    // Whether the keyboard focus ring should be drawn around the item.
    bool FocusVisible(ImGuiID id);

    // A group that is one Tab stop (segments, radio buttons) takes it on its selected member index; with the focus there
    // the arrows move the selection and the focus with it, as in AppKit. Returns the member to select.
    int GroupArrows(int index, int count);

    // The part of an ImGui label that is shown: everything before "##".
    std::string_view VisibleLabel(const char* label);

    // A click on a row of a list or table: selects it alone, Command toggles it, Shift selects the range from anchor
    // (the row clicked last).
    void ClickSelection(std::vector<bool>& selection, int& anchor, int row);

    // The list or table that took the last click: its selection is the accent one, as the first responder's in AppKit,
    // the others' gray. 0 until one is clicked.
    ImGuiID& FocusedList();

    // A list or table as one keyboard stop over its rect, registered before its rows: Tab lands on it and makes it the
    // focused list. While it is focused, ↑ and ↓ move the selection (Shift extends it from the anchor), Home and End
    // go to the ends and ⌘A selects all. Returns whether its focus ring shows.
    bool ListKeys(ImGuiID id, const ImRect& rect, std::vector<bool>* selection, int& anchor);
    // The row the keys moved the list's selection to this frame, which it scrolls into view, or -1.
    int ListKeyRow(ImGuiID id);

    // A click on a row: the list becomes the focused list and takes the keyboard focus, as a clicked table becomes the
    // first responder.
    void FocusList(ImGuiID id);

    // How much a list's selection shows the accent, 0 ... 1: as key as its window while it is the focused list (or, with
    // initial, while no list has taken the focus yet), gray otherwise.
    float ListFocus(ImGuiID id, bool initial = false);

    // Pushes the kit's disabled opacity for a disabled environment until the scope ends: 0.5, or less for layers the
    // kit fades twice.
    class DisabledFade {
    public:
        explicit DisabledFade(float opacity = 0.5f);
        ~DisabledFade();

    private:
        bool pushed = false;
    };
} // namespace Cupertino::Interaction
