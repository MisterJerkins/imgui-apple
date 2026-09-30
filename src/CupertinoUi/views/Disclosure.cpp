#include "Disclosure.h"

#include "controls/Bezel.h"
#include "controls/LabeledContent.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/Motion.h"
#include "core/State.h"
#include "core/Symbols.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"
#include "layout/Stacks.h"
#include "views/Sidebar.h"

namespace Cupertino {
    struct DisclosureState {
        AnimatedFloat turn;
    };

    // chevron.right Semibold 13 from the start of its 8 x 16 frame, turned a quarter clockwise about its own center as
    // the group opens and moved to where the kit draws chevron.down.
    static void DrawChevron(ImDrawList* draw, const ImRect& frame, float turn) {
        const Palette& colors = Theme::Colors();
        const Font font = Font::Style(TextStyle::Body).Weight(FontWeight::Semibold);
        const ImVec2 shift = Px(Metrics::Disclosure().openShift) * turn;
        const ImVec2 center(frame.Min.x + shift.x + Typography::SymbolWidth(Symbols::ChevronRight, font) * 0.5f, frame.GetCenter().y + shift.y);
        DrawOutlineChevron(draw, center, font, turn, colors.LabelColor(Environment().enabled));
    }

    float DisclosureTurn(ImGuiID id, bool expanded) {
        return State::Get<DisclosureState>(id).turn.Update(expanded ? 1.0f : 0.0f, Animation::EaseInOut(0.2f));
    }

    void DrawOutlineChevron(ImDrawList* draw, ImVec2 center, const Font& font, float turn, Rgba color) {
        const float half = Typography::SymbolWidth(Symbols::ChevronRight, font) * 0.5f;
        const int first = draw->VtxBuffer.Size;
        Typography::DrawSymbol(draw, Symbols::ChevronRight, font, ImRect(center.x - half, center.y - half * 2.0f, center.x + half, center.y + half * 2.0f), color);
        Draw::RotateVertices(draw, first, center, turn * IM_PI * 0.5f);
    }

    // Toggles on a click anywhere on the header and returns the chevron's turn, 0 closed to 1 open.
    static float DisclosureBehavior(ImGuiID id, const ImRect& header, bool* expanded) {
        if (Interaction::Button(id, header).pressed)
            *expanded = !*expanded;
        return DisclosureTurn(id, *expanded);
    }

    void DisclosureGroup(const char* label, bool* expanded, const std::function<void()>& content) {
        const Metrics::DisclosureMetrics& metrics = Metrics::Disclosure();
        const std::string_view text = Interaction::VisibleLabel(label);
        const Font font = Font::Style(TextStyle::Body);
        if (Layout::ParentRole() == Layout::Role::Sidebar) {
            SidebarDisclosureGroup(label, expanded, content);
            return;
        }
        if (Layout::ParentRole() == Layout::Role::FormSection) {
            const FormRowFrame row = PlaceFormRow({.label = text, .accessoryTop = Metrics::Form().rowVerticalInset});
            if (!Layout::IsMeasuring()) {
                ImDrawList* draw = ImGui::GetWindowDrawList();
                const float turn = DisclosureBehavior(ImGui::GetID(label), row.slot, expanded);
                const ImVec2 chevron = ImVec2(row.content.Min.x, row.label.Min.y) + Px(ImVec2(metrics.formChevronX, 0.0f));
                DrawChevron(draw, ImRect(chevron, chevron + Px(metrics.chevronFrame)), turn);
                const ImRect title(row.content.Min.x + Px(metrics.formLabelX), row.label.Min.y, row.label.Max.x, row.label.Max.y);
                Typography::Draw(draw, font, title, Theme::Colors().LabelColor(Environment().enabled), text);
            }
            if (*expanded)
                content();
            return;
        }

        VStack({.alignment = HorizontalAlignment::Leading, .spacing = metrics.contentSpacing}, [&] {
            const float width = metrics.labelX + Pt(Typography::Width(font, text));
            const ImRect header = Layout::Place(Px(ImVec2(width, font.lineHeight)));
            if (!Layout::IsMeasuring()) {
                ImDrawList* draw = ImGui::GetWindowDrawList();
                const float turn = DisclosureBehavior(ImGui::GetID(label), header, expanded);
                DrawChevron(draw, ImRect(header.Min, header.Min + Px(metrics.chevronFrame)), turn);
                const ImRect title(header.Min.x + Px(metrics.labelX), header.Min.y, header.Max.x, header.Max.y);
                Typography::Draw(draw, font, title, Theme::Colors().LabelColor(Environment().enabled), text);
            }
            if (*expanded)
                Padding(EdgeInsets{0.0f, metrics.labelX, 0.0f, 0.0f}, [&] { VStack({.alignment = HorizontalAlignment::Leading, .spacing = metrics.contentSpacing}, content); });
        });
    }

    void DisclosureGroup(bool* expanded, const std::function<void()>& label, const std::function<void()>& content) {
        if (Layout::ParentRole() == Layout::Role::Sidebar) {
            SidebarDisclosureGroup(expanded, label, content);
            return;
        }
        const Metrics::DisclosureMetrics& metrics = Metrics::Disclosure();
        const ImGuiID id = Layout::NextViewId();
        VStack({.alignment = HorizontalAlignment::Leading, .spacing = metrics.contentSpacing}, [&] {
            HStack({.alignment = VerticalAlignment::Center, .spacing = metrics.labelX - metrics.chevronFrame.x}, [&] {
                const ImRect frame = Layout::Place(Px(metrics.chevronFrame));
                if (!Layout::IsMeasuring())
                    DrawChevron(ImGui::GetWindowDrawList(), frame, DisclosureBehavior(id, frame, expanded));
                label();
            });
            if (*expanded)
                Padding(EdgeInsets{0.0f, metrics.labelX, 0.0f, 0.0f}, [&] { VStack({.alignment = HorizontalAlignment::Leading, .spacing = metrics.contentSpacing}, content); });
        });
    }

    bool DisclosureButton(const char* label, bool* expanded) {
        const Metrics::DisclosureMetrics& metrics = Metrics::Disclosure();
        const ImRect frame = Layout::Place(Layout::Placement{.size = Px(ImVec2(metrics.button, metrics.button)), .baseline = Typography::CenteredBaseline(Font::Style(TextStyle::Body), Px(metrics.button))});
        if (Layout::IsMeasuring())
            return false;
        const Interaction::Response response = Interaction::Button(ImGui::GetID(label), frame);
        if (response.pressed)
            *expanded = !*expanded;
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const CornerRadii radii(Px(Metrics::PushButton().radius));
        {
            const Interaction::DisabledFade fade;
            Bezel::Pill(draw, frame, radii, response.held && response.hovered);
        }
        // chevron Heavy 11 in an 11 x 13 frame at (5.5, 5), as the kit draws it.
        const ImVec2 origin = frame.Min + Px(metrics.buttonChevron);
        const ImRect chevron(origin, origin + Px(ImVec2(11.0f, 13.0f)));
        Typography::DrawSymbol(draw, *expanded ? Symbols::ChevronUp : Symbols::ChevronDown, Font::System(11.0f, FontWeight::Heavy).WithLineHeight(13.0f), chevron, colors.LabelColor(Environment().enabled));
        if (response.focused)
            Draw::FocusRing(draw, frame, radii, colors.accent);
        return response.pressed;
    }
} // namespace Cupertino
