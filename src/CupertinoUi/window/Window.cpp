#include "Window.h"

#include "controls/Button.h"
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
#include "layout/ScrollView.h"
#include "layout/SplitView.h"

#include "imgui_internal.h"

#include <string>
#include <vector>

namespace Cupertino {
    struct WindowPresentation {
        // The frame the window was last drawn in: sheets attach only to a window being drawn.
        int frame = -1;
        // How far the attached sheet is presented, 0 ... 1. The sheet sets it while it draws; the window reads it after
        // its content for the dim, and in the next frame to block the content.
        float sheet = 0.0f;
        // The window's own environment and opacity, which the sheet's content and the window background keep.
        EnvironmentValues environment;
        float alpha = 1.0f;
    };

    // The size the window was given and the size the user has dragged it to, in points; which edges are being dragged,
    // and where the window was when a drag of its chrome began.
    struct WindowSizing {
        ImVec2 given;
        ImVec2 size;
        bool draggingRight = false;
        bool draggingBottom = false;
        bool moving = false;
        ImVec2 moveFrom;
    };

    // Resizing by the bottom and right edges along the axes the limits allow: a band inside each edge takes the press
    // before the content does (scroll views and rows reach the edges), the drag follows the pointer outside the window,
    // and the new size applies from the next frame.
    static void ResizeByEdges(WindowSizing& sizing, ImGuiWindow* window, const WindowOptions& options) {
        const ImRect rect = window->Rect();
        const bool along_x = options.maxSize.x > options.minSize.x;
        const bool along_y = options.maxSize.y > options.minSize.y;
        const ImGuiIO& io = ImGui::GetIO();
        const ImGuiID id = window->GetID("##resize");
        const float grip = Px(Metrics::Window().resizeGrip);
        const bool hovered = Interaction::PointerOver(rect);
        const bool over_right = along_x && hovered && io.MousePos.x >= rect.Max.x - grip;
        const bool over_bottom = along_y && hovered && io.MousePos.y >= rect.Max.y - grip;
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && (over_right || over_bottom)) {
            sizing.draggingRight = over_right;
            sizing.draggingBottom = over_bottom;
            ImGui::SetActiveID(id, window);
        }
        const bool dragging = sizing.draggingRight || sizing.draggingBottom;
        if (dragging && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            sizing.draggingRight = sizing.draggingBottom = false;
            if (ImGui::GetActiveID() == id)
                ImGui::ClearActiveID();
        } else if (dragging) {
            ImGui::KeepAliveID(id);
        }
        if (sizing.draggingRight)
            sizing.size.x = ImClamp(Pt(io.MousePos.x - rect.Min.x), options.minSize.x, options.maxSize.x);
        if (sizing.draggingBottom)
            sizing.size.y = ImClamp(Pt(io.MousePos.y - rect.Min.y), options.minSize.y, options.maxSize.y);
        const bool right = over_right || sizing.draggingRight;
        const bool bottom = over_bottom || sizing.draggingBottom;
        if (right || bottom)
            ImGui::SetMouseCursor(right && bottom ? ImGuiMouseCursor_ResizeNWSE : right ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS);
    }

    // The presentation of the Cupertino window being drawn, or null outside one. The window is only read: outside a
    // window ImGui would otherwise show its fallback Debug window.
    static WindowPresentation* CurrentPresentation() {
        WindowPresentation& presentation = State::Get<WindowPresentation>(ImGui::GetCurrentWindowRead()->RootWindow->ID);
        return presentation.frame == ImGui::GetFrameCount() ? &presentation : nullptr;
    }

    SheetHost AttachSheet(float presentation) {
        const ImRect frame = ImGui::GetCurrentWindowRead()->RootWindow->Rect();
        WindowPresentation* window = CurrentPresentation();
        if (!window)
            return {frame, Environment(), ImGui::GetStyle().Alpha};
        window->sheet = ImMax(window->sheet, presentation);
        return {frame, window->environment, window->alpha, true};
    }

    void InSheet(const SheetHost& host, float opacity, const std::function<void(EnvironmentValues&)>& modify, const std::function<void()>& content) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, opacity);
        WithEnvironment([&](EnvironmentValues& environment) {
            environment = host.environment;
            if (modify)
                modify(environment);
        }, content);
        ImGui::PopStyleVar();
        ImGui::PopItemFlag();
    }

    // Hover glyphs of the traffic lights: dark shades of each button color.
    static void DrawTrafficGlyph(ImDrawList* draw, int index, const ImRect& light) {
        const ImVec2 center = light.GetCenter();
        if (index == 2) {
            // Zoom shows two small triangles pointing away from the center.
            const Rgba color = Rgba::Hex(0x006500);
            const float a = Px(3.0f);
            const float b = Px(1.0f);
            draw->AddTriangleFilled(center + ImVec2(-a, -a), center + ImVec2(b, -a), center + ImVec2(-a, b), color.Packed());
            draw->AddTriangleFilled(center + ImVec2(a, a), center + ImVec2(-b, a), center + ImVec2(a, -b), color.Packed());
            return;
        }
        const unsigned symbol = index == 0 ? Symbols::Xmark : Symbols::Minus;
        const Rgba color = index == 0 ? Rgba::Hex(0x4D0000) : Rgba::Hex(0x995700);
        Typography::DrawSymbol(draw, symbol, Font::System(8.0f, FontWeight::Bold), light, color);
    }

    // Close, minimize and zoom at (20, 20) of a window with a 52 pt toolbar; returns true when close is clicked.
    static bool TrafficLights(ImDrawList* draw, ImVec2 origin, const WindowOptions& options, bool closable) {
        const Metrics::WindowMetrics& metrics = Metrics::Window();
        const Palette& colors = Theme::Colors();
        const float size = Px(metrics.trafficLight);
        const float pitch = Px(metrics.trafficPitch);
        const ImRect group(origin, origin + ImVec2(pitch * 2.0f + size, size));
        const bool group_hovered = ImGui::IsWindowHovered() && group.Contains(ImGui::GetIO().MousePos);
        const bool enabled[3] = {closable && options.closable, options.minimizable, options.zoomable};
        const Rgba fills[3] = {colors.trafficClose, colors.trafficMinimize, colors.trafficZoom};
        const bool active = Environment().controlActiveState != ControlActiveState::Inactive;

        bool close_clicked = false;
        ImGui::PushID("##TrafficLights");
        for (int i = 0; i < 3; ++i) {
            const ImRect light(origin + ImVec2(pitch * float(i), 0.0f), origin + ImVec2(pitch * float(i) + size, size));
            const Interaction::Response response = Interaction::Button(ImGui::GetID(i), light, 0, ImGuiItemFlags_NoNav);
            const bool lit = enabled[i] && (active || group_hovered);
            const CornerRadii radii(size * 0.5f);
            Rgba fill = lit ? fills[i] : active ? colors.trafficDisabled : colors.trafficInactive;
            if (lit && response.held && response.hovered)
                fill = Blend::Over(Rgba::Black(0.25f), fill);
            Draw::FillRoundedRect(draw, light, radii, fill, CornerStyle::Circular);
            Draw::StrokeRoundedRect(draw, light, radii, colors.trafficBorder, Px(0.5f), StrokeAlignment::Inside, CornerStyle::Circular);
            if (group_hovered && enabled[i])
                DrawTrafficGlyph(draw, i, light);
            if (i == 0 && response.pressed && closable)
                close_clicked = true;
        }
        ImGui::PopID();
        return close_clicked;
    }

    // Scroll views draw after their window, above its dim, so their colors take the dim instead of being covered by it.
    static void DimChildWindows(ImGuiWindow* window, Rgba dim) {
        for (ImGuiWindow* child : window->DC.ChildWindows) {
            Draw::TintVertices(child->DrawList, 0, dim, dim.a);
            DimChildWindows(child, dim);
        }
    }

    void DrawWindowEdge(ImDrawList* draw, const ImRect& rect, const CornerRadii& radii, CornerStyle style) {
        const Palette& colors = Theme::Colors();
        Draw::StrokeRoundedRect(draw, rect, radii, colors.windowRim, Px(Environment().IsDark() ? 1.0f : 0.5f), StrokeAlignment::Inside, style);
        for (int row = 0; row < 2; ++row) {
            const float top = rect.Min.y + Px(0.5f * float(row));
            draw->PushClipRect(ImVec2(rect.Min.x, top), ImVec2(rect.Max.x, top + Px(0.5f)), true);
            Draw::FillRoundedRect(draw, rect, radii, row == 0 ? colors.windowHighlight : colors.windowHighlight.Opacity(0.4f), style);
            draw->PopClipRect();
        }
    }

    float ToolbarHeight() {
        const Metrics::WindowMetrics& metrics = Metrics::Window();
        return Environment().windowToolbarStyle == WindowToolbarStyle::UnifiedCompact ? metrics.compactToolbarHeight : metrics.toolbarHeight;
    }

    // How far a compact toolbar's contents move up from where the kit places them in the 52 pt bar.
    static float ToolbarShift() {
        return Px((ToolbarHeight() - Metrics::Window().toolbarHeight) * 0.5f);
    }

    void Window(const char* title, bool* open, const WindowOptions& options, const std::function<void()>& content) {
        if (open && !*open)
            return;
        const Metrics::WindowMetrics& metrics = Metrics::Window();
        // ImGui truncates window positions, so they are rounded to device pixels first.
        ImGui::SetNextWindowPos(Draw::Snap(Px(options.position)), ImGuiCond_FirstUseEver);
        // The size the user dragged the window to holds until the size given changes.
        WindowSizing& sizing = State::Get<WindowSizing>(ImHashStr(title));
        if (sizing.given.x != options.size.x || sizing.given.y != options.size.y) {
            sizing.given = options.size;
            sizing.size = options.size;
        }
        ImGui::SetNextWindowSize(Px(sizing.size), ImGuiCond_Always);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        // A window behind its sheet stays under it when clicked.
        WindowPresentation& presentation = State::Get<WindowPresentation>(ImHashStr(title));
        const float blocked = presentation.sheet;
        // ImGui would move a window without a title bar by any empty spot; the window moves itself, by its chrome only.
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground;
        if (blocked > 0.0f)
            flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
        ImGui::Begin(title, nullptr, flags);
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (options.floating)
            ImGui::BringWindowToDisplayFront(window);
        ResizeByEdges(sizing, window, options);
        IM_ASSERT(window->ID == ImHashStr(title));
        presentation = {ImGui::GetFrameCount(), 0.0f, Environment(), ImGui::GetStyle().Alpha};

        ImDrawList* draw = window->DrawList;
        const ImRect rect = window->Rect();
        const CornerRadii radii(Px(metrics.radius));
        const bool plain = options.style == WindowStyle::Plain;
        const bool titled = options.style == WindowStyle::TitleBar;
        draw->PushClipRectFullScreen();
        Draw::DropShadows(draw, rect, radii, Theme::WindowShadows());
        if (!plain)
            Draw::StrokeRoundedRect(draw, rect, radii, Theme::Colors().windowOutline, Px(0.5f), StrokeAlignment::Outside);
        draw->PopClipRect();
        Draw::FillRoundedRect(draw, rect, radii, Theme::Colors().windowBackground);

        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Vertical;
        spec.fillWidth = true;
        spec.fillHeight = true;
        // Behind its sheet the window stays main but is not key: the content fades and takes no input, and its controls turn
        // gray as the sheet comes in.
        WithEnvironment([&](EnvironmentValues& environment) {
            environment.windowToolbarStyle = options.toolbarStyle;
            if (blocked > 0.0f && environment.controlActiveState == ControlActiveState::Key) {
                environment.controlActiveState = ControlActiveState::Active;
                environment.controlActiveAmount = blocked;
            }
        }, [&] {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, presentation.alpha * ImLerp(1.0f, Metrics::Sheet().backgroundOpacity, blocked));
            if (blocked > 0.0f)
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            Layout::Container(spec, [&] {
                if (titled)
                    TitleBar(std::string(title, ImGui::FindRenderedTextEnd(title)).c_str());
                content();
            });
            if (blocked > 0.0f)
                ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        });

        // The dim covers the whole window with its scroll views; the traffic lights stay above it.
        if (presentation.sheet > 0.0f) {
            const Rgba dim = Theme::Colors().sheetDim.Opacity(presentation.sheet);
            Draw::FillRoundedRect(draw, rect, radii, dim);
            DimChildWindows(window, dim);
        }
        // The lights keep their place from the leading edge and center in a compact toolbar (kit's Bars/Toolbar Mono: 20, 13).
        const float toolbar = titled ? metrics.titleBarHeight : options.toolbarStyle == WindowToolbarStyle::UnifiedCompact ? metrics.compactToolbarHeight : metrics.toolbarHeight;
        if (!plain) {
            DrawWindowEdge(draw, rect, radii);
            const bool compact = options.compactTitleBar || titled;
            const float light_x = Px(compact ? metrics.compactLightInset : (metrics.toolbarHeight - metrics.trafficLight) * 0.5f);
            const float light_y = Px(compact ? metrics.compactLightInset : (toolbar - metrics.trafficLight) * 0.5f);
            if (TrafficLights(draw, rect.Min + ImVec2(light_x, light_y), options, presentation.sheet == 0.0f) && open)
                *open = false;
        }

        const ImGuiIO& io = ImGui::GetIO();
        const bool in_chrome = plain || io.MousePos.y < rect.Min.y + Px(toolbar);
        if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && in_chrome && !sizing.draggingRight && !sizing.draggingBottom && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            sizing.moving = true;
            sizing.moveFrom = window->Pos;
        }
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            sizing.moving = false;
        if (sizing.moving)
            ImGui::SetWindowPos(window, Draw::Snap(sizing.moveFrom + io.MousePos - io.MouseClickedPos[ImGuiMouseButton_Left]));
        ImGui::End();
        ImGui::PopStyleVar(3);
    }

    // A title that would cross the traffic lights follows them.
    void DrawTitleBar(ImDrawList* draw, const ImRect& bar, const char* title) {
        const Metrics::TabBarMetrics& metrics = Metrics::TabBar();
        const Metrics::WindowMetrics& window = Metrics::Window();
        const Palette& colors = Theme::Colors();
        const float radius = Px(window.radius);
        Draw::FillRoundedRect(draw, bar, CornerRadii(radius, radius, 0.0f, 0.0f), colors.titleBarBackground);
        Draw::FillRect(draw, ImRect(bar.Min.x, bar.Max.y - Px(window.toolbarLine), bar.Max.x, bar.Max.y), colors.toolbarLine);
        const Font title_font = Font::System(metrics.titleSize, FontWeight::Bold);
        const float lights = Px(window.compactLightInset + 2.0f * window.trafficPitch + window.trafficLight + window.titleAfterLights);
        const float left = ImMax(bar.GetCenter().x - Typography::Width(title_font, title) * 0.5f, bar.Min.x + lights);
        const float title_top = bar.Min.y + Px(metrics.titleBaseline) - Typography::Baseline(title_font);
        Typography::Draw(draw, title_font, ImRect(left, title_top, bar.Max.x, title_top + Px(title_font.lineHeight)), colors.toolbarTitle, title);
    }

    void TitleBar(const char* title) {
        const Metrics::WindowMetrics& metrics = Metrics::Window();
        const ImRect bar = Layout::Place(Layout::Placement{.size = ImVec2(Layout::FullProposal().x, Px(metrics.titleBarHeight + metrics.toolbarLine)), .ignoresChildInsets = true});
        if (!Layout::IsMeasuring())
            DrawTitleBar(ImGui::GetWindowDrawList(), bar, title);
    }

    bool TabBar(int* selection, std::span<const TabBarItem> items) {
        const Metrics::TabBarMetrics& metrics = Metrics::TabBar();
        const ImRect bar = Layout::Place(Layout::Placement{.size = ImVec2(Layout::FullProposal().x, Px(metrics.height)), .ignoresChildInsets = true});
        if (Layout::IsMeasuring() || items.empty())
            return false;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const int selected = ImClamp(*selection, 0, int(items.size()) - 1);
        DrawTitleBar(draw, bar, items[size_t(selected)].title);

        // Cells side by side in the middle of the bar, each as wide as its label and padding, and no narrower than the least.
        const Font label_font = Font::System(metrics.labelSize);
        const Font symbol_font = Font::System(metrics.symbolSize, FontWeight::Medium).ImageScale(SymbolScale::Large);
        std::vector<float> cells;
        float total = 0.0f;
        for (const TabBarItem& item : items) {
            cells.push_back(ImMax(Typography::Width(label_font, item.title) + Px(2.0f * metrics.labelPadding), Px(metrics.minItemWidth)));
            total += cells.back();
        }
        float left = bar.GetCenter().x - total * 0.5f;
        bool changed = false;
        ImGui::PushID(selection);
        for (int i = 0; i < int(items.size()); left += cells[size_t(i)], ++i) {
            const ImRect item(left, bar.Min.y + Px(metrics.itemTop), left + cells[size_t(i)], bar.Min.y + Px(metrics.itemTop + metrics.itemHeight));
            const Interaction::Response response = Interaction::Button(ImGui::GetID(i), item, ImGuiButtonFlags_PressedOnClick);
            if (response.pressed && i != *selection) {
                *selection = i;
                changed = true;
            }
            const bool on = i == selected;
            if (on || (response.held && response.hovered))
                Draw::FillRoundedRect(draw, item, CornerRadii(Px(metrics.backingRadius)), on ? colors.tertiaryFill : colors.quaternaryFill);
            const Rgba color = on ? colors.accent : colors.secondaryLabel;
            const float symbol_top = item.Min.y + Px(metrics.symbolTop);
            Typography::DrawSymbol(draw, items[size_t(i)].symbol, symbol_font, ImRect(item.Min.x, symbol_top, item.Max.x, symbol_top + Px(metrics.symbolLine)), color);
            const float label_top = item.Min.y + Px(metrics.labelTop);
            Typography::Draw(draw, label_font, ImRect(item.Min.x, label_top, item.Max.x, label_top + Px(label_font.lineHeight)), color, items[size_t(i)].title, TextAlignment::Center);
        }
        ImGui::PopID();
        return changed;
    }

    bool TabBar(int* selection, std::initializer_list<TabBarItem> items) {
        return TabBar(selection, std::span<const TabBarItem>(items.begin(), items.size()));
    }

    // A title over a subtitle is 13 pt Bold (Battery and Activity Monitor @2x); alone it is 15 pt Semibold.
    static Font ToolbarTitleFont(bool subtitled) {
        const Metrics::WindowMetrics& metrics = Metrics::Window();
        return subtitled ? Font::System(metrics.subtitledTitleSize, FontWeight::Bold) : Font::System(metrics.titleSize, FontWeight::Semibold);
    }

    // A title alone, centered on the toolbar's title line in 15 pt Semibold.
    static void DrawToolbarTitle(ImDrawList* draw, float x, float right, float top, const char* title) {
        const Font font = ToolbarTitleFont(false);
        const float center = top + Px(Metrics::Window().titleCenter);
        Typography::Draw(draw, font, ImRect(x, center - Px(font.lineHeight * 0.5f), right, center + Px(font.lineHeight * 0.5f)), Theme::Colors().toolbarTitle, title);
    }

    // A title over a subtitle placed by their baselines, as with a subtitle in System Settings; the subtitle's parts
    // follow each other with a symbol before their text.
    static void DrawToolbarTitles(ImDrawList* draw, float x, float right, float top, const char* title, std::span<const ToolbarSubtitle> subtitle) {
        const Metrics::WindowMetrics& metrics = Metrics::Window();
        const Palette& colors = Theme::Colors();
        const Font title_font = ToolbarTitleFont(true);
        const float title_top = top + Px(metrics.subtitledTitleBaseline) - Typography::Baseline(title_font);
        Typography::Draw(draw, title_font, ImRect(x, title_top, right, title_top + Px(title_font.lineHeight)), colors.toolbarTitle, title);
        const Font font = Font::System(metrics.subtitleSize);
        const float subtitle_top = top + Px(metrics.subtitleBaseline) - Typography::Baseline(font);
        float part_x = x;
        for (const ToolbarSubtitle& part : subtitle) {
            const ImRect line(part_x, subtitle_top, right, subtitle_top + Px(font.lineHeight));
            if (part.symbol) {
                Typography::DrawSymbol(draw, part.symbol, font, line, colors.secondaryLabel, TextAlignment::Leading);
                part_x += Typography::SymbolWidth(part.symbol, font) + Px(metrics.subtitleSymbolSpacing);
            }
            Typography::Draw(draw, font, ImRect(part_x, line.Min.y, right, line.Max.y), colors.secondaryLabel, part.text);
            part_x += Typography::Width(font, part.text) + Px(metrics.subtitlePartSpacing);
        }
    }

    // The detail's toolbar over bar: the history buttons, the title and the items, starting at leading at least (after a
    // hidden sidebar's toggle).
    static NavigationStep Toolbar(const ImRect& bar, const NavigationSplitViewOptions& options, float leading) {
        const Metrics::WindowMetrics& metrics = Metrics::Window();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        NavigationStep step = NavigationStep::None;
        // Items sit in 38 pt slots centered in the bar.
        const float slot_top = bar.Min.y + Px((ToolbarHeight() - metrics.symbolButtonSlot) * 0.5f);
        const float slot_bottom = slot_top + Px(metrics.symbolButtonSlot);
        // After a hidden sidebar's toggle the title follows as it follows the history buttons.
        const bool after_toggle = leading > bar.Min.x + Px(metrics.toolbarMargin);
        float x = after_toggle ? leading : bar.Min.x + Px(metrics.toolbarMargin);
        if (options.showsHistoryButtons) {
            const float width = Px(metrics.symbolButtonWidth);
            if (ToolbarButton("##Back", ImRect(x, slot_top, x + width, slot_bottom), Symbols::ChevronLeft, options.canGoBack))
                step = NavigationStep::Back;
            x += width;
            if (ToolbarButton("##Forward", ImRect(x, slot_top, x + width, slot_bottom), Symbols::ChevronRight, options.canGoForward))
                step = NavigationStep::Forward;
            x += width + Px(metrics.toolbarItemSpacing);
        } else if (!after_toggle) {
            x += Px(metrics.titleInset);
        }
        const float right = bar.Max.x - Px(metrics.toolbarMargin);
        if (options.title && !options.subtitle.empty()) {
            DrawToolbarTitles(draw, x, right, bar.Min.y + ToolbarShift(), options.title, options.subtitle);
        } else if (options.title) {
            DrawToolbarTitle(draw, x, right, bar.Min.y + ToolbarShift(), options.title);
            x += Typography::Width(ToolbarTitleFont(false), options.title) + Px(metrics.titleTrailing);
        }
        if (options.toolbarItems) {
            // The items line up from the bar's end, toolbarItemSpacing apart, in the toolbar's role.
            Layout::ContainerSpec items;
            items.arrangement = Layout::Arrangement::Horizontal;
            items.alignment = Alignment{HorizontalAlignment::Trailing, VerticalAlignment::Center};
            items.spacing = metrics.toolbarItemSpacing;
            items.role = Layout::Role::Toolbar;
            items.padding = EdgeInsets{0.0f, 0.0f, 0.0f, metrics.toolbarMargin};
            Layout::Region(ImHashStr("##ToolbarItems", 0, ImGui::GetID(options.title ? options.title : "")), ImRect(x, bar.Min.y, bar.Max.x, bar.Max.y), items, [&] {
                Spacer();
                options.toolbarItems();
            });
        }
        return step;
    }

    void Toolbar(const ToolbarOptions& options, const std::function<void()>& items) {
        const Metrics::WindowMetrics& metrics = Metrics::Window();
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Horizontal;
        spec.fillWidth = true;
        spec.height = ToolbarHeight();
        spec.spacing = metrics.toolbarItemSpacing;
        spec.role = Layout::Role::Toolbar;
        spec.padding = EdgeInsets{0.0f, metrics.plainTitleX, 0.0f, metrics.toolbarMargin};
        // The background goes under the titles and items.
        ImDrawList* draw = ImGui::GetWindowDrawList();
        Layout::ContainerBehind(spec, [&] {
            const Font title_font = ToolbarTitleFont(options.subtitle != nullptr);
            const float width = Typography::Width(title_font, options.title);
            const float subtitle_width = Typography::Width(Font::System(metrics.subtitleSize), options.subtitle);
            const ImRect titles = Layout::Place(ImVec2(ImMax(width, subtitle_width) + Px(metrics.titleTrailing), Px(ToolbarHeight())));
            if (!Layout::IsMeasuring() && options.title) {
                if (options.subtitle) {
                    const ToolbarSubtitle subtitle[] = {{0, options.subtitle}};
                    DrawToolbarTitles(draw, titles.Min.x, titles.Max.x, titles.Min.y + ToolbarShift(), options.title, subtitle);
                } else {
                    DrawToolbarTitle(draw, titles.Min.x, titles.Max.x, titles.Min.y + ToolbarShift(), options.title);
                }
            }
            items();
        }, [&](const Layout::ContainerFrame& frame) {
            const float radius = Px(metrics.radius);
            Draw::FillRoundedRect(draw, frame.rect, CornerRadii(radius, radius, 0.0f, 0.0f), options.background.value_or(Theme::Colors().toolbarBackground));
            Draw::FillRect(draw, ImRect(frame.rect.Min.x, frame.rect.Max.y - Px(metrics.toolbarLine), frame.rect.Max.x, frame.rect.Max.y), Theme::Colors().toolbarLine);
        });
    }

    // Fills part of the split view, which fills the window, rounded where it meets the window's corners. A part too narrow
    // for their curve (a sidebar sliding away) or starting within it (the toolbar beside that sidebar) is cut from a band
    // across the window, which has the curve.
    static void FillWithinWindow(ImDrawList* draw, const ImRect& part, const ImRect& view, Rgba color) {
        const float radius = Px(Metrics::Window().radius);
        const float reach = Draw::CornerReach(radius);
        const bool cut = part.Min.x <= view.Min.x ? part.GetWidth() < 2.0f * reach : part.Min.x < view.Min.x + reach;
        const ImRect shape = cut ? ImRect(view.Min.x, part.Min.y, view.Max.x, part.Max.y) : part;
        const bool top = shape.Min.y <= view.Min.y;
        const bool bottom = shape.Max.y >= view.Max.y;
        const bool leading = shape.Min.x <= view.Min.x;
        const bool trailing = shape.Max.x >= view.Max.x;
        const CornerRadii radii(top && leading ? radius : 0.0f, top && trailing ? radius : 0.0f, bottom && trailing ? radius : 0.0f, bottom && leading ? radius : 0.0f);
        if (cut)
            draw->PushClipRect(part.Min, part.Max, true);
        Draw::FillRoundedRect(draw, shape, radii, color);
        if (cut)
            draw->PopClipRect();
    }

    // The widths the pointer gave the sidebar and the inspector, in points; negative until first drawn. shown slides a
    // collapsible sidebar from 0 (hidden) to 1.
    struct SplitColumns {
        float sidebar = -1.0f;
        float inspector = -1.0f;
        AnimatedFloat shown;
        bool started = false;
    };

    NavigationStep NavigationSplitView(const NavigationSplitViewOptions& options, const std::function<void()>& sidebar, const std::function<void()>& detail) {
        const Metrics::WindowMetrics& window_metrics = Metrics::Window();
        const Metrics::SidebarMetrics& sidebar_metrics = Metrics::Sidebar();
        const ImGuiID id = Layout::NextViewId();
        Layout::Placement placement;
        placement.size = Layout::Proposal();
        placement.flexibleWidth = true;
        placement.flexibleHeight = true;
        const ImRect rect = Layout::Place(placement);
        if (Layout::IsMeasuring())
            return NavigationStep::None;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const bool in_sheet = Environment().insideSheet;
        // Columns the pointer resized keep their widths.
        SplitColumns& columns = State::Get<SplitColumns>(id);
        if (columns.sidebar < 0.0f)
            columns.sidebar = options.sidebarWidth;
        if (columns.inspector < 0.0f)
            columns.inspector = Metrics::SplitView().inspectorWidth;
        const bool resizable = options.maxSidebarWidth > options.minSidebarWidth;
        const float full_width = resizable ? columns.sidebar : options.sidebarWidth;
        const bool collapsible = options.sidebarVisible && !in_sheet;
        if (collapsible && !columns.started)
            columns.shown.Snap(*options.sidebarVisible ? 1.0f : 0.0f);
        columns.started = true;
        const float shown = collapsible ? columns.shown.Update(*options.sidebarVisible ? 1.0f : 0.0f, Animation::EaseInOut(window_metrics.sidebarSlide)) : 1.0f;
        const float sidebar_width = full_width * shown;
        const float sidebar_right = rect.Min.x + Px(sidebar_width);
        const ImRect sidebar_rect(rect.Min, ImVec2(sidebar_right, rect.Max.y));
        // The sidebar material and the divider belong to the window background, which a sheet does not fade.
        const WindowPresentation* presentation = CurrentPresentation();
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, presentation ? presentation->alpha : ImGui::GetStyle().Alpha);
        // The divider is the first column of the detail side, with a faint shadow cast onto the sidebar; a hidden sidebar
        // takes both along.
        const float divider = shown > 0.0f ? Px(sidebar_metrics.divider) : 0.0f;
        if (shown > 0.0f) {
            FillWithinWindow(draw, sidebar_rect, rect, in_sheet ? colors.sheetSidebarBackground : colors.sidebarBackground);
            // Within the curve of the window's corners, the sidebar almost gone, the divider stops short of them.
            const float reach = Draw::CornerReach(Px(window_metrics.radius));
            const float inset = sidebar_right < rect.Min.x + reach ? reach : 0.0f;
            Draw::DropShadow(draw, ImRect(sidebar_right, rect.Min.y + inset, sidebar_right + divider, rect.Max.y - inset), CornerRadii(0.0f), Shadow{Rgba::Black(0.05f), ImVec2(-1.0f, 0.0f), 0.5f});
            Draw::VerticalLine(draw, sidebar_right, rect.Min.y + inset, rect.Max.y - inset, divider, in_sheet ? colors.sheetSidebarDivider : colors.sidebarDivider);
        }
        ImGui::PopStyleVar();

        Layout::ContainerSpec column;
        column.arrangement = Layout::Arrangement::Vertical;
        column.role = Layout::Role::Sidebar;
        const ImGuiID sidebar_id = ImHashStr("##Sidebar", 0, id);
        const ImGuiID detail_id = ImHashStr("##Detail", 0, id);
        if (resizable && shown >= 1.0f)
            SplitDivider(ImHashStr("##SidebarDivider", 0, id), ImRect(sidebar_right, rect.Min.y, sidebar_right + divider, rect.Max.y), ImGuiAxis_X, rect.Min.x, &columns.sidebar, options.minSidebarWidth, options.maxSidebarWidth);
        // The inspector stands at the trailing edge under the toolbar, behind a divider of its own.
        const float inspector_left = options.inspector ? Draw::Snap(rect.Max.x - Px(columns.inspector)) : rect.Max.x;
        const ImRect detail_rect(sidebar_right + divider, rect.Min.y, options.inspector ? inspector_left - divider : rect.Max.x, rect.Max.y);
        if (in_sheet) {
            column.padding.top = sidebar_metrics.sheetTop;
            Layout::Region(sidebar_id, sidebar_rect, column, sidebar);
            column.padding.top = 0.0f;
            column.role = Layout::Role::None;
            Layout::Region(detail_id, detail_rect, column, detail);
            return NavigationStep::None;
        }

        // Each column's scroll view marks content under the bar above it: a line under the sidebar's search field, the
        // toolbar's shadow over the detail.
        const float top = rect.Min.y + Px(ToolbarHeight());
        if (shown > 0.0f) {
            // A sliding sidebar keeps its width and moves out past the window's edge.
            const float left = sidebar_right - Px(full_width);
            draw->PushClipRect(ImVec2(rect.Min.x, top), ImVec2(sidebar_right, rect.Max.y), true);
            Layout::Region(sidebar_id, ImRect(left, top, sidebar_right, rect.Max.y), column, [&] { ScrollEdge(sidebar_id, ScrollEdgeStyle::Line, sidebar); });
            draw->PopClipRect();
        }
        column.role = Layout::Role::None;
        Layout::Region(detail_id, ImRect(detail_rect.Min.x, top, detail_rect.Max.x, detail_rect.Max.y), column, [&] { ScrollEdge(detail_id, ScrollEdgeStyle::Shadow, detail); });
        if (options.inspector) {
            const ImRect line(detail_rect.Max.x, top, inspector_left, rect.Max.y);
            // The inspector's width counts from the window's trailing edge, so the pointer's position is mirrored.
            float leading = Pt(line.Min.x - rect.Min.x);
            if (SplitDivider(ImHashStr("##InspectorDivider", 0, id), line, ImGuiAxis_X, rect.Min.x, &leading, Pt(rect.GetWidth()) - options.maxInspectorWidth - divider, Pt(rect.GetWidth()) - options.minInspectorWidth - divider))
                columns.inspector = Pt(rect.GetWidth()) - leading - divider;
            Draw::VerticalLine(draw, line.Min.x, top, rect.Max.y, divider, colors.sidebarDivider);
            Layout::Region(ImHashStr("##Inspector", 0, id), ImRect(inspector_left, top, rect.Max.x, rect.Max.y), column, options.inspector);
        }

        // The toolbar comes last, when the detail has reported how far it is scrolled. Its material shows once content
        // scrolls under it (always with toolbarBackground) and goes under the sidebar's toggle, which stands in the bar
        // while the sidebar is hidden.
        const ImRect bar(detail_rect.Min.x, rect.Min.y, rect.Max.x, top);
        if (options.toolbarBackground || ScrollEdgeOffset(detail_id) > 0.0f)
            FillWithinWindow(draw, bar, rect, options.toolbarBackground ? colors.toolbarBackground : colors.scrollEdgeBackground);

        // The sidebar's toggle rides its trailing edge until it meets the traffic lights.
        float leading = 0.0f;
        if (collapsible) {
            const float width = Px(window_metrics.symbolButtonWidth);
            const float slot_top = rect.Min.y + Px((ToolbarHeight() - window_metrics.symbolButtonSlot) * 0.5f);
            const float x = ImMax(rect.Min.x + Px(window_metrics.lightsEnd + window_metrics.toolbarItemSpacing), sidebar_right - Px(window_metrics.toolbarMargin) - width);
            ImGui::PushID(id);
            if (ToolbarButton("##SidebarToggle", ImRect(x, slot_top, x + width, slot_top + Px(window_metrics.symbolButtonSlot)), Symbols::SidebarLeft))
                *options.sidebarVisible = !*options.sidebarVisible;
            ImGui::PopID();
            leading = x + width + Px(window_metrics.toolbarItemSpacing);
        }
        return Toolbar(bar, options, leading);
    }
} // namespace Cupertino
