#pragma once

#include "controls/IconPlate.h"
#include "controls/LabeledContent.h"
#include "core/Bitmap.h"

#include <functional>
#include <string>

namespace Cupertino {
    // Sidebar list: scrolls, 28 pt rows, groups made with Section. It has the keyboard focus until another list or table
    // takes a click, and again after a click on one of its rows; without it its selection turns gray (Photos @2x).
    void List(const std::function<void()>& content);

    // A sidebar row bound to a selection: the icon and the title, with a description under it in a 42 pt row, and the
    // badge's count at the row's end. The selected row lies on the accent platter with white content while its list has
    // the focus in the key window, on a gray one otherwise. Returns true when clicked.
    bool NavigationLink(const char* title, int* selection, int tag, const NavigationLinkOptions& options = {});

    // A sidebar row with its own content, as tall as the content (the network at the top of the Wi-Fi details sheet):
    // selected, it lies on the platter and its text turns white while emphasized.
    bool NavigationLink(int* selection, int tag, const std::function<void()>& label);

    // DisclosureGroup in a sidebar list (Photos @2x): the group's row with a chevron in the list's chevron column, then
    // content a level deeper while expanded. The row is either the title alone, which a click turns, or label, usually a
    // NavigationLink, whose chevron alone turns the group.
    void SidebarDisclosureGroup(const char* title, bool* expanded, const std::function<void()>& content);
    void SidebarDisclosureGroup(bool* expanded, const std::function<void()>& label, const std::function<void()>& content);

    // The rounded search field at the top of a System Settings sidebar, pinned above the list.
    bool SidebarSearchField(std::string* text);

    struct SidebarAccountOptions {
        // Shown on a blue disc when the account has no picture.
        const char* initials = "";
        // The account's picture, cut to the avatar's circle.
        Bitmap picture;
        FontWeight nameWeight = FontWeight::Semibold;
        // The account's pane is open: the row lies on the selection platter.
        bool selected = false;
    };

    // The account row at the top of the list: the avatar, the name and a secondary line. Returns true when clicked.
    bool SidebarAccount(const char* name, const char* subtitle, const SidebarAccountOptions& options = {});
} // namespace Cupertino
