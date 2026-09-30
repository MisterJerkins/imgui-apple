#pragma once

#include "IconPlate.h"
#include "core/Environment.h"
#include "core/Typography.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <functional>
#include <optional>
#include <string_view>

namespace Cupertino {
    // The icon before a form row's text.
    enum class RowIconSize {
        // A 26 pt plate at the top of a pane (Bluetooth and Wi-Fi @2x): the text from 48 pt.
        Large,
        // A 20 pt plate centered on a 42 pt row (Control Center @2x): the text from 40 pt, centered with the control.
        Small,
    };

    // A grouped-form row: its label, an optional description wrapped under it, and the trailing accessory (points)
    // with its inset from the row's end and its top.
    struct FormRowSpec {
        std::string_view label;
        std::string_view description;
        // Subheadline 11/14 under a row's label; the header of the Bluetooth and Wi-Fi panes sets it in Callout 12/15.
        TextStyle descriptionStyle = TextStyle::Subheadline;
        // A symbol before the description in its own color, as a Label would put it (the warning of Lock Screen @2x).
        unsigned descriptionSymbol = 0;
        Rgba descriptionSymbolColor;
        // An icon before the text, large or small.
        Icon icon;
        RowIconSize iconSize = RowIconSize::Large;
        ImVec2 accessorySize;
        float trailingInset = 0.0f;
        float accessoryTop = 0.0f;
        // The label's top when it is centered on a tall accessory; the row's top inset otherwise.
        float labelTop = -1.0f;
        // Where the text starts when not where the icon puts it.
        float labelLeading = -1.0f;
        // Between the text column and the accessory when not the controls' 14 pt.
        float accessorySpacing = -1.0f;
        // Outside a form section the accessory narrows to this when the row is offered less than it asks for, as a pop-up
        // button truncating its title; 0 keeps its width.
        float accessoryMinWidth = 0.0f;
    };

    // Geometry of one grouped-form row, in pixels.
    struct FormRowFrame {
        ImRect slot;
        ImRect content;
        ImRect icon;
        ImRect label;
        // The description wrapped under the label; empty without one.
        ImRect description;
        ImRect accessory;
    };

    // Reserves a form row: the label's lines from the top inset, the description under them, both wrapped 10 pt before
    // the accessory (or accessorySpacing). The row is at least 36 pt tall and keeps 10 pt below its content.
    FormRowFrame PlaceFormRow(const FormRowSpec& spec);

    // Draws the icon, the label wrapped as placed and the description of a form row.
    void DrawFormRowLabel(const FormRowFrame& row, const FormRowSpec& spec);

    // A control with its label: a form row inside a form section, placed as spec says; elsewhere the label and then the
    // accessory, label_spacing apart and centered on each other, with the baseline a centered label has even without one.
    // Draws the label; returns the accessory's frame, or nothing while measuring.
    std::optional<ImRect> PlaceLabeledControl(const FormRowSpec& spec, float label_spacing);

    struct LabelOptions {
        const char* description = nullptr;
        Icon icon;
    };

    // A form row with a title and no control: the title over its description after a large icon (the header of a pane),
    // or after a small one without a description (a module of Control Center). Elsewhere the icon and the title side by
    // side.
    void Label(std::string_view title, const LabelOptions& options = {});

    struct LabeledContentOptions {
        // Secondary text under the label, like the second Text in a SwiftUI label; the content stays on the label's line.
        const char* description = nullptr;
    };

    // A label with a value on the trailing side (secondary text) or with custom trailing content.
    void LabeledContent(std::string_view label, std::string_view value);
    void LabeledContent(std::string_view label, const std::function<void()>& content);
    void LabeledContent(std::string_view label, const LabeledContentOptions& options, const std::function<void()>& content);

    // A form row with an image where a large icon goes and the label at its text column, 50 pt tall (a device in the
    // Bluetooth pane, @2x).
    void LabeledContent(std::string_view label, const Icon& image, const std::function<void()>& content);

    // SwiftUI's Form with .formStyle(.columns), as AppKit windows lay out settings and inspectors (TV Settings, Finder's
    // Get Info @2x): LabeledContent and labeled controls in content put their label right-aligned in the first column,
    // on the first baseline of what they label, which starts in the second; rows follow each other without spacing.
    void ColumnsForm(const ColumnsFormStyle& style, const std::function<void()>& content);

    struct NavigationLinkOptions {
        // A small icon plate before the title; a large one with a description.
        Icon icon;
        // Secondary text before the chevron: a state or a count.
        std::string_view value;
        // Secondary text under the title: wrapped in a form row, as an account's services in Internet Accounts @2x; one
        // 11 pt line in a sidebar row, which grows to 42 pt (kit's Sidebar List).
        std::string_view description;
        // SwiftUI's badge(_:) on a sidebar row: a count at the row's end, white in a red disc with increased prominence.
        int badge = 0;
        // The search text of a sidebar's results (System Settings @2x): its matches in the title keep the label color and
        // the rest turns secondary; a result without an icon stands at the titles' column, and a long one wraps.
        std::string_view highlight;
    };

    // A form row that opens a page: the icon, the title, the value and a chevron. Returns true when clicked.
    bool NavigationLink(std::string_view title, const NavigationLinkOptions& options = {});

    // A form row that opens a page, drawn by label: its content in the row's insets and a chevron at the end, as a
    // printer with its state in Printers & Scanners @2x. Returns true when clicked.
    bool NavigationLink(std::string_view id, const std::function<void()>& label);
} // namespace Cupertino
