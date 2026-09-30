#pragma once

#include "core/Color.h"
#include "core/Metrics.h"
#include "core/Shape.h"
#include "core/Typography.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <functional>
#include <string_view>

// Surfaces shared by macOS controls, drawn once here instead of in every control.
namespace Cupertino::Bezel {
    // White control bezel (push and pop-up buttons, help button, stepper): two tiny shadows, a hairline edge,
    // and an instant darkening while pressed. A disabled bezel is drawn at half opacity as one layer.
    void Pill(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, bool pressed, CornerStyle style = CornerStyle::Continuous);
    // The same bezel with another fill, for surfaces that are not the control background in every appearance.
    void Pill(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, Rgba fill, bool pressed, CornerStyle style = CornerStyle::Continuous);

    // Selected checkbox, radio and pop-up indicator: accent fill with an accent glow and a soft top highlight.
    void AccentFill(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, Rgba accent, CornerStyle style = CornerStyle::Continuous);

    // Bezeled text field (square corners) or search field (rounded): an opaque fill, a 0.5 pt outline and a hard
    // one-point shade under the bottom edge.
    void Field(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, CornerStyle style = CornerStyle::Continuous);

    // The recessed capsule of a slider or progress bar: the track color with inner shadows.
    void Track(ImDrawList* draw, const ImRect& rect);

    // Unselected checkbox and radio: a white well with inner shadows and an inside hairline.
    void Well(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, CornerStyle style = CornerStyle::Continuous);

    // A checkbox's box or a radio button's circle: accented when on or mixed, a well when off; a white checkmark, dot or
    // dash, tertiary when disabled (the bezel then fades as one layer). pressed darkens it.
    enum class Mark {
        None,
        Check,
        Dot,
        Dash,
    };
    void Check(ImDrawList* draw, const ImRect& box, const CornerRadii& radii, Mark mark, bool pressed, CornerStyle style = CornerStyle::Continuous);

    // Where a push or alert button draws its title: a Body line 1 pt above the middle of the bezel, inset (points) on
    // both sides (Done and alert buttons @2x: baselines 14 of 20 and 18 of 28 pt); a line of another font raised by
    // raise points.
    ImRect TitleFrame(const ImRect& bezel, float inset);
    ImRect TitleFrame(const ImRect& bezel, float inset, const Font& font, float raise);

    // The ring around an item whose context menu is open (Mail's sidebar @2x): 2 pt of the pressed accent inside rect.
    void MenuTarget(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii);

    // A control's plain look and its accented one, cross-faded by how key the window is: the accented look only where
    // accented is true and the control enabled.
    void KeyCrossFade(bool accented, const std::function<void()>& plain, const std::function<void()>& lit);

    // chevron.up over chevron.down, centered across indicator from its top as chevrons says.
    void UpDownChevrons(ImDrawList* draw, const ImRect& indicator, Rgba color, const Metrics::ChevronPair& chevrons);

    enum class MenuIndicator {
        // chevron.up over chevron.down: a pop-up button showing its selection.
        UpDown,
        // chevron.down: a pull-down button whose menu holds actions.
        Down,
        // chevron.down in the label color without a plate (Add Photo in Wallpaper @2x).
        PlainDown,
    };

    // Where a pop-up or pull-down button in frame (its 22 pt layout frame) draws its value.
    ImRect MenuButtonLabel(const ImRect& frame);

    // The pill of a pop-up or pull-down button inside its 22 pt frame, and the focus ring around it (a combo box's too).
    ImRect MenuButtonBezel(const ImRect& frame);
    void MenuButtonFocusRing(ImDrawList* draw, const ImRect& frame);

    // The white pop-up or pull-down button: a 20 pt pill, the value and an indicator plate, accented while the window
    // is active; the focus ring when focused. A disabled button fades its pill; value and chevrons turn tertiary.
    void MenuButton(ImDrawList* draw, const ImRect& frame, std::string_view value, bool open, MenuIndicator indicator, bool focused);

    // NSComboBox (AppKit docs @2x, both appearances): the field on its own bezel and its button 2 pt from the end, the
    // accent plate with a white chevron in the key window, a bezel with a dark chevron out of it (where the kit leaves
    // the chevron bare).
    void ComboBox(ImDrawList* draw, const ImRect& frame);

    // Where a combo box in frame edits its text: 10 pt in, up to its button, and that button's plate.
    ImRect ComboBoxText(const ImRect& frame);
    ImRect ComboBoxPlate(const ImRect& frame);

    // A pull-down button titled by a symbol, like SwiftUI's Menu with an Image label.
    void SymbolMenuButton(ImDrawList* draw, const ImRect& frame, unsigned symbol, bool open, bool plain_indicator, bool focused);
} // namespace Cupertino::Bezel
