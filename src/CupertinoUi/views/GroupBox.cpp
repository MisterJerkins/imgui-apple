#include "GroupBox.h"

#include "controls/Picker.h"
#include "controls/Text.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Metrics.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"
#include "layout/Stacks.h"

namespace Cupertino {
    // Views/Boxes/Group Box: the quaternary fill with a one-point line of it again inside the edge.
    static void DrawBox(ImDrawList* draw, const ImRect& rect) {
        const Palette& colors = Theme::Colors();
        const CornerRadii radii(Px(Metrics::GroupBox().radius));
        Draw::FillRoundedRect(draw, rect, radii, colors.quaternaryFill);
        Draw::StrokeRoundedRect(draw, rect, radii, colors.quaternaryFill, Px(1.0f), StrokeAlignment::Inside);
    }

    // The content as wide as offered, inset in the box, its views one under another as in a VStack.
    static void BoxContent(const std::function<void()>& content) {
        Frame({.maxWidth = Infinity, .alignment = {HorizontalAlignment::Leading, VerticalAlignment::Top}}, [&] {
            Padding(Metrics::GroupBox().padding, [&] { VStack({.alignment = HorizontalAlignment::Leading}, content); });
        });
    }

    void GroupBox(const std::function<void()>& content) {
        Background(DrawBox, [&] { BoxContent(content); });
    }

    // The label sits on the box with its baseline 3 pt above the edge, as far in as the content (Chess sheet @2x).
    void GroupBox(std::string_view label, const std::function<void()>& content) {
        VStack({.alignment = HorizontalAlignment::Leading, .spacing = 0.0f}, [&] {
            Padding(EdgeInsets{0.0f, Metrics::GroupBox().padding, 0.0f, 0.0f}, [&] { Text(label); });
            GroupBox(content);
        });
    }

    void TabView(int* selection, std::span<const char* const> tabs, const std::function<void(int tab)>& content) {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImRect well;
        Layout::ContainerSpec spec;
        spec.alignment = Alignment{HorizontalAlignment::Center, VerticalAlignment::Top};
        spec.fillWidth = true;
        ImGui::PushID(selection);
        Layout::ContainerBehind(spec, [&] {
            // Equal tabs as wide as the widest title and its padding, even in a form, where an unlabeled segmented control
            // would fill the row.
            const float widest = Pt(Typography::WidestWidth(Font::Style(TextStyle::Body), tabs));
            const float width = float(tabs.size()) * (widest + 2.0f * Metrics::GroupBox().tabPadding);
            Overlay([&](ImDrawList*, const ImRect& rect) { well = rect; }, [&] { Picker("##tabs", selection, tabs, {.style = PickerStyle::Segmented, .width = width}); });
            // Each tab is a view of its own that keeps its state while another shows, as an NSTabView's tabs do.
            BoxContent([&] {
                ImGui::PushID(*selection);
                content(*selection);
                ImGui::PopID();
            });
        }, [&](const Layout::ContainerFrame& frame) {
            // The box starts at the middle of the tabs and is cut out under their well, as the kit masks it.
            const ImRect box(frame.rect.Min.x, well.GetCenter().y, frame.rect.Max.x, frame.rect.Max.y);
            const ImRect parts[] = {
                ImRect(box.Min.x, box.Min.y, well.Min.x, box.Max.y),
                ImRect(well.Max.x, box.Min.y, box.Max.x, box.Max.y),
                ImRect(well.Min.x, well.Max.y, well.Max.x, box.Max.y),
            };
            for (const ImRect& part : parts) {
                draw->PushClipRect(part.Min, part.Max, true);
                DrawBox(draw, box);
                draw->PopClipRect();
            }
        });
        ImGui::PopID();
    }

    void TabView(int* selection, std::initializer_list<const char*> tabs, const std::function<void(int tab)>& content) {
        TabView(selection, std::span<const char* const>(tabs.begin(), tabs.size()), content);
    }
} // namespace Cupertino
