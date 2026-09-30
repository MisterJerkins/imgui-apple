#include "Form.h"

#include "controls/Text.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/MenuContent.h"
#include "core/Metrics.h"
#include "core/Theme.h"
#include "layout/Layout.h"
#include "layout/ScrollView.h"
#include "layout/Stacks.h"

namespace Cupertino {
    void Form(const std::function<void()>& content) {
        const Metrics::FormMetrics& metrics = Metrics::Form();
        const bool grows_sheet = Environment().insideSheet && Environment().sheetGrows;
        const bool under_bar = Environment().scrollEdge != 0;
        WithEnvironment([](EnvironmentValues& environment) { environment.insideForm = true; }, [&] {
            // In a sheet the form sizes the sheet: margins all around (Battery Options @2x) and no scrolling.
            if (grows_sheet) {
                Layout::ContainerSpec spec;
                spec.spacing = metrics.sectionSpacing;
                spec.padding = EdgeInsets::All(metrics.margin);
                spec.fillWidth = true;
                Layout::Container(spec, content);
                return;
            }
            // Under a toolbar the first section starts right at its edge; elsewhere the margin goes all around (Wi-Fi
            // details sheet @2x).
            ScrollViewOptions options;
            options.padding = EdgeInsets{under_bar ? metrics.topMargin : metrics.margin, metrics.margin, metrics.margin, metrics.margin};
            options.spacing = metrics.sectionSpacing;
            ScrollView(options, content);
        });
    }

    void DrawSectionBox(ImDrawList* draw, const ImRect& rect) {
        Draw::FillRoundedRect(draw, rect, CornerRadii(Px(Metrics::Form().sectionRadius)), Theme::Colors().sectionBackground, CornerStyle::Circular);
        DrawSectionBorder(draw, rect);
    }

    void DrawInsetBox(ImDrawList* draw, const ImRect& rect) {
        Draw::FillRoundedRect(draw, rect, CornerRadii(Px(Metrics::Form().sectionRadius)), Theme::Colors().quaternaryFill, CornerStyle::Circular);
        DrawSectionBorder(draw, rect);
    }

    // A tile keeps one point of the separator color around its fill (Saved to iCloud @2x: 212 around 221).
    void DrawTileBox(ImDrawList* draw, const ImRect& rect) {
        const CornerRadii radii(Px(Metrics::Form().sectionRadius));
        Draw::FillRoundedRect(draw, rect, radii, Theme::Colors().tertiaryFill, CornerStyle::Circular);
        Draw::StrokeRoundedRect(draw, rect, radii, Theme::Colors().rowSeparator, Px(1.0f), StrokeAlignment::Inside, CornerStyle::Circular);
    }

    void DrawSectionBorder(ImDrawList* draw, const ImRect& rect) {
        const Palette& colors = Theme::Colors();
        const CornerRadii radii(Px(Metrics::Form().sectionRadius));
        const float half = Px(0.5f);
        Draw::StrokeRoundedRect(draw, rect, radii, colors.sectionBorder, half, StrokeAlignment::Inside, CornerStyle::Circular);
        const ImRect inner(rect.Min + ImVec2(half, half), rect.Max - ImVec2(half, half));
        Draw::StrokeRoundedRect(draw, inner, radii.Offset(-half), colors.sectionInnerBorder, half, StrokeAlignment::Inside, CornerStyle::Circular);
    }

    // One rounded shape for the whole section: the box and inset separators between rows, edge to edge over a view that
    // asks for it.
    static void DrawSectionBackground(ImDrawList* draw, const Layout::ContainerFrame& frame) {
        const Metrics::FormMetrics& metrics = Metrics::Form();
        const Palette& colors = Theme::Colors();
        DrawSectionBox(draw, frame.rect);
        // Separators fill the one-point gaps between rows.
        const float thickness = Px(metrics.separatorThickness);
        for (int i = 0; i + 1 < frame.slots.Size; ++i) {
            const float y = frame.slots[i].Max.y;
            const float inset = frame.fullWidthSeparators[i + 1] ? 0.0f : frame.separatorLeadings[i] >= 0.0f ? frame.separatorLeadings[i] : Px(metrics.separatorInset);
            Draw::HorizontalLine(draw, frame.rect.Min.x + inset, frame.rect.Max.x - inset, y, thickness, colors.rowSeparator);
        }
    }

    static void FormSection(const std::function<void()>& content) {
        const Metrics::FormMetrics& metrics = Metrics::Form();
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Vertical;
        // Rows are at least 36 pt tall and one point apart; the separators are drawn in those gaps.
        spec.spacing = metrics.separatorThickness;
        spec.childInsets = EdgeInsets::All(metrics.rowInset);
        spec.minChildLength = metrics.rowHeight;
        spec.allowsOverhang = true;
        spec.fillWidth = true;
        // Content narrower than a row (a line of text) starts at the row inset, as in SwiftUI forms.
        spec.alignment = Alignment{HorizontalAlignment::Leading, VerticalAlignment::Center};
        spec.role = Layout::Role::FormSection;

        // The background is drawn after the rows, whose height is known only then, below them.
        Layout::ContainerBehind(spec, content, [&](const Layout::ContainerFrame& frame) { DrawSectionBackground(ImGui::GetWindowDrawList(), frame); });
    }

    // Sidebar groups have no background; groups are separated by a gap. A group's header stands over its rows, the first
    // group's right under the toolbar (Finder @2x).
    static void SidebarGroup(const char* header, const std::function<void()>& content) {
        const Metrics::SidebarMetrics& metrics = Metrics::Sidebar();
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Vertical;
        spec.padding.top = header && Layout::ChildIndex() == 0 ? 0.0f : metrics.groupGap;
        spec.fillWidth = true;
        spec.role = Layout::Role::Sidebar;
        Layout::Container(spec, [&] {
            if (header) {
                const ImRect row = Layout::Place(Layout::Placement{.size = ImVec2(Layout::FullProposal().x, Px(metrics.headerHeight))});
                if (!Layout::IsMeasuring()) {
                    const Font font = Font::System(metrics.headerSize, FontWeight::Semibold);
                    const float top = row.Min.y + Px(metrics.headerBaseline) - Typography::Baseline(font);
                    Typography::Draw(ImGui::GetWindowDrawList(), font, ImRect(row.Min.x + Px(metrics.headerX), top, row.Max.x, top + Px(font.lineHeight)), Theme::Colors().sidebarTertiaryLabel, header);
                }
            }
            content();
        });
    }

    void Section(const SectionOptions& options, const std::function<void()>& content) {
        // In a menu a section is a group of items after a separator, under its header if it has one.
        if (MenuContent::IsCollecting()) {
            MenuContent::BeginSection(options.header);
            content();
            MenuContent::EndSection();
            return;
        }
        if (Layout::ParentRole() == Layout::Role::Sidebar) {
            SidebarGroup(options.header, content);
            return;
        }
        const Metrics::FormMetrics& metrics = Metrics::Form();
        if (!options.header && !options.footer) {
            FormSection(content);
            return;
        }
        const float header_top = Layout::ChildIndex() == 0 ? metrics.firstHeaderTop : metrics.headerTop - metrics.sectionSpacing;
        VStack({.alignment = HorizontalAlignment::Leading, .spacing = 0.0f}, [&] {
            if (options.header) {
                Padding(EdgeInsets{header_top, metrics.rowInset, metrics.headerSpacing, metrics.rowInset}, [&] {
                    VStack({.alignment = HorizontalAlignment::Leading, .spacing = metrics.headerDescriptionSpacing}, [&] {
                        HStack([&] {
                            Text(options.header, {.font = Font::Style(TextStyle::Body).Weight(FontWeight::Semibold)});
                            if (options.headerAccessory) {
                                Spacer();
                                options.headerAccessory();
                            }
                        });
                        if (options.description)
                            Text(options.description, {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary, .wraps = true});
                    });
                });
            }
            if (options.boxed) {
                FormSection(content);
            } else {
                Layout::ContainerSpec spec;
                spec.fillWidth = true;
                Layout::Container(spec, content);
            }
            if (options.footer) {
                Padding(EdgeInsets{6.0f, metrics.rowInset, 0.0f, metrics.rowInset}, [&] {
                    Text(options.footer, {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary, .wraps = true});
                });
            }
        });
    }

    void Section(const std::function<void()>& content) {
        Section(SectionOptions{}, content);
    }
} // namespace Cupertino
