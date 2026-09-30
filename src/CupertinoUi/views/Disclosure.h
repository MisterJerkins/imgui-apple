#pragma once

#include "core/Color.h"
#include "core/Typography.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <functional>

namespace Cupertino {
    // A collapsible group, like SwiftUI's DisclosureGroup: the label behind a chevron that turns down while the content
    // shows. In a form the label is a row and the content adds rows to the same section; in a sidebar list the group is an
    // outline row with its content a level deeper (SidebarDisclosureGroup).
    void DisclosureGroup(const char* label, bool* expanded, const std::function<void()>& content);
    // The same group with its label drawn by label, as DisclosureGroup(isExpanded:content:label:); in a sidebar list the
    // label is usually the group's NavigationLink.
    void DisclosureGroup(bool* expanded, const std::function<void()>& label, const std::function<void()>& content);

    // AppKit's push disclosure button (the Save dialog's): a 22 pt square bezel with chevron.down while collapsed and
    // chevron.up while expanded. The label is only its id. Returns true when clicked.
    bool DisclosureButton(const char* label, bool* expanded);

    // The turn of a disclosure chevron, 0 closed to 1 open, easing over 0.2 s as expanded changes.
    float DisclosureTurn(ImGuiID id, bool expanded);

    // The chevron of an outline row (sidebars and tables): chevron.right in font centered on center, turned down about
    // its middle by turn.
    void DrawOutlineChevron(ImDrawList* draw, ImVec2 center, const Font& font, float turn, Rgba color);
} // namespace Cupertino
