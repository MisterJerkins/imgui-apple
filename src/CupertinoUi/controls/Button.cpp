#include "Button.h"

#include "Bezel.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/MenuContent.h"
#include "core/Metrics.h"
#include "core/Symbols.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"

#include <optional>

namespace Cupertino {
    // The label alone: a large-scale symbol in the secondary color (Battery Health's info button @2x) or the title in
    // the label color; both darken while pressed. A large symbol's frame reaches 1 em above the text baseline and
    // 0.385 em below it (the info button makes its row 38 pt) and a point to either side.
    static bool BorderlessButton(const char* label, const ButtonOptions& options) {
        const Font font = options.font;
        const Font symbol_font = font.ImageScale(options.symbolScale);
        const std::string_view text = Interaction::VisibleLabel(label);
        const float width = options.symbol ? Typography::SymbolWidth(options.symbol, symbol_font) + Px(2.0f) : Typography::Width(font, text);
        const float height = options.symbol ? font.size * 1.385f : font.lineHeight;
        const float baseline = options.symbol ? Px(font.size) : Typography::Baseline(font);
        const ImRect frame = Layout::Place(Layout::Placement{.size = ImVec2(width, Px(height)), .baseline = baseline});
        if (Layout::IsMeasuring())
            return false;

        const Interaction::Response response = Interaction::Button(ImGui::GetID(label), frame);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const bool pressed = response.held && response.hovered;
        if (options.symbol) {
            // The symbol sits on the text baseline of the frame.
            const float line_top = frame.Min.y + baseline - Typography::Baseline(font);
            const Rgba color = !Environment().enabled ? colors.tertiaryLabel : pressed ? colors.label : colors.secondaryLabel;
            Typography::DrawSymbol(draw, options.symbol, symbol_font, ImRect(frame.Min.x, line_top, frame.Max.x, line_top + Px(font.lineHeight)), color);
        } else if (options.style == ButtonStyle::Link) {
            if (response.hovered && Environment().enabled)
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            const Rgba color = !Environment().enabled ? colors.tertiaryLabel : pressed ? colors.link.Opacity(0.7f) : colors.link;
            Typography::Draw(draw, font, frame, color, text);
        } else {
            const Rgba color = !Environment().enabled ? colors.tertiaryLabel : pressed ? colors.secondaryLabel : colors.label;
            Typography::Draw(draw, font, frame, color, text);
        }
        if (response.focused)
            Draw::FocusRing(draw, frame, CornerRadii(Px(4.0f)), colors.accent);
        return response.pressed;
    }

    // Accessory bar buttons (Finder's search scope bar @2x), 16 pt tall with r 4 corners and the title 7.5 pt in. A scope
    // is its Bold 12 title in the secondary color, or darker on the gray plate while on or pressed; an action is its
    // Regular 12 title, or a small symbol in an 18 pt button, inside a point of outline, filled while pressed.
    static bool AccessoryBarButton(const char* label, const ButtonOptions& options, bool* is_on) {
        const Metrics::AccessoryBarMetrics& metrics = Metrics::AccessoryBar();
        const bool action = options.style == ButtonStyle::AccessoryBarAction;
        const Font font = action ? Font::System(metrics.actionFontSize) : Font::System(metrics.fontSize, FontWeight::Bold);
        const std::string_view title = Interaction::VisibleLabel(label);
        const float padding = Px(metrics.padding);
        const float width = options.symbol ? (action ? Px(metrics.symbolWidth) : Typography::SymbolWidth(options.symbol, font) + 2.0f * padding) : Typography::Width(font, title) + 2.0f * padding;
        const float height = Px(metrics.buttonHeight);
        const ImRect frame = Layout::Place(Layout::Placement{.size = ImVec2(width, height), .baseline = Typography::CenteredBaseline(font, height)});
        if (Layout::IsMeasuring())
            return false;
        const Interaction::Response response = Interaction::Button(ImGui::GetID(label), frame);
        if (response.pressed && is_on)
            *is_on = !*is_on;
        const bool pressed = response.held && response.hovered;
        const bool on = is_on && *is_on;
        const bool enabled = Environment().enabled;
        const Palette& colors = Theme::Colors();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const CornerRadii radii(Px(metrics.radius));
        Rgba color = colors.label;
        if (action) {
            if (pressed)
                Draw::FillRoundedRect(draw, frame, radii, colors.accessoryBarPlate);
            Draw::StrokeRoundedRect(draw, frame, radii, colors.accessoryBarOutline, Px(1.0f));
        } else {
            if (on || pressed)
                Draw::FillRoundedRect(draw, frame, radii, pressed && !on ? colors.accessoryBarPlate.Opacity(0.7f) : colors.accessoryBarPlate);
            color = on || pressed ? colors.accessoryBarSelectedText : colors.secondaryLabel;
        }
        if (!enabled)
            color = colors.tertiaryLabel;
        if (options.symbol)
            Typography::DrawSymbol(draw, options.symbol, font.ImageScale(SymbolScale::Small), frame, color);
        else
            Typography::Draw(draw, font, Bezel::TitleFrame(frame, 0.0f, font, metrics.titleRaise), color, title, TextAlignment::Center);
        if (response.focused)
            Draw::FocusRing(draw, frame, radii, colors.accent);
        return response.pressed;
    }

    ImRect ToolbarPlate(const ImRect& slot) {
        const float inset = Px(Metrics::Window().toolbarPlateInset);
        return ImRect(slot.Min.x, slot.Min.y + inset, slot.Max.x, slot.Max.y - inset);
    }

    bool ToolbarButton(const char* id, const ImRect& slot, bool enabled, const std::function<void(ImDrawList*, Rgba)>& content) {
        const Palette& colors = Theme::Colors();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const ImRect plate = ToolbarPlate(slot);
        bool pressed = false;
        WithEnvironment([&](EnvironmentValues& environment) { environment.enabled = environment.enabled && enabled; }, [&] {
            const Interaction::Response response = Interaction::Button(ImGui::GetID(id), plate);
            if (response.hovered && enabled)
                Draw::FillRoundedRect(draw, plate, CornerRadii(Px(Metrics::Window().toolbarPlateRadius)), response.held ? colors.fill : colors.tertiaryFill);
            pressed = response.pressed;
        });
        content(draw, enabled ? colors.toolbarSymbol : colors.toolbarSymbolDisabled);
        return pressed && enabled;
    }

    bool ToolbarButton(const char* id, const ImRect& slot, unsigned symbol, bool enabled) {
        const ImVec2 center(slot.GetCenter().x, slot.Min.y + Px(Metrics::Window().symbolCenterY));
        return ToolbarButton(id, slot, enabled, [&](ImDrawList* draw, Rgba color) { DrawToolbarSymbol(draw, symbol, center, color); });
    }

    void DrawToolbarSymbol(ImDrawList* draw, unsigned symbol, ImVec2 center, Rgba color) {
        const Metrics::WindowMetrics& metrics = Metrics::Window();
        const Font font = Font::System(metrics.toolbarSymbolSize, FontWeight::Medium).ImageScale(SymbolScale::Large);
        const ImVec2 half = Px(metrics.toolbarSymbolFrame) * 0.5f;
        Typography::DrawSymbol(draw, symbol, font, ImRect(center - half, center + half), color);
    }

    // A push button placed like a label's line: the bezel 20 pt in a 22 pt frame, its title on the line's baseline, the
    // bezel reaching over a form row's top inset (Privacy & Security @2x). Nothing while measuring. Its size follows the
    // environment's control size.
    struct PushButtonFrame {
        ImRect frame;
        Interaction::Response response;
    };

    static Font PushButtonFont(const Metrics::PushButtonMetrics& metrics) {
        return Font::System(metrics.titleSize).WithLineHeight(metrics.titleLineHeight);
    }

    // The symbol and the title side by side, as wide as they are together.
    static float PushButtonContentWidth(const Metrics::PushButtonMetrics& metrics, const Font& font, unsigned symbol, std::string_view text) {
        const float symbol_width = symbol ? Typography::SymbolWidth(symbol, font) : 0.0f;
        const float gap = symbol && !text.empty() ? Px(metrics.symbolGap) : 0.0f;
        return symbol_width + gap + Typography::Width(font, text);
    }

    static std::optional<PushButtonFrame> PlacePushButton(const char* label, unsigned symbol, float min_width) {
        const Metrics::PushButtonMetrics metrics = Metrics::PushButton(Environment().controlSize);
        const Font font = PushButtonFont(metrics);
        const std::string_view text = Interaction::VisibleLabel(label);
        const float width = ImMax(PushButtonContentWidth(metrics, font, symbol, text) + Px(2.0f * metrics.titleInset), Px(ImMax(min_width, Environment().buttonMinWidth)));
        const ImVec2 size(width, Px(metrics.height));
        const float title_top = Bezel::TitleFrame(ImRect(ImVec2(0.0f, 0.0f), size), 0.0f, font, metrics.titleRaise).Min.y;
        const ImRect frame = Layout::Place(Layout::Placement{.size = size, .baseline = title_top + Typography::Baseline(font), .overhang = title_top});
        if (Layout::IsMeasuring())
            return std::nullopt;
        return PushButtonFrame{frame, Interaction::Button(ImGui::GetID(label), frame)};
    }

    // The white bezel with the title in title_color, or, accented, the accent bezel with a white title: accented cross-fades
    // to white as the window stops being key. Disabled like the kit: the bezel fades as one layer, the title turns tertiary.
    static void DrawPushButton(const PushButtonFrame& button, const char* label, unsigned symbol, bool accented, Rgba title_color) {
        const Metrics::PushButtonMetrics metrics = Metrics::PushButton(Environment().controlSize);
        const Font font = PushButtonFont(metrics);
        const std::string_view text = Interaction::VisibleLabel(label);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const CornerRadii radii(Px(metrics.radius));
        const bool pressed = button.response.held && button.response.hovered;
        const ImRect label_frame = Bezel::TitleFrame(button.frame, metrics.titleInset, font, metrics.titleRaise);
        // A symbol leads the title, the two centered together.
        const auto content = [&](Rgba color) {
            if (!symbol) {
                Typography::Draw(draw, font, label_frame, color, text, TextAlignment::Center);
                return;
            }
            const float left = label_frame.GetCenter().x - PushButtonContentWidth(metrics, font, symbol, text) * 0.5f;
            const float symbol_right = left + Typography::SymbolWidth(symbol, font);
            Typography::DrawSymbol(draw, symbol, font, ImRect(left, label_frame.Min.y, symbol_right, label_frame.Max.y), color, TextAlignment::Leading);
            if (!text.empty())
                Typography::Draw(draw, font, ImRect(symbol_right + Px(metrics.symbolGap), label_frame.Min.y, label_frame.Max.x, label_frame.Max.y), color, text);
        };
        Bezel::KeyCrossFade(accented, [&] {
            {
                const Interaction::DisabledFade fade;
                Bezel::Pill(draw, button.frame, radii, pressed);
            }
            content(Environment().enabled ? title_color : colors.tertiaryLabel);
        }, [&] {
            Bezel::AccentFill(draw, button.frame, radii, pressed ? colors.accentPressed : colors.controlAccent);
            content(Rgba::White(1.0f));
        });
        if (button.response.focused)
            Draw::FocusRing(draw, button.frame, radii, colors.accent);
    }

    bool Button(const char* label, const ButtonOptions& options) {
        if (MenuContent::IsCollecting())
            return MenuContent::AddItem({.title = std::string(Interaction::VisibleLabel(label)), .subtitle = options.subtitle ? options.subtitle : "", .symbol = options.symbol, .image = options.image, .shortcut = options.shortcut, .badge = options.badge ? options.badge : "", .alternate = options.alternate});
        if (Layout::ParentRole() == Layout::Role::Toolbar && !options.symbol) {
            // A titled toolbar item (kit's Text Button): the title in the toolbar's symbol color on the item's plate.
            const Metrics::WindowMetrics& metrics = Metrics::Window();
            const std::string_view title = Interaction::VisibleLabel(label);
            const Font font = Font::Style(TextStyle::Body);
            const ImRect slot = Layout::Place(ImVec2(Typography::Width(font, title) + Px(2.0f * metrics.toolbarTitlePadding), Px(metrics.symbolButtonSlot)));
            if (Layout::IsMeasuring())
                return false;
            return ToolbarButton(label, slot, Environment().enabled, [&](ImDrawList* draw, Rgba color) { Typography::Draw(draw, font, slot, color, title, TextAlignment::Center); });
        }
        if (Layout::ParentRole() == Layout::Role::Toolbar && options.symbol) {
            const Metrics::WindowMetrics& metrics = Metrics::Window();
            const ImRect slot = Layout::Place(Px(ImVec2(metrics.toolbarButtonWidth, metrics.symbolButtonSlot)));
            if (Layout::IsMeasuring())
                return false;
            return ToolbarButton(label, slot, options.symbol, Environment().enabled);
        }
        if (options.style == ButtonStyle::Borderless || options.style == ButtonStyle::Link)
            return BorderlessButton(label, options);
        if (options.style == ButtonStyle::AccessoryBar || options.style == ButtonStyle::AccessoryBarAction)
            return AccessoryBarButton(label, options, nullptr);
        const std::optional<PushButtonFrame> button = PlacePushButton(label, options.symbol, options.minWidth);
        if (!button)
            return false;
        bool clicked = button->response.pressed;
        if (options.role == ButtonRole::Default && Interaction::Enabled() && ImGui::IsKeyPressed(ImGuiKey_Enter, false))
            clicked = true;
        if (options.role == ButtonRole::Cancel && Interaction::Enabled() && ImGui::IsKeyPressed(ImGuiKey_Escape, false))
            clicked = true;
        const Palette& colors = Theme::Colors();
        const Rgba title = options.role == ButtonRole::Destructive ? Theme::SystemRed().Opacity(0.85f) : colors.label;
        DrawPushButton(*button, label, options.symbol, options.role == ButtonRole::Default, title);
        return clicked;
    }

    bool ButtonToggle(const char* label, bool* is_on, bool lit) {
        const std::optional<PushButtonFrame> button = PlacePushButton(label, 0, 0.0f);
        if (!button)
            return false;
        if (button->response.pressed)
            *is_on = !*is_on;
        const Palette& colors = Theme::Colors();
        DrawPushButton(*button, label, 0, *is_on && lit, *is_on ? colors.controlAccent : colors.label);
        return button->response.pressed;
    }

    bool AccessoryBarToggle(const char* label, bool* is_on) {
        return AccessoryBarButton(label, {.style = ButtonStyle::AccessoryBar}, is_on);
    }

    bool HelpButton(const char* id) {
        const Metrics::HelpButtonMetrics& metrics = Metrics::HelpButton();
        const ImRect frame = Layout::Place(Px(ImVec2(metrics.diameter, metrics.diameter)));
        if (Layout::IsMeasuring())
            return false;

        const Interaction::Response response = Interaction::Button(ImGui::GetID(id), frame);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const CornerRadii radii(frame.GetWidth() * 0.5f);
        {
            const Interaction::DisabledFade fade;
            Bezel::Pill(draw, frame, radii, response.held && response.hovered, CornerStyle::Circular);
        }
        // The kit's question mark: Semibold 13/16 in an 11 x 16 frame at (4.5, 1.5).
        const ImRect symbol(frame.Min + Px(ImVec2(4.5f, 1.5f)), frame.Min + Px(ImVec2(15.5f, 17.5f)));
        const Rgba color = colors.LabelColor(Environment().enabled);
        Typography::DrawSymbol(draw, Symbols::Questionmark, Font::System(metrics.symbolSize, FontWeight::Semibold).WithLineHeight(16.0f), symbol, color);
        if (response.focused)
            Draw::FocusRing(draw, frame, radii, colors.accent, CornerStyle::Circular);
        return response.pressed;
    }
} // namespace Cupertino
