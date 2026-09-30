#include "Alert.h"

#include "Sheet.h"
#include "controls/Bezel.h"
#include "controls/Toggle.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/Motion.h"
#include "core/State.h"
#include "core/TextInput.h"
#include "core/Theme.h"
#include "core/Color.h"
#include "core/Typography.h"
#include "layout/Layout.h"
#include "layout/Stacks.h"
#include "window/Window.h"

#include <algorithm>
#include <cmath>
#include <string_view>
#include <vector>

namespace Cupertino {
    // The caution icon of a critical alert (VS Code's alert @2x, 64 pt): a yellow triangle, its apex rounder than its
    // base, in a light rim; a white exclamation mark over a gray edge; the app icon as a badge over its lower right.
    static void DrawCaution(ImDrawList* draw, const ImRect& frame, void (*badge)(ImDrawList* draw, const ImRect& frame)) {
        const Metrics::AlertMetrics& metrics = Metrics::Alert();
        const float unit = frame.GetWidth() / metrics.icon;
        const auto at = [&](float x, float y) { return frame.Min + ImVec2(x, y) * unit; };
        const ImVec2 inner[] = {at(32.0f, 2.25f), at(61.15f, 55.25f), at(2.85f, 55.25f)};
        const float inner_radii[] = {metrics.cautionApexRadius * unit, metrics.cautionBaseRadius * unit, metrics.cautionBaseRadius * unit};
        // The rim: the same triangle grown by its width.
        const float rim = metrics.cautionRim * unit;
        ImVec2 outer[3];
        Draw::OffsetTriangle(inner, rim, outer);
        const float outer_radii[] = {inner_radii[0] + rim, inner_radii[1] + rim, inner_radii[2] + rim};
        Draw::FillRoundedTriangle(draw, outer, outer_radii, Rgba::Hex(0xFDFDFD), Rgba::Hex(0xE6E6E6));
        Draw::FillRoundedTriangle(draw, inner, inner_radii, Rgba::Hex(0xFDE442), Rgba::Hex(0xE1AB00));
        // The mark: a bar and a dot, lit from above, each over a gray edge a point lower.
        const float bar = 2.4f * unit;
        const ImRect stem(at(32.0f, 20.5f) - ImVec2(bar, 0.0f), at(32.0f, 38.5f) + ImVec2(bar, 0.0f));
        const ImVec2 dot = at(32.0f, 45.25f);
        const float dot_radius = 3.0f * unit;
        const ImVec2 edge(0.0f, unit);
        const Rgba edge_color = Rgba::Hex(0x999999, 0.55f);
        Draw::FillRoundedRect(draw, ImRect(stem.Min + edge, stem.Max + edge), CornerRadii(bar), edge_color, CornerStyle::Circular);
        Draw::FillCircle(draw, dot + edge, dot_radius, edge_color);
        const int first = draw->VtxBuffer.Size;
        Draw::FillRoundedRect(draw, stem, CornerRadii(bar), Rgba::White(1.0f), CornerStyle::Circular);
        Draw::FillCircle(draw, dot, dot_radius, Rgba::White(1.0f));
        Draw::ShadeVertices(draw, first, [&](ImVec2 point) { return Blend::Mix(Rgba::Hex(0xFAFAFA), Rgba::Hex(0xE4E4E4), ImSaturate((point.y - stem.Min.y) / (dot.y + dot_radius - stem.Min.y))); });
        if (badge)
            badge(draw, ImRect(at(35.0f, 35.0f), at(60.5f, 60.5f)));
    }

    // Alert buttons are large, 28 pt: the default one is the accent bezel, a destructive first one the same in red, the
    // others a flat gray fill.
    static bool DrawAlertButton(ImGuiID id, const ImRect& frame, const AlertButton& button, bool first, bool focused) {
        const Metrics::AlertMetrics& metrics = Metrics::Alert();
        const Palette& colors = Theme::Colors();
        const Interaction::Response response = Interaction::Button(id, frame);
        const bool pressed = response.held && response.hovered;
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const CornerRadii radii(Px(metrics.buttonRadius));
        const bool destructive = button.role == ButtonRole::Destructive;
        const bool accented = button.role == ButtonRole::Default || (destructive && first);
        const Rgba fill = destructive ? colors.destructiveFill : colors.controlAccent;
        if (accented)
            Bezel::AccentFill(draw, frame, radii, pressed ? Theme::Pressed(fill) : fill);
        else
            Draw::FillRoundedRect(draw, frame, radii, pressed ? colors.alertButtonPressed : colors.alertButton);
        const Rgba text = accented ? Rgba::White(1.0f) : destructive ? Theme::SystemRed() : colors.alertButtonText;
        Typography::Draw(draw, Font::Style(TextStyle::Body), Bezel::TitleFrame(frame, 0.0f), text, button.title, TextAlignment::Center);
        if (response.focused || focused)
            Draw::FocusRing(draw, frame, radii, colors.accent);
        return response.pressed;
    }

    // An alert's text field: the bezel, the ring while focused, and the text 10 pt in.
    static void AlertField(ImGuiID id, const ImRect& frame, const AlertTextField& field) {
        const Metrics::AlertMetrics& metrics = Metrics::Alert();
        const Palette& colors = Theme::Colors();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const CornerRadii radii(Px(metrics.fieldRadius));
        // The light line under the field shows only below its bottom edge.
        if (colors.alertFieldHighlight.a > 0.0f) {
            draw->PushClipRect(ImVec2(frame.Min.x, frame.Max.y), ImVec2(frame.Max.x, frame.Max.y + Px(0.5f)), true);
            Draw::FillRoundedRect(draw, ImRect(frame.Min.x, frame.Min.y + Px(0.5f), frame.Max.x, frame.Max.y + Px(0.5f)), radii, colors.alertFieldHighlight);
            draw->PopClipRect();
        }
        Draw::FillRoundedRect(draw, frame, radii, colors.alertField);
        Draw::StrokeRoundedRect(draw, frame, radii, colors.alertFieldRim, Px(0.5f));
        const float inset = Px(metrics.fieldTextInset);
        const TextInput::Style style = {.color = colors.alertText, .placeholder = field.placeholder, .secure = field.secure};
        TextInput::Edit(id, ImRect(frame.Min.x + inset, frame.Min.y, frame.Max.x - inset, frame.Max.y), field.text, style);
        if (ImGui::GetActiveID() == id)
            Draw::FocusRing(draw, frame, radii, colors.accent);
    }

    struct AlertState {
        AnimatedFloat presentation;
        bool shown = false;
        int focus = -1;
        // The field to give the keyboard to on the next frame, or -1.
        int focusField = -1;
    };

    int Alert(const char* title, bool* is_presented, std::span<const AlertButton> buttons, const AlertOptions& options) {
        // The alert and the dim of its window fade together, as a sheet's do; a dismissed alert draws until it has faded
        // out and takes no more input.
        AlertState& state = State::Get<AlertState>(ImGui::GetID(is_presented));
        const float presentation = state.presentation.Update(*is_presented ? 1.0f : 0.0f, Animation::EaseOut(Metrics::Sheet().fade));
        if (presentation <= 0.0f) {
            state.shown = false;
            return -1;
        }
        if (!state.shown) {
            state.shown = true;
            state.focus = options.focus;
            state.focusField = options.textFields.empty() ? -1 : 0;
        }
        const bool interactive = *is_presented;
        const Metrics::AlertMetrics& metrics = Metrics::Alert();
        const Font title_font = Font::Style(TextStyle::Body).Weight(FontWeight::Bold);
        const Font message_font = Font::Style(TextStyle::Subheadline);
        const std::string_view message = options.message ? options.message : "";
        const float text_width = metrics.width - 2.0f * metrics.textInset;

        // Measured on macOS 15 alerts @2x, top to bottom: 20, the 64 pt icon, 19.5, title, 10, message, 16, buttons, 16,
        // suppression, 16. Two buttons share a row while both titles fit half of it; otherwise they stack in the given
        // order with a cancel button last and further apart (Keychain Access @2x).
        const float title_height = Pt(Typography::Measure(title_font, title, Px(text_width)).y);
        const float message_height = message.empty() ? 0.0f : Pt(Typography::Measure(message_font, message, Px(text_width)).y);
        const int count = int(buttons.size());
        const float half_row = (metrics.width - 2.0f * metrics.padding - metrics.buttonGap) * 0.5f - 2.0f * metrics.buttonTitleInset;
        const bool stacked = count > 2 || std::any_of(buttons.begin(), buttons.end(), [&](const AlertButton& button) {
            return Pt(Typography::Width(Font::Style(TextStyle::Body), button.title)) > half_row;
        });
        std::vector<int> order;
        for (int pass = 0; pass < 2; ++pass) {
            for (int i = 0; i < count; ++i) {
                if ((buttons[size_t(i)].role == ButtonRole::Cancel) == (pass == 1))
                    order.push_back(i);
            }
        }
        std::vector<float> tops(size_t(count), 0.0f);
        float stack = 0.0f;
        for (size_t k = 0; k < order.size(); ++k) {
            if (k > 0)
                stack += buttons[size_t(order[k])].role == ButtonRole::Cancel ? metrics.cancelGap : metrics.stackedGap;
            tops[size_t(order[k])] = stack;
            stack += metrics.buttonHeight;
        }
        const float buttons_height = stacked ? stack : metrics.buttonHeight;
        const float icon_height = options.icon || options.critical ? metrics.icon + metrics.iconSpacing : 0.0f;
        const float message_block = message.empty() ? 0.0f : metrics.messageSpacing + message_height;
        const bool suppressible = options.suppression && options.suppressed;
        const float suppression_block = suppressible ? metrics.buttonsSpacing + Font::Style(TextStyle::Body).lineHeight : 0.0f;
        const int field_count = int(options.textFields.size());
        const float fields_block = field_count > 0 ? metrics.fieldsSpacing + metrics.fieldPitch * float(field_count - 1) + metrics.fieldHeight + metrics.fieldsBottom - metrics.buttonsSpacing : 0.0f;
        // The alert's window is whole points high; the half point left over goes above the content (Empty Trash and
        // Keychain Access @2x, 236 and 342 pt).
        const float content = metrics.top + icon_height + title_height + message_block + fields_block + metrics.buttonsSpacing + buttons_height + suppression_block + metrics.padding;
        const ImVec2 size(metrics.width, std::ceil(content));

        // Inside a window the alert is its sheet, as SwiftUI presents .alert on macOS: the window dims and takes no input,
        // and the alert stands where a sheet of its size would. Elsewhere it is app-modal in the middle of the screen, over
        // a transparent window that takes the pointer.
        const SheetHost host = AttachSheet(presentation);
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 origin = host.attached ? SheetOrigin(host.frame, size, true) : Draw::Snap(viewport->GetCenter() - Px(size) * 0.5f);
        const ImRect panel(origin, origin + Px(size));
        if (!host.attached)
            Interaction::Backdrop("##CupertinoAlertBackdrop");

        Interaction::BeginOverlay(title, panel);
        // Like a sheet, the alert takes input and draws with its window's own environment and opacity.
        int chosen = -1;
        InSheet(host, host.alpha * presentation, nullptr, [&] {
            const Palette& colors = Theme::Colors();
            ImDrawList* draw = ImGui::GetWindowDrawList();
            const CornerRadii radii(Px(metrics.radius));
            draw->PushClipRectFullScreen();
            Draw::DropShadows(draw, panel, radii, host.attached ? Theme::SheetShadows() : Theme::WindowShadows(), CornerStyle::Circular);
            Draw::StrokeRoundedRect(draw, panel, radii, colors.windowOutline, Px(0.5f), StrokeAlignment::Outside, CornerStyle::Circular);
            draw->PopClipRect();
            // An alert on its own is a window with its edge; an attached one keeps only the rim (Mail @2x: no highlight).
            Draw::FillRoundedRect(draw, panel, radii, host.attached ? colors.attachedAlertBackground : colors.alertBackground, CornerStyle::Circular);
            if (host.attached)
                Draw::StrokeRoundedRect(draw, panel, radii, colors.windowRim, Px(Environment().IsDark() ? 1.0f : 0.5f), StrokeAlignment::Inside, CornerStyle::Circular);
            else
                DrawWindowEdge(draw, panel, radii, CornerStyle::Circular);

            float y = panel.Min.y + Px(metrics.top + size.y - content);
            if (options.icon || options.critical) {
                const float left = panel.GetCenter().x - Px(metrics.icon) * 0.5f;
                const ImRect icon(left, y, left + Px(metrics.icon), y + Px(metrics.icon));
                if (options.critical)
                    DrawCaution(draw, icon, options.icon);
                else
                    options.icon(draw, icon);
                y += Px(metrics.icon + metrics.iconSpacing);
            }
            const float text_left = panel.Min.x + Px(metrics.textInset);
            Typography::DrawWrapped(draw, title_font, ImRect(text_left, y, text_left + Px(text_width), y + Px(title_height)), colors.alertText, title, TextAlignment::Center);
            y += Px(title_height);
            if (!message.empty()) {
                y += Px(metrics.messageSpacing);
                Typography::DrawWrapped(draw, message_font, ImRect(text_left, y, text_left + Px(text_width), y + Px(message_height)), colors.alertText, message, TextAlignment::Center);
                y += Px(message_height);
            }
            if (options.showsHelp) {
                const ImVec2 min(panel.Max.x - Px(metrics.helpTrailing + metrics.help), panel.Min.y + Px(metrics.helpTop));
                const ImRect help(min, min + Px(ImVec2(metrics.help, metrics.help)));
                Draw::FillCircle(draw, help.GetCenter(), help.GetWidth() * 0.5f, colors.alertButton);
                Typography::Draw(draw, Font::System(metrics.helpSymbol, FontWeight::Semibold), help, colors.label, "?", TextAlignment::Center);
            }

            // Text fields across the buttons' row; Tab moves the keyboard to the next one.
            const float buttons_left = panel.Min.x + Px(metrics.padding);
            const float row_width = Px(metrics.width - 2.0f * metrics.padding);
            if (field_count > 0) {
                y += Px(metrics.fieldsSpacing);
                for (int i = 0; i < field_count; ++i) {
                    ImGui::PushID(i);
                    const ImGuiID id = ImGui::GetID("field");
                    ImGui::PopID();
                    const bool tabbed = interactive && ImGui::GetActiveID() == id && ImGui::IsKeyPressed(ImGuiKey_Tab, false);
                    if (interactive && state.focusField == i) {
                        ImGui::SetActiveID(id, ImGui::GetCurrentWindow());
                        ImGui::SetFocusID(id, ImGui::GetCurrentWindow());
                        state.focusField = -1;
                    }
                    const float top = y + Px(metrics.fieldPitch * float(i));
                    AlertField(id, ImRect(buttons_left, top, buttons_left + row_width, top + Px(metrics.fieldHeight)), options.textFields[size_t(i)]);
                    if (tabbed)
                        state.focusField = (i + (ImGui::GetIO().KeyShift ? field_count - 1 : 1)) % field_count;
                }
                y += Px(metrics.fieldPitch * float(field_count - 1) + metrics.fieldHeight + metrics.fieldsBottom - metrics.buttonsSpacing);
            }

            // Buttons: side by side with the first on the leading side, or stacked as ordered above.
            y += Px(metrics.buttonsSpacing);
            const float gap = Px(metrics.buttonGap);
            const float button_width = stacked || count == 1 ? row_width : (row_width - gap) * 0.5f;
            // Tab moves the keyboard focus through the buttons as they stand, Space presses the focused one.
            if (interactive && state.focus >= 0 && ImGui::IsKeyPressed(ImGuiKey_Tab, false)) {
                const auto at = std::find(order.begin(), order.end(), state.focus);
                const int step = ImGui::GetIO().KeyShift ? count - 1 : 1;
                state.focus = order[size_t((std::distance(order.begin(), at) + step) % count)];
            }
            if (interactive && state.focus >= 0 && ImGui::IsKeyPressed(ImGuiKey_Space, false))
                chosen = state.focus;
            for (int i = 0; i < count; ++i) {
                const AlertButton& button = buttons[size_t(i)];
                const float left = stacked || count == 1 ? buttons_left : buttons_left + float(i) * (button_width + gap);
                const float top = stacked ? y + Px(tops[size_t(i)]) : y;
                ImGui::PushID(i);
                if (DrawAlertButton(ImGui::GetID("button"), ImRect(left, top, left + button_width, top + Px(metrics.buttonHeight)), button, i == 0, i == state.focus) && interactive)
                    chosen = i;
                ImGui::PopID();
                const bool answers_return = button.role == ButtonRole::Default || (button.role == ButtonRole::Destructive && i == 0);
                if (interactive && answers_return && (ImGui::IsKeyPressed(ImGuiKey_Enter, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false)))
                    chosen = i;
                if (interactive && button.role == ButtonRole::Cancel && ImGui::IsKeyPressed(ImGuiKey_Escape, false))
                    chosen = i;
            }
            y += Px(buttons_height);
            if (suppressible) {
                y += Px(metrics.buttonsSpacing);
                Layout::ContainerSpec row;
                row.arrangement = Layout::Arrangement::Vertical;
                row.alignment = Alignment{HorizontalAlignment::Center, VerticalAlignment::Center};
                // The checkbox is centered with 4 pt of trailing room after its title, as AppKit's cell keeps, so its ink
                // stands 2 pt left of the middle (VS Code alert @2x).
                Layout::Region(ImGui::GetID("##suppression"), ImRect(panel.Min.x, y, panel.Max.x, y + Px(Font::Style(TextStyle::Body).lineHeight)), row, [&] {
                    Padding(EdgeInsets{0.0f, 0.0f, 0.0f, metrics.suppressionTrailing}, [&] { Toggle(options.suppression, options.suppressed, {.style = ToggleStyle::Checkbox}); });
                });
            }
        });
        ImGui::End();
        if (chosen >= 0)
            *is_presented = false;
        return chosen;
    }

    int Alert(const char* title, bool* is_presented, std::initializer_list<AlertButton> buttons, const AlertOptions& options) {
        return Alert(title, is_presented, std::span<const AlertButton>(buttons.begin(), buttons.size()), options);
    }

    int ConfirmationDialog(const char* title, bool* is_presented, std::span<const AlertButton> actions, const AlertOptions& options) {
        std::vector<AlertButton> buttons(actions.begin(), actions.end());
        buttons.push_back({"Cancel", ButtonRole::Cancel});
        return Alert(title, is_presented, buttons, options);
    }

    int ConfirmationDialog(const char* title, bool* is_presented, std::initializer_list<AlertButton> actions, const AlertOptions& options) {
        return ConfirmationDialog(title, is_presented, std::span<const AlertButton>(actions.begin(), actions.size()), options);
    }
} // namespace Cupertino
