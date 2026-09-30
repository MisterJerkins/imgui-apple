#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include <functional>

namespace Cupertino {
    enum class HorizontalAlignment {
        Leading,
        Center,
        Trailing,
    };

    enum class VerticalAlignment {
        Top,
        Center,
        Bottom,
        // Children of a horizontal stack share their first text baseline; a view without text aligns its bottom edge.
        FirstTextBaseline,
    };

    struct Alignment {
        HorizontalAlignment horizontal = HorizontalAlignment::Center;
        VerticalAlignment vertical = VerticalAlignment::Center;
    };

    // Insets in points, like SwiftUI's EdgeInsets.
    struct EdgeInsets {
        float top = 0.0f;
        float leading = 0.0f;
        float bottom = 0.0f;
        float trailing = 0.0f;

        static EdgeInsets All(float value) {
            return EdgeInsets{value, value, value, value};
        }

        static EdgeInsets Symmetric(float horizontal, float vertical) {
            return EdgeInsets{vertical, horizontal, vertical, horizontal};
        }
    };
} // namespace Cupertino

// The layout engine behind stacks, forms and lists. Every view reserves a rectangle with Place(); containers lay
// their children out along an axis. Sizes a container needs before its children are known (alignment, spacers)
// come from the previous frame, and a container seen for the first time measures its content in an extra pass.
namespace Cupertino::Layout {
    enum class Arrangement {
        Horizontal,
        Vertical,
        Overlay,
    };

    // Lets views adapt to their container, the way SwiftUI resolves control styles from context.
    enum class Role {
        None,
        FormSection,
        Sidebar,
        Toolbar,
        // A row of a bordered list: pop-ups take their borderless form look.
        ListRow,
    };

    // A container description in points.
    struct ContainerSpec {
        Arrangement arrangement = Arrangement::Vertical;
        float spacing = 0.0f;
        Alignment alignment;
        EdgeInsets padding;
        // Around every child, with a minimum slot length along the main axis (form rows).
        EdgeInsets childInsets;
        float minChildLength = 0.0f;
        // Children reach into the top child inset by their overhang (grouped form rows).
        bool allowsOverhang = false;
        // Fixed size, or the whole offered size; neither means the container hugs its content, but not below the minimum.
        float width = 0.0f;
        float height = 0.0f;
        float minWidth = 0.0f;
        float minHeight = 0.0f;
        bool fillWidth = false;
        bool fillHeight = false;
        Role role = Role::None;
        // A grid row: each child takes at least its column's width (pixels, the widest cell of the column last pass) and
        // lines up in it by columnAlignment; the row adds its cells' own widths to measuredColumns.
        const ImVector<float>* columns = nullptr;
        ImVector<float>* measuredColumns = nullptr;
        HorizontalAlignment columnAlignment = HorizontalAlignment::Leading;
    };

    // What a view asks from its container, in pixels.
    struct Placement {
        ImVec2 size;
        // Takes the free space of the container along its axis, like a Spacer.
        bool flexibleWidth = false;
        bool flexibleHeight = false;
        float minLength = 0.0f;
        // Views that draw their own row insets (form rows) get the whole slot.
        bool ignoresChildInsets = false;
        // Pixels from the top to the first text baseline; negative for a view without text.
        float baseline = -1.0f;
        // Pixels the view reaches above its text line: a grouped form row keeps its top inset above that line, so a push
        // button's bezel reaches a point into it (Privacy & Security @2x).
        float overhang = 0.0f;
        // Shapes land on whole pixels; text keeps its fractional frame and snaps its baseline when it draws, so a
        // baseline-aligned label lands where its row put it.
        bool snapsToPixels = true;
        // In a form section the separator over this view runs from edge to edge, as over a table (Sound @2x).
        bool fullWidthSeparator = false;
    };

    // The finished container handed to decorations: its rectangle, the slot of every child, which children asked for a
    // separator from edge to edge, and where the separator under each starts (negative for the section's inset).
    struct ContainerFrame {
        ImRect rect;
        ImVector<ImRect> slots;
        ImVector<bool> fullWidthSeparators;
        ImVector<float> separatorLeadings;
    };

    ImRect Place(const Placement& placement);
    ImRect Place(ImVec2 size);

    // Size offered to the next view in pixels; 0 on an axis means unconstrained.
    ImVec2 Proposal();
    // The same offer before the container's child insets, for views that draw their own row insets.
    ImVec2 FullProposal();

    // True while a new container measures its content: views report their size and draw nothing.
    bool IsMeasuring();

    // Identity of the next view from its position in the hierarchy, for views without a label. An id pushed inside the
    // container (Id, ImGui::PushID) goes into it, as it does into the containers' own.
    ImGuiID NextViewId();

    Arrangement ParentArrangement();
    Role ParentRole();
    // Position of the next view among the children of its container.
    int ChildIndex();

    // Runs content inside a container. finish runs after the content with the final geometry, before the
    // container reports its size to its parent (backgrounds, separators); it is skipped while measuring.
    void Container(const ContainerSpec& spec, const std::function<void()>& content, const std::function<void(const ContainerFrame&)>& finish = nullptr);
    // A container whose finish draws behind its content: the content goes into a draw channel over the one finish draws
    // into once the frame is known (a section's box, a selected row's platter).
    void ContainerBehind(const ContainerSpec& spec, const std::function<void()>& content, const std::function<void(const ContainerFrame&)>& behind);

    // A container at an explicit rectangle, for views that split their own space (columns, toolbars).
    void Region(ImGuiID id, const ImRect& rect, const ContainerSpec& spec, const std::function<void()>& content);
} // namespace Cupertino::Layout
