#include "Examples.h"

#include "Cupertino.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace Examples {
    using namespace Cupertino;

    // Apple's Activity Monitor screenshot (macOS 15 @2x, Energy tab): the rows, their values (null for the user's name)
    // and which ones expand.
    struct Process {
        const char* name;
        Icon icon;
        const char* values[6];
        bool expands = true;
        bool dimmed = false;
    };

    static const std::array<Process, 11>& Processes() {
        static const std::array<Process, 11> processes = {{
            {"Final Cut Pro", {Symbols::Film, IconPlate::Black}, {"53.9", "-", "No", "Yes", "Yes", nullptr}},
            {"Photos", {Symbols::CameraMacro, IconPlate::White}, {"5.4", "-", "No", "Yes", "No", nullptr}},
            {"Activity Monitor", {Symbols::WaveformPathEcg, IconPlate::Black}, {"2.8", "2.41", "No", "No", "No", nullptr}},
            {"Music", {Symbols::MusicNote, IconPlate::Pink}, {"2.4", "-", "No", "No", "No", nullptr}},
            {"Finder", {Symbols::FaceSmiling, IconPlate::Blue}, {"0.1", "-", "No", "No", "No", nullptr}},
            {"Safari", {Symbols::Safari, IconPlate::White}, {"0.1", "10.38", "No", "No", "No", nullptr}},
            {"Freeform", {Symbols::Scribble, IconPlate::White}, {"0.1", "-", "No", "No", "No", nullptr}},
            {"Notes", {Symbols::NoteText, IconPlate::Yellow}, {"0.2", "-", "No", "No", "No", nullptr}},
            {"Spotlight", {Symbols::Magnifyingglass, IconPlate::Gray}, {"11.2", "-", "-", "-", "-", "-"}},
            {"Time Machine", {Symbols::ClockArrowCirclepath, IconPlate::DarkGray}, {"-", "0.04", "-", "-", "-", "-"}, false, true},
            {"photolibraryd", {Symbols::AppleTerminal, IconPlate::Black}, {"11.2", "-", "-", "-", "-", "-"}},
        }};
        return processes;
    }

    // The rows in the table's order: numbers by value with a dash under every number, names and words by their letters;
    // rows that tie keep the order Activity Monitor lists them in.
    static std::vector<int> SortedRows(const std::array<Process, 11>& processes, const TableSort& sort, const char* user) {
        const auto text = [&](int row) -> std::string_view {
            const Process& process = processes[size_t(row)];
            if (sort.column <= 0)
                return process.name;
            return sort.column <= 6 && process.values[sort.column - 1] ? process.values[sort.column - 1] : user;
        };
        const auto number = [](std::string_view value) { return value == "-" ? -1.0f : std::strtof(std::string(value).c_str(), nullptr); };
        const auto less = [&](int a, int b) {
            const std::string_view x = text(a);
            const std::string_view y = text(b);
            const bool numeric = (x == "-" || std::isdigit(static_cast<unsigned char>(x.front()))) && (y == "-" || std::isdigit(static_cast<unsigned char>(y.front())));
            return numeric ? number(x) < number(y) : x < y;
        };
        std::vector<int> order(processes.size());
        for (int i = 0; i < int(order.size()); ++i)
            order[size_t(i)] = i;
        std::stable_sort(order.begin(), order.end(), [&](int a, int b) { return sort.ascending ? less(a, b) : less(b, a); });
        return order;
    }

    // The app column of the outline: the disclosure triangle (chevron.right Semibold 9.5, 7.5 pt tall @2x) at the
    // column's start, the 16 pt icon, then the name.
    static void AppCell(const TableCell& cell, const Process& process) {
        const Palette& colors = Theme::Colors();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        if (process.expands) {
            const float left = cell.bounds.Min.x - Px(2.25f);
            const float top = cell.bounds.GetCenter().y - Px(8.0f);
            const ImRect frame(left, top, left + Px(8.0f), top + Px(16.0f));
            Typography::DrawSymbol(draw, Symbols::ChevronRight, Font::System(9.5f, FontWeight::Semibold), frame, cell.emphasized ? colors.selectedContent : colors.secondaryLabel);
        }
        const float icon_top = cell.bounds.GetCenter().y - Px(8.0f);
        const ImVec2 icon_min(cell.bounds.Min.x + Px(10.25f), icon_top);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * (process.dimmed ? 0.5f : 1.0f));
        DrawIcon(draw, ImRect(icon_min, icon_min + Px(ImVec2(16.0f, 16.0f))), process.icon);
        ImGui::PopStyleVar();
        TableCell text = cell;
        text.rect.Min.x = cell.bounds.Min.x + Px(37.5f);
        TableText(text, process.name, process.dimmed);
    }

    // The energy impact history, a value every 3 pt, read off the reference's line.
    static constexpr std::array<float, 66> EnergyImpact = {
        75.9f, 43.8f, 33.0f, 31.3f, 29.4f, 47.0f, 95.2f, 43.0f, 49.8f, 37.2f, 30.8f, 29.4f, 36.6f, 40.8f, 35.0f, 29.4f,
        37.9f, 39.3f, 31.2f, 23.5f, 38.2f, 32.5f, 35.9f, 32.3f, 29.4f, 30.6f, 33.7f, 35.3f, 47.3f, 41.3f, 32.5f, 49.5f,
        32.5f, 42.1f, 42.2f, 46.8f, 38.9f, 42.6f, 39.9f, 57.2f, 62.8f, 53.9f, 56.7f, 49.6f, 50.7f, 48.8f, 45.5f, 37.2f,
        33.3f, 30.7f, 26.9f, 38.7f, 49.8f, 38.6f, 25.0f, 47.5f, 56.1f, 38.8f, 43.5f, 82.1f, 45.9f, 39.3f, 42.3f, 53.0f,
        48.9f, 61.2f,
    };

    // A chart column of the panel: a 9 pt Bold caption over a line, both inset from the column's sides, then the chart.
    static void ChartColumn(const char* caption, float leading, float trailing, const std::function<void()>& chart) {
        const Palette& colors = Theme::Colors();
        VStack({.spacing = 0.0f}, [&] {
            Padding(EdgeInsets{0.0f, leading, 0.0f, trailing}, [&] {
                VStack({.spacing = 0.0f}, [&] {
                    Frame({.height = 23.0f, .maxWidth = Infinity}, [&] { Text(caption, {.font = Font::System(9.0f, FontWeight::Bold), .foreground = Foreground::Secondary}); });
                    Divider(colors.boxBorder);
                });
            });
            chart();
        });
    }

    // The middle column: 11 pt rows 20 pt apart between inset lines, values on the trailing side.
    static void InfoColumn() {
        static const std::array<std::array<const char*, 2>, 4> rows = {{
            {"Graphics Card:", "High Perf."},
            {"Remaining charge:", "100%"},
            {"Battery Is Charged", ""},
            {"Time on AC:", "0:01"},
        }};
        const Font font = Font::System(11.0f);
        Padding(EdgeInsets{5.0f, 10.5f, 0.0f, 10.5f}, [&] {
            VStack({.spacing = 0.0f}, [&] {
                for (size_t i = 0; i < rows.size(); ++i) {
                    if (i > 0)
                        Divider(Theme::Colors().boxBorder);
                    Frame({.height = 19.0f, .maxWidth = Infinity}, [&] {
                        Padding(EdgeInsets{1.0f, 3.0f, 0.0f, 3.5f}, [&] {
                            HStack({.spacing = 0.0f}, [&] {
                                Text(rows[i][0], {.font = font});
                                Spacer();
                                Text(rows[i][1], {.font = font});
                            });
                        });
                    });
                }
            });
        });
    }

    static void EnergyColumn() {
        ChartColumn("ENERGY IMPACT", 12.0f, 9.5f, [] {
            Padding(EdgeInsets{2.0f, 3.0f, 2.0f, 0.5f}, [] {
                AreaChart(EnergyImpact, {.height = 62.0f, .maximum = 100.0f, .color = Theme::SystemCyan(), .outlinesArea = true, .spacing = 3.0f});
            });
        });
    }

    // The battery level over a green band while on AC power; the chart reaches under the column's divider.
    static void BatteryColumn() {
        ChartColumn("BATTERY (Last 12 hours)", 9.5f, 12.0f, [] {
            Padding(EdgeInsets{0.0f, -0.5f, 2.0f, 2.0f}, [] {
                Background([](ImDrawList* draw, const ImRect& rect) { Draw::FillRect(draw, rect, Theme::SystemGreen().Opacity(0.28f)); }, [] {
                    Padding(EdgeInsets{2.0f, 0.0f, 0.0f, 0.0f}, [] {
                        static constexpr std::array<float, 2> level = {100.0f, 100.0f};
                        AreaChart(level, {.height = 62.0f, .maximum = 100.0f, .color = Theme::SystemCyan(), .lineWidth = 0.5f, .areaOpacity = 0.0f, .outlinesArea = true});
                    });
                });
            });
        });
    }

    // An AppKit box: the content background with a 1 pt border and 4.5 pt corners.
    static void PanelBox(const std::function<void()>& content) {
        const Palette& colors = Theme::Colors();
        const CornerRadii radii(Px(4.5f));
        Background([&](ImDrawList* draw, const ImRect& rect) { Draw::FillRoundedRect(draw, rect, radii, colors.tableBackground, CornerStyle::Circular); }, [&] {
            Overlay([&](ImDrawList* draw, const ImRect& rect) { Draw::StrokeRoundedRect(draw, rect, radii, colors.boxBorder, Px(1.0f), StrokeAlignment::Inside, CornerStyle::Circular); }, content);
        });
    }

    // The panel under the table: a 580 x 90 pt box of three columns, 12 pt under the table and centered.
    static void BottomPanel() {
        const Palette& colors = Theme::Colors();
        VStack({.spacing = 0.0f}, [&] {
            Divider(colors.boxBorder);
            Frame({.height = 116.0f, .maxWidth = Infinity, .alignment = {HorizontalAlignment::Center, VerticalAlignment::Top}}, [&] {
                Padding(EdgeInsets{12.0f, 0.0f, 0.0f, 0.0f}, [&] {
                    PanelBox([&] {
                        Frame({.width = 580.0f, .height = 90.0f}, [&] {
                            HStack({.spacing = 0.0f}, [&] {
                                Frame({.width = 197.5f, .height = 90.0f}, EnergyColumn);
                                Divider(colors.boxBorder);
                                Frame({.width = 183.0f, .height = 90.0f, .alignment = {HorizontalAlignment::Center, VerticalAlignment::Top}}, InfoColumn);
                                Divider(colors.boxBorder);
                                Frame({.width = 197.5f, .height = 90.0f}, BatteryColumn);
                            });
                        });
                    });
                });
            });
        });
    }

    void ActivityMonitor(bool* open, const ActivityMonitorOptions& options) {
        static std::vector<bool> selection = {true, true};
        static TableSort sort = {4, false};
        Window("Activity Monitor", open, {.size = options.placement.SizeOr(ImVec2(960.0f, 550.0f)), .position = options.placement.position}, [&] {
            Toolbar({.title = "Activity Monitor", .subtitle = "Applications in last 12 hours"}, [] {
                static int tab = 2;
                static std::string search;
                Button("##stop", {.symbol = Symbols::XmarkOctagon});
                Disabled(true, [] { Button("##inspect", {.symbol = Symbols::InfoCircle}); });
                Menu("##actions", {"Sample Process", "Run Spindump", "Run System Diagnostics"}, {.symbol = Symbols::EllipsisCircle});
                Spacer();
                Picker("##tab", &tab, {"CPU", "Memory", "Energy", "Disk", "Network"}, {.style = PickerStyle::Segmented, .width = 339.0f});
                Spacer();
                SearchField("##search", &search, {.width = 197.0f});
            });
            const std::array<Process, 11>& processes = Processes();
            const std::vector<int> order = SortedRows(processes, sort, options.user);
            Table("processes", {
                .columns = {
                    {"App Name", 198.5f},
                    {"Energy Impact", 100.0f, TextAlignment::Trailing},
                    {"12 hr Power", 91.0f, TextAlignment::Trailing},
                    {"App Nap", 86.0f, TextAlignment::Center},
                    {"Graphics Card", 120.5f, TextAlignment::Center},
                    {"Preventing Sleep", 117.0f, TextAlignment::Center},
                    {"User", 93.0f},
                    {},
                },
                .rows = int(processes.size()),
                .sort = &sort,
                .font = Font::System(11.0f).MonospacedDigit(),
                .initialFocus = true,
            }, &selection, [&](const TableCell& cell) {
                const Process& process = processes[size_t(order[size_t(cell.row)])];
                if (cell.column == 0)
                    AppCell(cell, process);
                else if (cell.column <= 6)
                    TableText(cell, process.values[cell.column - 1] ? process.values[cell.column - 1] : options.user, process.dimmed);
            });
            BottomPanel();
        });
    }
} // namespace Examples
