#include "Bezel.h"

#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/Symbols.h"
#include "core/Theme.h"
#include "core/Typography.h"

namespace Cupertino::Bezel {
    void Pill(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, bool pressed, CornerStyle style) {
        Pill(draw, rect, radii, Theme::Colors().controlBackground, pressed, style);
    }

    void Pill(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, Rgba fill, bool pressed, CornerStyle style) {
        const Palette& colors = Theme::Colors();
        Draw::DropShadows(draw, rect, radii, Theme::BezelShadows(), style, 1.0f, true);
        Draw::FillRoundedRect(draw, rect, radii, fill, style);
        Draw::InnerShadows(draw, rect, radii, Theme::BezelHighlights(false), style);
        Draw::StrokeRoundedRect(draw, rect, radii, colors.controlBorder, Px(0.5f), StrokeAlignment::Outside, style);
        if (pressed)
            Draw::FillRoundedRect(draw, rect, radii, colors.controlPressed, style);
    }

    void AccentFill(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, Rgba accent, CornerStyle style) {
        const std::array<Shadow, 2> shadows = Theme::ControlShadows(accent);
        Draw::DropShadows(draw, rect, radii, shadows, style);
        Draw::FillRoundedRect(draw, rect, radii, accent, style);
        // The kit's highlight: white fading to clear down the control at 17% opacity (lighten; normal blending gives
        // the same result on a saturated accent). Dark controls darken by 8% toward the bottom instead (default button
        // on the Wi-Fi sheet @2x) under a lighter top edge.
        if (Environment().IsDark()) {
            Draw::FillVerticalGradient(draw, rect, radii, Rgba::Black(0.0f), Rgba::Black(0.08f), style);
            Draw::InnerShadows(draw, rect, radii, Theme::BezelHighlights(true), style);
        } else {
            Draw::FillVerticalGradient(draw, rect, radii, Rgba::White(0.17f), Rgba::White(0.0f), style);
        }
    }

    void Field(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, CornerStyle style) {
        const Palette& colors = Theme::Colors();
        Draw::DropShadows(draw, rect, radii, Theme::FieldShadows(), style);
        Draw::FillRoundedRect(draw, rect, radii, colors.fieldBackground, style);
        Draw::StrokeRoundedRect(draw, rect, radii, colors.fieldBorder, Px(0.5f), StrokeAlignment::Outside, style);
    }

    void Well(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, CornerStyle style) {
        // A dark well lightens toward the bottom in the bezel's shadow under a lighter top edge (Spotlight @2x).
        if (Environment().IsDark()) {
            const std::array<Shadow, 2> shadows = Theme::ControlShadows(Rgba());
            Draw::DropShadows(draw, rect, radii, shadows, style, 1.0f, true);
            Draw::FillVerticalGradient(draw, rect, radii, Rgba::White(0.15f), Rgba::White(0.29f), style);
            Draw::InnerShadows(draw, rect, radii, Theme::BezelHighlights(false), style);
            return;
        }
        Draw::FillRoundedRect(draw, rect, radii, Theme::Colors().controlBackground, style);
        Draw::InnerShadows(draw, rect, radii, Theme::WellInnerShadows(), style);
        Draw::StrokeRoundedRect(draw, rect, radii, Rgba::Black(0.15f), Px(0.5f), StrokeAlignment::Inside, style);
    }

    ImRect TitleFrame(const ImRect& bezel, float inset) {
        return TitleFrame(bezel, inset, Font::Style(TextStyle::Body), 1.0f);
    }

    ImRect TitleFrame(const ImRect& bezel, float inset, const Font& font, float raise) {
        const float middle = bezel.GetCenter().y - Px(raise);
        const float half = Px(font.lineHeight) * 0.5f;
        return ImRect(bezel.Min.x + Px(inset), middle - half, bezel.Max.x - Px(inset), middle + half);
    }

    void Check(ImDrawList* draw, const ImRect& box, const CornerRadii& radii, Mark mark, bool pressed, CornerStyle style) {
        const Palette& colors = Theme::Colors();
        const bool enabled = Environment().enabled;
        {
            const Interaction::DisabledFade fade;
            if (mark != Mark::None && enabled)
                AccentFill(draw, box, radii, colors.controlAccent, style);
            else if (mark != Mark::None)
                Pill(draw, box, radii, false, style);
            else
                Well(draw, box, radii, style);
            if (pressed)
                Draw::FillRoundedRect(draw, box, radii, colors.controlPressed, style);
        }
        // The mark lets 15% of a dark control's accent through (Spotlight @2x).
        const Rgba color = !enabled ? colors.tertiaryLabel : Environment().IsDark() ? Rgba::White(0.85f) : Rgba::White(1.0f);
        const float unit = box.GetWidth() / 14.0f;
        const float points = Pt(box.GetWidth());
        if (mark == Mark::Check || mark == Mark::Dash) {
            // A heavy checkmark or minus centered in a 12 x 12 frame offset by (1, 1) inside the 14 pt box (AirPods @2x; the
            // kit has the checkmark bold and half a point higher).
            const ImRect frame(box.Min + ImVec2(1.0f, 1.0f) * unit, box.Min + ImVec2(13.0f, 13.0f) * unit);
            Typography::DrawSymbol(draw, mark == Mark::Check ? Symbols::Checkmark : Symbols::Minus, Font::System(10.0f * points / 14.0f, FontWeight::Heavy), frame, color);
        } else if (mark == Mark::Dot) {
            // circle.fill Heavy 6 in an 8 x 7 frame, half a point above the center as in the kit.
            const ImVec2 center = box.GetCenter();
            const ImRect dot(center - ImVec2(4.0f, 4.0f) * unit, center + ImVec2(4.0f, 3.0f) * unit);
            Typography::DrawSymbol(draw, Symbols::CircleFill, Font::System(6.0f * points / 14.0f, FontWeight::Heavy), dot, color);
        }
    }

    ImRect MenuButtonLabel(const ImRect& frame) {
        const Metrics::BezelPopUpMetrics metrics = Metrics::BezelPopUp();
        const float line = Font::System(metrics.labelSize).lineHeight;
        const float top = frame.Min.y + Px((metrics.height - line) * 0.5f);
        return ImRect(frame.Min.x + Px(metrics.labelInset), top, frame.Max.x - Px(metrics.gap + metrics.indicator + metrics.trailingInset), top + Px(line));
    }

    // The white part is 20 pt (measured on @2x screenshots); the kit's 22 pt frame includes the edge and the shadow.
    ImRect MenuButtonBezel(const ImRect& frame) {
        const float inset = Px(Metrics::BezelPopUp().bezelInset);
        return ImRect(frame.Min.x, frame.Min.y + inset, frame.Max.x, frame.Max.y - inset);
    }

    static void MenuPill(ImDrawList* draw, const ImRect& frame, bool open) {
        const Interaction::DisabledFade fade;
        Pill(draw, MenuButtonBezel(frame), CornerRadii(Px(Metrics::BezelPopUp().radius)), open);
    }

    static void MenuIndicatorPlate(ImDrawList* draw, const ImRect& plate, MenuIndicator indicator) {
        const Metrics::BezelPopUpMetrics metrics = Metrics::BezelPopUp();
        const Palette& colors = Theme::Colors();
        const bool enabled = Environment().enabled;
        const auto chevrons = [&](Rgba color) {
            if (indicator == MenuIndicator::UpDown) {
                UpDownChevrons(draw, plate, color, metrics.chevrons);
                return;
            }
            const ImVec2 min = plate.Min + Px(metrics.downChevron);
            const Font font = Font::System(metrics.chevrons.size, FontWeight::Heavy).WithLineHeight(metrics.downChevronFrame.y);
            Typography::DrawSymbol(draw, Symbols::ChevronDown, font, ImRect(min, min + Px(metrics.downChevronFrame)), color);
        };
        // In the key window the chevrons stand white on an accent plate; they cross-fade to plain ones as it stops being
        // key.
        KeyCrossFade(true, [&] { chevrons(colors.LabelColor(enabled)); }, [&] {
            AccentFill(draw, plate, CornerRadii(Px(metrics.indicatorRadius)), colors.controlAccent);
            chevrons(Rgba::White(1.0f));
        });
    }

    // chevron.down in the label color at the end of a pull-down without a plate.
    static void PlainChevron(ImDrawList* draw, const ImRect& frame, Rgba color) {
        const Metrics::BezelPopUpMetrics metrics = Metrics::BezelPopUp();
        const ImVec2 min(frame.Max.x - Px(metrics.plainTrailingInset + metrics.plainChevronFrame.x), frame.Min.y + Px(metrics.plainChevronTop));
        const Font font = Font::System(metrics.plainChevronSize, FontWeight::Heavy).WithLineHeight(metrics.plainChevronFrame.y);
        Typography::DrawSymbol(draw, Symbols::ChevronDown, font, ImRect(min, min + Px(metrics.plainChevronFrame)), color);
    }

    void MenuButtonFocusRing(ImDrawList* draw, const ImRect& frame) {
        Draw::FocusRing(draw, MenuButtonBezel(frame), CornerRadii(Px(Metrics::BezelPopUp().radius)), Theme::Colors().accent);
    }

    void MenuButton(ImDrawList* draw, const ImRect& frame, std::string_view value, bool open, MenuIndicator indicator, bool focused) {
        const Metrics::BezelPopUpMetrics metrics = Metrics::BezelPopUp();
        const Palette& colors = Theme::Colors();
        MenuPill(draw, frame, open);
        const Rgba color = colors.LabelColor(Environment().enabled);
        // A title longer than the button gives way to an ellipsis at its end; the pixel the frame may lose to snapping
        // does not count.
        const Font font = Font::System(metrics.labelSize);
        const auto title = [&](const ImRect& line) { Typography::Draw(draw, font, line, color, Typography::Truncate(font, value, line.GetWidth() + 1.0f)); };
        if (indicator == MenuIndicator::PlainDown) {
            const ImRect label = MenuButtonLabel(frame);
            title(ImRect(frame.Min.x + Px(metrics.plainInset), label.Min.y, frame.Max.x - Px(metrics.plainTrailingInset + metrics.plainChevronFrame.x), label.Max.y));
            PlainChevron(draw, frame, color);
        } else {
            title(MenuButtonLabel(frame));
            const float left = frame.Max.x - Px(metrics.trailingInset + metrics.indicator);
            const float top = frame.Min.y + Px(indicator == MenuIndicator::Down ? metrics.pullDownIndicatorTop : metrics.indicatorTop);
            MenuIndicatorPlate(draw, ImRect(left, top, left + Px(metrics.indicator), top + Px(metrics.indicator)), indicator);
        }
        if (focused)
            MenuButtonFocusRing(draw, frame);
    }

    ImRect ComboBoxPlate(const ImRect& frame) {
        const Metrics::BezelPopUpMetrics metrics = Metrics::BezelPopUp();
        const float left = frame.Max.x - Px(metrics.comboTrailingInset + metrics.indicator);
        const float top = frame.Min.y + Px(metrics.comboIndicatorTop);
        return ImRect(left, top, left + Px(metrics.indicator), top + Px(metrics.indicator));
    }

    ImRect ComboBoxText(const ImRect& frame) {
        const Metrics::BezelPopUpMetrics metrics = Metrics::BezelPopUp();
        const ImRect label = MenuButtonLabel(frame);
        return ImRect(frame.Min.x + Px(metrics.comboTextInset), label.Min.y, ComboBoxPlate(frame).Min.x - Px(metrics.gap * 0.5f), label.Max.y);
    }

    void ComboBox(ImDrawList* draw, const ImRect& frame) {
        const Metrics::BezelPopUpMetrics metrics = Metrics::BezelPopUp();
        const Palette& colors = Theme::Colors();
        {
            const Interaction::DisabledFade fade;
            Pill(draw, MenuButtonBezel(frame), CornerRadii(Px(metrics.radius)), colors.comboBoxBackground, false);
        }
        const ImRect plate = ComboBoxPlate(frame);
        const ImVec2 min = plate.Min + Px(metrics.comboChevron);
        const Font font = Font::System(metrics.comboChevronSize, FontWeight::Heavy).WithLineHeight(metrics.downChevronFrame.y);
        const ImRect chevron(min, min + Px(metrics.downChevronFrame));
        // The accent comes in with the window's key state, as on pop-up plates.
        KeyCrossFade(true, [&] {
            const Interaction::DisabledFade fade;
            Pill(draw, plate, CornerRadii(Px(metrics.indicatorRadius)), false);
            Typography::DrawSymbol(draw, Symbols::ChevronDown, font, chevron, colors.LabelColor(Environment().enabled));
        }, [&] {
            AccentFill(draw, plate, CornerRadii(Px(metrics.indicatorRadius)), colors.controlAccent);
            Typography::DrawSymbol(draw, Symbols::ChevronDown, font, chevron, Rgba::White(1.0f));
        });
    }

    void SymbolMenuButton(ImDrawList* draw, const ImRect& frame, unsigned symbol, bool open, bool plain_indicator, bool focused) {
        const Metrics::BezelPopUpMetrics metrics = Metrics::BezelPopUp();
        const Palette& colors = Theme::Colors();
        MenuPill(draw, frame, open);
        const ImRect label = MenuButtonLabel(frame);
        const Rgba color = colors.LabelColor(Environment().enabled);
        Typography::DrawSymbol(draw, symbol, Font::Style(TextStyle::Body), ImRect(frame.Min.x + Px(plain_indicator ? metrics.plainInset : metrics.symbolInset), label.Min.y, label.Max.x, label.Max.y), color, TextAlignment::Leading);
        if (plain_indicator) {
            PlainChevron(draw, frame, color);
        } else {
            const float left = frame.Max.x - Px(metrics.symbolTrailingInset + metrics.indicator);
            const float top = frame.Min.y + Px(metrics.symbolIndicatorTop);
            MenuIndicatorPlate(draw, ImRect(left, top, left + Px(metrics.indicator), top + Px(metrics.indicator)), MenuIndicator::Down);
        }
        if (focused)
            MenuButtonFocusRing(draw, frame);
    }

    void MenuTarget(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii) {
        Draw::StrokeRoundedRect(draw, rect, radii, Theme::Colors().accentPressed, Px(Metrics::Menu().targetRing), StrokeAlignment::Inside);
    }

    void Track(ImDrawList* draw, const ImRect& rect) {
        const CornerRadii radii(rect.GetHeight() * 0.5f);
        Draw::FillRoundedRect(draw, rect, radii, Theme::Colors().track, CornerStyle::Circular);
        Draw::InnerShadows(draw, rect, radii, Theme::TrackInnerShadows(), CornerStyle::Circular);
    }

    void KeyCrossFade(bool accented, const std::function<void()>& plain, const std::function<void()>& lit) {
        const float key = accented && Environment().enabled ? Environment().KeyAmount() : 0.0f;
        if (key < 1.0f) {
            const Draw::Opacity fade(1.0f - key);
            plain();
        }
        if (key > 0.0f) {
            const Draw::Opacity fade(key);
            lit();
        }
    }

    void UpDownChevrons(ImDrawList* draw, const ImRect& indicator, Rgba color, const Metrics::ChevronPair& chevrons) {
        const Font font = Font::System(chevrons.size, FontWeight::Heavy);
        const ImVec2 frame = Px(chevrons.frame);
        const float left = indicator.GetCenter().x - frame.x * 0.5f;
        const ImRect upper(left, indicator.Min.y + Px(chevrons.upperY), left + frame.x, indicator.Min.y + Px(chevrons.upperY) + frame.y);
        const ImRect lower(left, indicator.Min.y + Px(chevrons.lowerY), left + frame.x, indicator.Min.y + Px(chevrons.lowerY) + frame.y);
        Typography::DrawSymbol(draw, Symbols::ChevronUp, font, upper, color);
        Typography::DrawSymbol(draw, Symbols::ChevronDown, font, lower, color);
    }
} // namespace Cupertino::Bezel
