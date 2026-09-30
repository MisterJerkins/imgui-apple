#include "Sidebar.h"

#include "controls/TextField.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/State.h"
#include "core/Symbols.h"
#include "core/TextInput.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"
#include "layout/ScrollView.h"
#include "views/Disclosure.h"

#include <cctype>
#include <string>
#include <vector>

namespace Cupertino {
    struct SidebarListState {
        // A list with disclosure groups gives every row the chevron column; its rows learn it from the last frame.
        bool outline = false;
        bool hasGroups = false;
    };

    // A selected row is emphasized, on the accent with white content, while its list has the focus in the key window.
    static bool Emphasized(bool selected) {
        const EnvironmentValues& environment = Environment();
        return selected && environment.isFocused && environment.controlActiveState == ControlActiveState::Key;
    }

    // The selection platter of a sidebar row: gray unless emphasized; a sheet's sidebar has its own, lighter accent.
    static Rgba PlatterColor(bool emphasized) {
        const Palette& colors = Theme::Colors();
        if (!emphasized)
            return colors.unfocusedSelection;
        return Environment().insideSheet ? colors.sheetSelection : colors.selection;
    }

    static void DrawPlatter(ImDrawList* draw, const ImRect& row, bool emphasized) {
        const Metrics::SidebarMetrics& metrics = Metrics::Sidebar();
        const ImRect platter(row.Min.x + Px(metrics.platterInset), row.Min.y, row.Max.x - Px(metrics.platterInset), row.Max.y);
        Draw::FillRoundedRect(draw, platter, CornerRadii(Px(metrics.platterRadius)), PlatterColor(emphasized));
    }

    // A click on a row gives its list the keyboard focus.
    static void TakeFocus() {
        Interaction::FocusedList() = Environment().focusScope;
    }

    void List(const std::function<void()>& content) {
        const ImGuiID id = Layout::NextViewId();
        SidebarListState& state = State::Get<SidebarListState>(id);
        state.hasGroups = false;
        const ImGuiID focused = Interaction::FocusedList();
        ScrollViewOptions options;
        options.padding.bottom = Metrics::Sidebar().groupGap;
        options.role = Layout::Role::Sidebar;
        WithEnvironment([&](EnvironmentValues& environment) {
            environment.focusScope = id;
            environment.isFocused = focused == id || focused == 0;
            environment.outlineLevel = 0;
            environment.outlineColumn = state.outline;
            environment.outlineExpansion = nullptr;
        }, [&] { ScrollView(options, content); });
        state.outline = state.hasGroups;
    }

    // The matches of query in text, ASCII case aside, in color.
    static std::vector<Typography::ColorSpan> Matches(std::string_view text, std::string_view query, Rgba color) {
        std::vector<Typography::ColorSpan> spans;
        const auto lower = [](char c) { return char(std::tolower((unsigned char)c)); };
        for (size_t at = 0; !query.empty() && at + query.size() <= text.size();) {
            size_t i = 0;
            while (i < query.size() && lower(text[at + i]) == lower(query[i]))
                ++i;
            if (i == query.size()) {
                spans.push_back({at, at + query.size(), color});
                at += query.size();
            } else {
                ++at;
            }
        }
        return spans;
    }

    // A sidebar row: the platter, the chevron of a group's row, the icon, the title with its description and the badge.
    // A group's own row turns the group with its chevron; selection may be null for a row that only turns its group.
    static bool SidebarRow(const char* title, int* selection, int tag, const NavigationLinkOptions& options) {
        const Metrics::SidebarMetrics& metrics = Metrics::Sidebar();
        const bool described = !options.description.empty();
        // A search result without an icon takes the titles' column and wraps there.
        const bool result = !options.highlight.empty();
        const Font font = Font::Style(TextStyle::Body);
        const std::string_view text = Interaction::VisibleLabel(title);
        const float offset = Px(metrics.outlineIndent * float(Environment().outlineLevel) + (Environment().outlineColumn ? metrics.outlineColumn : 0.0f));
        const float text_left = offset + Px(!options.icon.IsEmpty() || result ? metrics.labelX : metrics.iconX);
        const float text_width = Layout::FullProposal().x - text_left - Px(metrics.platterInset);
        const float wrapped = result ? Pt(Typography::Measure(font, text, text_width).y) + 2.0f * metrics.wrappedRowPadding : 0.0f;
        Layout::Placement placement;
        placement.size = ImVec2(Layout::FullProposal().x, Px(ImMax(described ? metrics.subtitleRowHeight : metrics.rowHeight, wrapped)));
        placement.ignoresChildInsets = true;
        const ImRect row = Layout::Place(placement);
        if (Layout::IsMeasuring())
            return false;

        const EnvironmentValues& environment = Environment();
        // The title's line: the first 28 pt, where the chevron and the badge stand.
        const ImRect line(row.Min.x, row.Min.y, row.Max.x, row.Min.y + Px(metrics.rowHeight));
        ImGui::PushID(title);
        // The chevron takes its clicks before the row.
        bool* expansion = environment.outlineExpansion;
        const ImVec2 chevron(row.Min.x + Px(metrics.outlineChevronX + metrics.outlineIndent * float(environment.outlineLevel)), line.GetCenter().y);
        if (expansion && selection) {
            const ImRect slot(chevron.x - Px(metrics.outlineColumn), line.Min.y, chevron.x + Px(metrics.outlineColumn), line.Max.y);
            if (Interaction::Button(ImGui::GetID("##chevron"), slot).pressed)
                *expansion = !*expansion;
        }
        const bool pressed = Interaction::Button(ImGui::GetID("##row"), row, ImGuiButtonFlags_PressedOnClick, ImGuiItemFlags_NoTabStop).pressed;
        if (pressed) {
            TakeFocus();
            if (selection)
                *selection = tag;
            else if (expansion)
                *expansion = !*expansion;
        }
        const bool selected = selection && *selection == tag;
        const bool emphasized = Emphasized(selected);

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        if (selected)
            DrawPlatter(draw, row, emphasized);
        if (expansion) {
            const float turn = DisclosureTurn(ImGui::GetID("##chevron"), *expansion);
            DrawOutlineChevron(draw, chevron, Font::System(metrics.outlineChevronSize, FontWeight::Bold), turn, emphasized ? colors.selectedContent : colors.sidebarSecondaryLabel);
        }

        const Icon& icon = options.icon;
        float text_x = row.Min.x + text_left;
        if (!icon.IsEmpty()) {
            // A plate or a picture beside a description grows to the large icon (kit's Item with Image).
            const bool large = described && (icon.plate != IconPlate::None || !icon.image.IsEmpty());
            const float half = Px(large ? metrics.largeIcon : metrics.iconSize) * 0.5f;
            const float center = row.Min.x + offset + Px(large ? metrics.largeIconCenter : metrics.iconX + metrics.iconSize * 0.5f);
            const ImRect frame(center - half, row.GetCenter().y - half, center + half, row.GetCenter().y + half);
            // A symbol without a plate turns white with the emphasized row's text.
            Icon shown = icon;
            if (emphasized && icon.plate == IconPlate::None && icon.image.IsEmpty())
                shown.color = colors.selectedContent;
            // The icon fades as a whole over the sidebar material or the platter.
            const float alpha = ImGui::GetStyle().Alpha;
            const float opacity = ImLerp(metrics.inactiveIconOpacity, 1.0f, environment.KeyAmount());
            const Rgba ground = environment.insideSheet ? colors.sheetSidebarBackground : colors.sidebarBackground;
            const Rgba backdrop = selected ? Blend::Over(PlatterColor(emphasized).Opacity(alpha), ground) : ground;
            Draw::FadedGroup(draw, backdrop, alpha * opacity, [&] { DrawIcon(draw, frame, shown); });
            text_x = row.Min.x + offset + Px(large ? metrics.largeLabelX : metrics.labelX);
        }

        // Without an icon the title takes the icon's place (Software Update Available, Sharing @2x).
        const float text_end = row.Max.x - Px(metrics.platterInset);
        if (result) {
            const Rgba matched = emphasized ? colors.selectedContent : colors.sidebarLabel;
            const Rgba rest = emphasized ? colors.selectedSecondaryContent : colors.sidebarSecondaryLabel;
            const float lines = Typography::Measure(font, text, text_end - text_x).y;
            const float top = row.GetCenter().y - lines * 0.5f;
            Typography::DrawWrapped(draw, font, ImRect(text_x, top, text_end, top + lines), rest, text, Matches(text, options.highlight, matched));
        } else if (described) {
            const Font small = Font::Style(TextStyle::Subheadline);
            const float title_top = row.Min.y + Px(metrics.subtitleTitleTop);
            const float subtitle_top = row.Min.y + Px(metrics.subtitleTop);
            Typography::Draw(draw, font, ImRect(text_x, title_top, text_end, title_top + Px(font.lineHeight)), emphasized ? colors.selectedContent : colors.sidebarLabel, text);
            Typography::Draw(draw, small, ImRect(text_x, subtitle_top, text_end, subtitle_top + Px(small.lineHeight)), emphasized ? colors.selectedSecondaryContent : colors.sidebarSecondaryLabel, options.description);
        } else {
            Typography::Draw(draw, font, ImRect(text_x, row.Min.y, text_end, row.Max.y), emphasized ? colors.selectedContent : colors.sidebarLabel, text);
        }

        if (options.badge > 0) {
            const std::string count = std::to_string(options.badge);
            if (environment.badgeProminence == BadgeProminence::Increased) {
                const float right = row.Max.x - Px(metrics.badgeTrailing);
                const float half = Px(metrics.badge) * 0.5f;
                const ImRect disc(right - 2.0f * half, line.GetCenter().y - half, right, line.GetCenter().y + half);
                Draw::FillRoundedRect(draw, disc, CornerRadii(half), Theme::SystemRed(), CornerStyle::Circular);
                Typography::Draw(draw, Font::System(metrics.badgeFontSize, FontWeight::Medium), disc, Rgba::White(1.0f), count, TextAlignment::Center);
            } else {
                const float right = row.Max.x - Px(metrics.countTrailing);
                Typography::Draw(draw, font, ImRect(right - Typography::Width(font, count), line.Min.y, right, line.Max.y), emphasized ? colors.selectedContent : colors.sidebarTertiaryLabel, count);
            }
        }
        ImGui::PopID();
        return pressed;
    }

    bool NavigationLink(const char* title, int* selection, int tag, const NavigationLinkOptions& options) {
        return SidebarRow(title, selection, tag, options);
    }

    bool NavigationLink(int* selection, int tag, const std::function<void()>& label) {
        const ImGuiID id = Layout::NextViewId();
        const bool selected = *selection == tag;
        const bool emphasized = Emphasized(selected);
        bool pressed = false;
        // The platter under the content needs the row's final frame.
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Overlay;
        spec.fillWidth = true;
        Layout::ContainerBehind(spec, [&] {
            WithEnvironment([&](EnvironmentValues& environment) {
                if (emphasized)
                    environment.backgroundProminence = BackgroundProminence::Increased;
            }, label);
        }, [&](const Layout::ContainerFrame& frame) {
            pressed = Interaction::Button(id, frame.rect, ImGuiButtonFlags_PressedOnClick, ImGuiItemFlags_NoTabStop).pressed;
            if (selected)
                DrawPlatter(ImGui::GetWindowDrawList(), frame.rect, emphasized);
        });
        if (pressed) {
            TakeFocus();
            *selection = tag;
        }
        return pressed;
    }

    // Marks the list as an outline and draws content a level deeper while expanded.
    static void OutlineChildren(bool expanded, const std::function<void()>& content) {
        State::Get<SidebarListState>(Environment().focusScope).hasGroups = true;
        if (!expanded)
            return;
        WithEnvironment([](EnvironmentValues& environment) {
            ++environment.outlineLevel;
            environment.outlineExpansion = nullptr;
        }, content);
    }

    void SidebarDisclosureGroup(const char* title, bool* expanded, const std::function<void()>& content) {
        WithEnvironment([&](EnvironmentValues& environment) { environment.outlineExpansion = expanded; }, [&] { SidebarRow(title, nullptr, 0, {}); });
        OutlineChildren(*expanded, content);
    }

    void SidebarDisclosureGroup(bool* expanded, const std::function<void()>& label, const std::function<void()>& content) {
        WithEnvironment([&](EnvironmentValues& environment) { environment.outlineExpansion = expanded; }, label);
        OutlineChildren(*expanded, content);
    }

    bool SidebarSearchField(std::string* text) {
        const Metrics::SidebarMetrics& metrics = Metrics::Sidebar();
        const ImVec2 offered = Layout::FullProposal();
        const ImRect slot = Layout::Place(ImVec2(offered.x, Px(metrics.searchTop + metrics.searchHeight + metrics.searchBottom)));
        if (Layout::IsMeasuring())
            return false;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const float top = slot.Min.y + Px(metrics.searchTop);
        const ImRect field(slot.Min.x + Px(metrics.searchInset), top, slot.Max.x - Px(metrics.searchInset), top + Px(metrics.searchHeight));
        const CornerRadii radii(Px(metrics.searchRadius));
        Draw::FillRoundedRect(draw, field, radii, colors.searchField);
        Draw::StrokeRoundedRect(draw, field, radii, colors.searchFieldBorder, Px(0.5f), StrokeAlignment::Inside);

        const Font font = Font::Style(TextStyle::Body);
        const ImRect symbol(field.Min.x + Px(metrics.searchSymbolX), field.Min.y, field.Min.x + Px(metrics.searchTextX - 4.0f), field.Max.y);
        Typography::DrawSymbol(draw, Symbols::Magnifyingglass, font.Weight(FontWeight::Medium), symbol, colors.label, TextAlignment::Leading);

        // With text the field ends in its clear button, in the label color like the magnifier.
        bool cleared = false;
        float text_end = field.Max.x - Px(metrics.searchTextTrailing);
        if (!text->empty()) {
            const float center = field.Max.x - Px(metrics.searchClearCenter);
            const float half = Px(metrics.searchClearSize) * 0.5f;
            const ImRect clear(center - half, field.GetCenter().y - half, center + half, field.GetCenter().y + half);
            cleared = FieldClearButton(ImGui::GetID("##SidebarSearchClear"), clear, colors.label);
            if (cleared)
                text->clear();
            text_end = clear.Min.x - Px(metrics.searchClearGap);
        }
        const ImRect area(field.Min.x + Px(metrics.searchTextX), field.Min.y, text_end, field.Max.y);
        return TextInput::Edit(ImGui::GetID("##SidebarSearch"), area, text, {.color = colors.label, .placeholder = "Search"}).changed || cleared;
    }

    bool SidebarAccount(const char* name, const char* subtitle, const SidebarAccountOptions& options) {
        const Metrics::SidebarMetrics& metrics = Metrics::Sidebar();
        const ImRect slot = Layout::Place(ImVec2(Layout::FullProposal().x, Px(metrics.accountTop + metrics.profileHeight)));
        if (Layout::IsMeasuring())
            return false;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const ImRect block(slot.Min.x, slot.Min.y + Px(metrics.accountTop), slot.Max.x, slot.Max.y);
        const bool pressed = Interaction::Button(ImGui::GetID(name), block, ImGuiButtonFlags_PressedOnClick).pressed;
        if (pressed)
            TakeFocus();
        const bool emphasized = Emphasized(options.selected);
        if (options.selected)
            DrawPlatter(draw, ImRect(block.Min.x, block.Min.y - Px(metrics.accountPlatterRise), block.Max.x, block.Max.y), emphasized);
        // The avatar starts where the rows' icons do.
        const ImVec2 center(block.Min.x + Px(metrics.iconX + metrics.avatar * 0.5f), block.GetCenter().y - Px(metrics.accountRaise));
        const float radius = Px(metrics.avatar) * 0.5f;
        DrawAvatar(draw, ImRect(center - ImVec2(radius, radius), center + ImVec2(radius, radius)), options.initials, options.picture);

        const Font title_font = Font::Style(TextStyle::Body).Weight(options.nameWeight);
        const Font subtitle_font = Font::Style(TextStyle::Subheadline);
        const float text_height = Px(title_font.lineHeight + metrics.accountLineSpacing + subtitle_font.lineHeight);
        const float top = center.y - text_height * 0.5f;
        const float left = block.Min.x + Px(metrics.accountTextX);
        const float subtitle_top = top + Px(title_font.lineHeight + metrics.accountLineSpacing);
        Typography::Draw(draw, title_font, ImRect(left, top, block.Max.x, top + Px(title_font.lineHeight)), emphasized ? colors.selectedContent : colors.sidebarLabel, name);
        Typography::Draw(draw, subtitle_font, ImRect(left, subtitle_top, block.Max.x, subtitle_top + Px(subtitle_font.lineHeight)), emphasized ? colors.selectedSecondaryContent : colors.sidebarSecondaryLabel, subtitle);
        return pressed;
    }
} // namespace Cupertino
