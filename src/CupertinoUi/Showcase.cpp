#include "Cupertino.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace Cupertino {

    enum class ShowcasePane {
        Buttons,
        Choices,
        Values,
        Text,
        Form,
        Lists,
        Boxes,
        Presentations,
        Layout,
        Charts,
    };

    struct ShowcaseEntry {
        const char* title;
        const char* slug;
        ShowcasePane pane;
        Icon icon;
    };

    static const std::vector<ShowcaseEntry>& Entries() {
        static const std::vector<ShowcaseEntry> entries = {
            {"Buttons", "buttons", ShowcasePane::Buttons, {Symbols::CursorarrowRays, IconPlate::Blue}},
            {"Choices", "choices", ShowcasePane::Choices, {Symbols::ListBullet, IconPlate::Purple}},
            {"Values", "values", ShowcasePane::Values, {Symbols::SliderHorizontal3, IconPlate::Orange}},
            {"Text", "text", ShowcasePane::Text, {Symbols::Textformat, IconPlate::Gray}},
            {"Form", "form", ShowcasePane::Form, {Symbols::GearshapeFill, IconPlate::Gray}},
            {"Lists", "lists", ShowcasePane::Lists, {Symbols::ListBulletRectangle, IconPlate::Green}},
            {"Boxes", "boxes", ShowcasePane::Boxes, {Symbols::RectangleStack, IconPlate::Cyan}},
            {"Presentations", "presentations", ShowcasePane::Presentations, {Symbols::SquareOnSquare, IconPlate::Red}},
            {"Layout", "layout", ShowcasePane::Layout, {Symbols::RectangleSplit2x1, IconPlate::Pink}},
            {"Charts", "charts", ShowcasePane::Charts, {Symbols::ChartBarXaxis, IconPlate::Orange}},
        };
        return entries;
    }

    struct ShowcaseModel {
        ShowcasePane pane = ShowcasePane::Buttons;
        // The first pane is taken from the options once, so the sidebar keeps working after it.
        bool paneChosen = false;
        bool sidebarVisible = true;
        float splitLeading = 160.0f;
        float splitTop = 70.0f;
        float shelfOffset = 0.0f;
        bool bold = true;
        bool checked = true;
        bool unchecked = false;
        bool enabled = true;
        bool notifications = true;
        bool formats[3] = {true, false, false};
        int size = 1;
        int alignment = 0;
        int appearance = 0;
        int color = 2;
        int arrows = 0;
        int scope = 0;
        bool shared = true;
        std::string combo = "Helvetica";
        Rgba well = Rgba::Hex(0x0A84FF);
        float volume = 0.6f;
        float brightness = 0.4f;
        float angle = 0.25f;
        float ticks = 0.5f;
        int copies = 2;
        float rating = 3.0f;
        DateTime date = {2024, 9, 13, 22, 0};
        DateTime alarm = {2024, 9, 13, 7, 0};
        float delay = 1.5f;
        std::string name = "Untitled";
        std::string empty;
        std::string password = "secret";
        std::string query;
        std::string note;
        std::string code = "let total = items\n    .map(\\.price)\n    .reduce(0, +)";
        std::vector<std::string> tokens = {"Design", "Review"};
        std::string notes = "A text editor wraps its lines at its width and scrolls when they no longer fit. Return starts a new line.\n\nClick to place the insertion point, drag to select, and use the arrow keys to move between lines.";
        ImGuiKey key = ImGuiKey_F1;
        ImGuiKeyChord shortcut = ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_K;
        int quality = 1;
        int theme = 0;
        bool expanded = true;
        std::vector<bool> tableSelection = std::vector<bool>(12, false);
        TableSort tableSort = {0, true};
        std::vector<bool> listSelection = std::vector<bool>(4, false);
        std::vector<bool> plainSelection = {false, true, false};
        std::vector<bool> insetSelection = {true, false, false};
        int tab = 0;
        bool alert = false;
        bool critical = false;
        bool confirm = false;
        bool login = false;
        std::string username;
        std::string secret;
        bool sheet = false;
        bool banner = false;
        bool popover = false;
        int day = -1;
        int category = -1;
    };

    static ShowcaseModel& Model() {
        static ShowcaseModel model;
        return model;
    }


    // A pane of views outside a form: groups under their titles, one above the other.
    static void Pane(const std::function<void()>& content) {
        ScrollView({.padding = EdgeInsets::All(20.0f), .spacing = 20.0f}, content);
    }

    static void Row(const std::function<void()>& content) {
        HStack({.alignment = VerticalAlignment::Center, .spacing = 12.0f}, content);
    }

    static void ButtonsPane(ShowcaseModel& model) {
        Pane([&] {
            GroupBox("Push buttons", [&] {
                Row([] {
                    Button("Default", {.role = ButtonRole::Default});
                    Button("Button");
                    Button("Delete", {.role = ButtonRole::Destructive});
                    Disabled(true, [] { Button("Disabled"); });
                });
                Row([] {
                    for (const ControlSize size : {ControlSize::Mini, ControlSize::Small, ControlSize::Regular, ControlSize::Large}) {
                        WithControlSize(size, [&] {
                            ImGui::PushID(int(size));
                            Button("Button");
                            ImGui::PopID();
                        });
                    }
                });
                Row([] {
                    Button("Add", {.symbol = Symbols::Plus});
                    Button("Share", {.symbol = Symbols::SquareAndArrowUp});
                    Button("##settings", {.symbol = Symbols::Gearshape});
                });
            });
            GroupBox("Borderless, links and help", [] {
                Row([] {
                    Button("##share", {.style = ButtonStyle::Borderless, .symbol = Symbols::SquareAndArrowUp});
                    Button("##trash", {.style = ButtonStyle::Borderless, .symbol = Symbols::Trash});
                    Button("##info", {.style = ButtonStyle::Borderless, .symbol = Symbols::InfoCircle});
                    Button("Learn More…", {.style = ButtonStyle::Link});
                    HelpButton();
                });
            });
            GroupBox("Toggle buttons and segments", [&] {
                Row([&] {
                    Toggle("Bold", &model.bold, {.style = ToggleStyle::Button});
                    SegmentedToggles("##formats", std::span<bool>(model.formats), {Typography::SymbolText(Symbols::Bold).c_str(), Typography::SymbolText(Symbols::Italic).c_str(), Typography::SymbolText(Symbols::Underline).c_str()});
                });
            });
            GroupBox("Accessory bar", [&] {
                ScopeBar("Search:", &model.scope, {"This Mac", "Documents", "Shared"}, [] {
                    Button("Save", {.style = ButtonStyle::AccessoryBarAction});
                    Button("##add-scope", {.style = ButtonStyle::AccessoryBarAction, .symbol = Symbols::Plus});
                });
                Row([&] {
                    Toggle("Shared", &model.shared, {.style = ToggleStyle::AccessoryBar});
                    Button("Recents", {.style = ButtonStyle::AccessoryBar});
                });
            });
            GroupBox("Menus", [] {
                Row([] {
                    Menu("Actions", {"Duplicate", "Rename", "Move to Trash"});
                    Menu("##more", {"Duplicate", "Rename", "Move to Trash"}, {.symbol = Symbols::EllipsisCircle});
                    ControlGroup([] {
                        Button("##add", {.symbol = Symbols::Plus});
                        Button("##remove", {.symbol = Symbols::Minus});
                        Menu("##action", {.symbol = Symbols::Gearshape}, [] {
                            Button("Rename");
                            Button("Duplicate");
                        });
                    });
                });
            });
        });
    }

    static void ChoicesPane(ShowcaseModel& model) {
        Pane([&] {
            GroupBox("Checkboxes and radio buttons", [&] {
                Row([&] {
                    Toggle("On", &model.checked, {.style = ToggleStyle::Checkbox});
                    Toggle("Off", &model.unchecked, {.style = ToggleStyle::Checkbox});
                    Toggle("Mixed", &model.unchecked, {.style = ToggleStyle::Checkbox, .mixed = true});
                    Disabled(true, [&] { Toggle("Disabled", &model.checked, {.style = ToggleStyle::Checkbox}); });
                });
                Picker("Alignment:", &model.alignment, {"Left", "Center", "Right"}, {.style = PickerStyle::RadioGroup});
            });
            GroupBox("Switches", [&] {
                Row([&] {
                    for (const ControlSize size : {ControlSize::Mini, ControlSize::Small, ControlSize::Regular}) {
                        WithControlSize(size, [&] {
                            ImGui::PushID(int(size));
                            Toggle("##switch", &model.enabled, {.style = ToggleStyle::Switch});
                            ImGui::PopID();
                        });
                    }
                    Disabled(true, [&] { Toggle("##disabled", &model.enabled, {.style = ToggleStyle::Switch}); });
                });
            });
            GroupBox("Pop-up buttons, combo boxes and wells", [&] {
                Row([&] {
                    Picker("##size", &model.size, {"Small", "Medium", "Large"});
                    Picker("##arrows", &model.arrows, {"Automatic", "Always", "Never"}, {.style = PickerStyle::Arrows});
                    ComboBox("##font", &model.combo, {"Helvetica", "Menlo", "SF Pro", "New York"}, {.width = 140.0f});
                });
                Row([&] {
                    ColorPicker("##well", &model.well);
                    ColorPicker("##bordered", &model.well, {.style = ColorWellStyle::Bordered});
                    ImageWell("##image", {Symbols::FolderFill, IconPlate::Blue});
                });
            });
            GroupBox("Segmented controls", [&] {
                Picker("##appearance", &model.appearance, {"Light", "Dark", "Auto"}, {.style = PickerStyle::Segmented});
                const std::string left = Typography::SymbolText(Symbols::ListBullet);
                const std::string middle = Typography::SymbolText(Symbols::RectangleStack);
                const std::string right = Typography::SymbolText(Symbols::Square);
                Picker("##view", &model.color, {left.c_str(), middle.c_str(), right.c_str()}, {.style = PickerStyle::Segmented});
            });
        });
    }

    static void ValuesPane(ShowcaseModel& model) {
        Pane([&] {
            GroupBox("Sliders", [&] {
                Slider("##volume", &model.volume, 0.0f, 1.0f, {.width = 240.0f, .minimumSymbol = Symbols::SpeakerFill, .maximumSymbol = Symbols::SpeakerWave3Fill});
                Slider("##ticks", &model.ticks, 0.0f, 1.0f, {.width = 240.0f, .ticks = 5, .snapsToTicks = true, .captions = {"Slow", "Fast"}});
                Row([&] {
                    Slider("##brightness", &model.brightness, 0.0f, 1.0f, {.width = 160.0f, .ticks = 9, .filled = true});
                    Slider("##angle", &model.angle, 0.0f, 1.0f, {.style = SliderStyle::Circular});
                    Disabled(true, [&] { Slider("##disabled", &model.volume, 0.0f, 1.0f, {.width = 100.0f}); });
                });
            });
            GroupBox("Steppers", [&] {
                Row([&] {
                    // The label carries the count, its id stays after ###.
                    Stepper(("Copies: " + std::to_string(model.copies) + "###copies").c_str(), &model.copies, 1, 99);
                    Stepper("##delay", &model.delay, 0.0f, 10.0f, 0.5f, "%.1f s");
                });
            });
            GroupBox("Dates", [&] {
                DatePicker("Starts:", &model.date);
                Row([&] {
                    DatePicker("##alarm", &model.alarm, {.components = DatePickerComponents::HourAndMinute});
                    DatePicker("##alarm-24", &model.alarm, {.components = DatePickerComponents::HourAndMinute, .uses24HourClock = true});
                    Disabled(true, [&] { DatePicker("##disabled-date", &model.date, {.components = DatePickerComponents::Date}); });
                });
                // Today pinned, so captures stay the same from day to day.
                WithEnvironment([](EnvironmentValues& environment) { environment.today = {2024, 9, 16}; }, [&] {
                    DatePicker("##calendar", &model.date, {.style = DatePickerStyle::Graphical, .components = DatePickerComponents::Date});
                });
            });
            GroupBox("Level indicators", [&] {
                LevelIndicator("##capacity", 0.75f, {.width = 240.0f});
                LevelIndicator("##tiered", 0.75f, {.warning = 0.5f, .critical = 0.125f, .tiered = true, .width = 240.0f});
                LevelIndicator("##warning", 0.85f, {.warning = 0.7f, .critical = 0.9f, .width = 240.0f});
                LevelIndicator("##cells", 6.0f, {.style = LevelIndicatorStyle::DiscreteCapacity, .maximum = 8.0f, .width = 240.0f});
                LevelIndicator("Rating:", &model.rating, {.style = LevelIndicatorStyle::Rating, .maximum = 5.0f});
            });
            GroupBox("Progress", [] {
                ProgressView(0.62f, {.style = ProgressViewStyle::Linear, .width = 240.0f});
                ProgressView({.style = ProgressViewStyle::Linear, .width = 240.0f});
                Row([] {
                    ProgressView(0.62f, {.style = ProgressViewStyle::Circular});
                    ProgressView();
                });
                ProgressView(0.62f, {.width = 240.0f, .label = "Downloading \xE2\x80\x9CProjects.zip\xE2\x80\x9D", .currentValueLabel = "744 MB of 1.2 GB"});
            });
        });
    }

    static void TextPane(ShowcaseModel& model) {
        Pane([&] {
            GroupBox("Fields", [&] {
                TextField("##name", &model.name, {.width = 220.0f});
                TextField("Placeholder", &model.empty, {.width = 220.0f});
                SecureField("##password", &model.password, {.width = 220.0f});
                TextField("Plain field", &model.note, {.style = TextFieldStyle::Plain, .width = 220.0f});
                // Suggestions filtered by what is typed, as an app hands them to searchSuggestions.
                static const char* const Fruits[] = {"Apple", "Apricot", "Avocado", "Banana", "Blueberry", "Cherry"};
                std::vector<const char*> matches;
                for (const char* fruit : Fruits) {
                    if (!model.query.empty() && std::string_view(fruit).substr(0, model.query.size()) == model.query)
                        matches.push_back(fruit);
                }
                SearchField("##search", &model.query, {.width = 220.0f, .suggestions = matches});
                Disabled(true, [&] { TextField("##disabled", &model.name, {.width = 220.0f}); });
            });
            GroupBox("Paths", [&] {
                const Icon folder = {.paint = PaintFolderIcon};
                const Icon disk = {.symbol = Symbols::Internaldrive, .color = Theme::SystemGray()};
                Frame({.width = 360.0f}, [&] { PathControl("##path", {{"Macintosh HD", disk}, {"Users", folder}, {"anna", folder}, {"Projects", folder}}); });
                Frame({.width = 220.0f}, [&] { PathControl("##collapsed", {{"Macintosh HD", disk}, {"Users", folder}, {"anna", folder}, {"Projects", folder}}); });
                Frame({.width = 220.0f}, [&] { PathControl("##popup", {{"Macintosh HD", disk}, {"Users", folder}, {"anna", folder}, {"Projects", folder}}, {.style = PathControlStyle::PopUp}); });
            });
            GroupBox("Tokens and shortcuts", [&] {
                TokenField("##tokens", &model.tokens, {.placeholder = "Tags", .width = 260.0f});
                Row([&] {
                    KeyRecorder("##key", &model.key);
                    KeyRecorder("##shortcut", &model.shortcut);
                });
            });
            GroupBox("Text editors", [&] {
                // The editors share the row, however wide the window.
                Row([&] {
                    Frame({.height = 120.0f}, [&] { TextEditor("##notes", &model.notes, {.border = ScrollViewBorder::Line}); });
                    Frame({.height = 120.0f}, [&] { TextEditor("##code", &model.code, {.font = Font::Style(TextStyle::Callout).Monospaced(), .border = ScrollViewBorder::Line}); });
                });
            });
        });
    }

    static void FormPane(ShowcaseModel& model) {
        Form([&] {
            Section({.header = "General"}, [&] {
                Toggle("Notifications", &model.notifications, {.description = "Banners appear in the top-right corner and go away on their own."});
                Picker("Quality", &model.quality, {"Low", "Medium", "High"});
                Picker("Theme", &model.theme, {"Light", "Dark", "Auto"}, {.style = PickerStyle::RadioGroup, .horizontal = true});
                Slider("Volume", &model.volume, 0.0f, 1.0f, {.minimumSymbol = Symbols::SpeakerFill, .maximumSymbol = Symbols::SpeakerWave3Fill});
                Stepper("Copies", &model.copies, 1, 99, 1, "%d");
                DatePicker("Downtime", &model.date, {.components = DatePickerComponents::HourAndMinute});
            });
            Section({.header = "Account"}, [&] {
                TextField("Name", &model.name);
                LabeledContent("Storage", "12.4 GB of 50 GB");
                LabeledContent("Shortcut", {.description = "Opens the window from anywhere."}, [&] { KeyRecorder("##form-shortcut", &model.shortcut); });
                NavigationLink("Advanced", {.icon = {Symbols::GearshapeFill, IconPlate::Gray}});
            });
        });
    }

    static void ListsPane(ShowcaseModel& model) {
        Pane([&] {
            GroupBox("Table", [&] {
                struct Folder {
                    const char* name;
                    const char* size;
                    float megabytes;
                };
                static const Folder Folders[] = {
                    {"Applications", "24.5 GB", 24500.0f}, {"Desktop", "760 MB", 760.0f}, {"Documents", "1.2 GB", 1200.0f}, {"Downloads", "340 MB", 340.0f},
                    {"Library", "18.3 GB", 18300.0f}, {"Movies", "8.9 GB", 8900.0f}, {"Music", "2.1 GB", 2100.0f}, {"Pictures", "5.6 GB", 5600.0f},
                    {"Projects", "3.4 GB", 3400.0f}, {"Public", "12 KB", 0.012f}, {"Shared", "96 MB", 96.0f}, {"Sites", "4 MB", 4.0f},
                };
                static const char* const Actions[] = {"Open", "Show in Finder", "Get Info", "Move to Trash"};
                // The rows in the sort order the header sets.
                std::vector<int> order(IM_ARRAYSIZE(Folders));
                for (int i = 0; i < int(order.size()); ++i)
                    order[size_t(i)] = i;
                std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
                    const bool by_size = model.tableSort.column == 1;
                    const bool less = by_size ? Folders[a].megabytes < Folders[b].megabytes : std::string_view(Folders[a].name) < Folders[b].name;
                    const bool more = by_size ? Folders[b].megabytes < Folders[a].megabytes : std::string_view(Folders[b].name) < Folders[a].name;
                    return model.tableSort.ascending ? less : more;
                });
                Frame({.height = 180.0f}, [&] {
                    Table("##table", {.columns = {{"Name", 200.0f}, {"Size", 80.0f, TextAlignment::Trailing}}, .rows = int(order.size()), .sort = &model.tableSort, .contextMenu = Actions}, &model.tableSelection, [&](const TableCell& cell) {
                        const Folder& folder = Folders[order[size_t(cell.row)]];
                        TableText(cell, cell.column == 0 ? folder.name : folder.size);
                    });
                });
            });
            GroupBox("Bordered list", [&] {
                static const char* const folders[] = {"Public", "Shared", "Projects", "Archive"};
                BorderedList("##list", {.rows = 4, .height = 120.0f, .showsAddRemove = true}, &model.listSelection, [&](int row) { Text(folders[row]); });
            });
            GroupBox("Plain and inset lists", [&] {
                static const char* const accounts[] = {"iCloud", "Work", "Personal"};
                static const char* const actions[] = {"Edit Account\xE2\x80\xA6", "Duplicate"};
                HStack({.alignment = VerticalAlignment::Top, .spacing = 20.0f}, [&] {
                    Frame({.width = 200.0f}, [&] {
                        BorderedList("##plain", {.style = ListStyle::Plain, .rows = 3, .height = 96.0f, .alternatesRows = false, .showsSeparators = true, .showsAddRemove = true, .actions = actions}, &model.plainSelection, [&](int row) { Text(accounts[row]); });
                    });
                    Frame({.width = 200.0f}, [&] {
                        BorderedList("##inset", {.style = ListStyle::Inset, .rows = 3, .height = 109.0f, .alternatesRows = false, .showsSeparators = true, .showsAddRemove = true}, &model.insetSelection, [&](int row) { Text(accounts[row]); });
                    });
                });
            });
            GroupBox("Disclosure", [&] {
                DisclosureGroup("Details", &model.expanded, [] {
                    Text("Created: Today, 9:41", {.foreground = Foreground::Secondary});
                    Text("Modified: Today, 10:12", {.foreground = Foreground::Secondary});
                });
            });
        });
    }

    static void BoxesPane(ShowcaseModel& model) {
        Pane([&] {
            GroupBox("Group box", [] { Text("Content in a faintly tinted box, under its label."); });
            TabView(&model.tab, {"General", "Advanced"}, [](int tab) {
                Text(tab == 0 ? "The first tab's content." : "The second tab's content.");
            });
            GroupBox("Scroll views", [] {
                Row([] {
                    for (const ScrollViewBorder border : {ScrollViewBorder::Line, ScrollViewBorder::Rounded}) {
                        Frame({.width = 160.0f, .height = 90.0f}, [&] {
                            ScrollView({.border = border}, [] {
                                Padding(8.0f, [] {
                                    VStack({.alignment = HorizontalAlignment::Leading, .spacing = 4.0f}, [] {
                                        for (int line = 1; line <= 12; ++line)
                                            Text("Line " + std::to_string(line));
                                    });
                                });
                            });
                        });
                    }
                });
            });
            GroupBox("Content unavailable", [&] {
                SearchField("Search", &model.query, {.width = 180.0f});
                Frame({.height = 150.0f}, [&] {
                    if (model.query.empty())
                        ContentUnavailableView("No Selection", {.symbol = Symbols::DocText, .description = "Select a document to see its details."});
                    else
                        ContentUnavailableSearch(model.query);
                });
            });
        });
    }

    // A pane of the layout views: a split view whose second pane splits again.
    static void LayoutPane(ShowcaseModel& model) {
        Pane([&] {
            GroupBox("Split views", [&] {
                Frame({.height = 180.0f}, [&] {
                    HSplitView({.position = &model.splitLeading, .minimum = 100.0f, .secondMinimum = 160.0f}, [] {
                        Padding(10.0f, [] { Text("Navigator"); });
                    }, [&] {
                        VSplitView({.position = &model.splitTop, .minimum = 40.0f, .secondMinimum = 40.0f}, [] {
                            Padding(10.0f, [] { Text("Editor"); });
                        }, [] {
                            Padding(10.0f, [] { Text("Console", {.foreground = Foreground::Secondary}); });
                        });
                    });
                });
            });
            GroupBox("Horizontal scroll view", [&] {
                Frame({.height = 44.0f}, [&] {
                    ScrollView({.axis = ImGuiAxis_X, .spacing = 8.0f, .offset = &model.shelfOffset}, [] {
                        for (int i = 1; i <= 16; ++i) {
                            ImGui::PushID(i);
                            Button(("Item " + std::to_string(i)).c_str());
                            ImGui::PopID();
                        }
                    });
                });
            });
            GroupBox("Grid", [&] {
                Grid({.alignment = {HorizontalAlignment::Leading, VerticalAlignment::FirstTextBaseline}, .horizontalSpacing = 12.0f}, [&] {
                    GridRow([&] {
                        Text("Name:");
                        TextField("##gridname", &model.name, {.width = 180.0f});
                    });
                    GridRow([&] {
                        Text("Shared folder:");
                        Toggle("Read only", &model.shared);
                    });
                    Divider();
                    GridRow([&] {
                        Text("Quality:");
                        Picker("##gridquality", &model.quality, {"Low", "Medium", "High"});
                    });
                });
            });
            GroupBox("Lazy grid", [&] {
                static const GridItem tiles[] = {{.size = GridItemSize::Adaptive, .minimum = 64.0f, .maximum = 96.0f}};
                LazyVGrid(tiles, 18, [](int index) {
                    Canvas(ImVec2(64.0f, 44.0f), [index](ImDrawList* draw, const ImRect& rect) {
                        Draw::FillRoundedRect(draw, rect, CornerRadii(Px(8.0f)), ChartColor(index));
                    });
                });
            });
        });
    }

    // A pane of Swift Charts' marks and SwiftUI's gauges.
    static void ChartsPane(ShowcaseModel& model) {
        static const float steps[] = {6.2f, 8.1f, 7.4f, 9.8f, 11.2f, 8.6f, 10.4f};
        static const float goal[] = {7.0f, 7.0f, 7.5f, 7.5f, 8.0f, 8.0f, 8.0f};
        static const char* const days[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
        static const ChartSector storage[] = {{"Photos", 48.0f}, {"Apps", 31.0f}, {"Documents", 12.0f}, {"System", 9.0f}};
        const ChartSeries series[] = {{"Steps", steps}, {"Goal", goal}};
        const ChartRule average[] = {{8.8f, Theme::SystemRed(), "Average"}};
        Pane([&] {
            GroupBox("Line and points", [&] {
                LineChart(series, {
                    .height = 140.0f,
                    .maximum = 12.0f,
                    .yMarks = {{0.0f, "0"}, {4.0f, "4k"}, {8.0f, "8k"}, {12.0f, "12k"}},
                    .xMarks = {{0.0f, "Mon"}, {2.0f, "Wed"}, {4.0f, "Fri"}, {6.0f, "Sun"}},
                    .points = true,
                    .rules = average,
                    .selection = &model.day,
                    .annotation = [](int day) { return std::string(days[day]) + "\n" + std::to_string(int(steps[day] * 1000.0f)) + " steps"; },
                });
            });
            GroupBox("Sectors", [&] {
                HStack({.alignment = VerticalAlignment::Top, .spacing = 24.0f}, [&] {
                    Frame({.width = 180.0f}, [&] {
                        SectorChart(storage, {
                            .innerRadius = 0.6f,
                            .angularInset = 1.5f,
                            .cornerRadius = 4.0f,
                            .selection = &model.category,
                            .center = [&] {
                                const int shown = model.category >= 0 ? model.category : 0;
                                VStack({.alignment = HorizontalAlignment::Center, .spacing = 0.0f}, [&] {
                                    Text(storage[shown].name, {.font = Font::Style(TextStyle::Headline)});
                                    Text((std::to_string(int(storage[shown].value)) + " GB").c_str(), {.foreground = Foreground::Secondary});
                                });
                            },
                        });
                    });
                    Frame({.width = 160.0f}, [] { SectorChart(storage, {}); });
                });
            });
            GroupBox("Gauges", [&] {
                const GaugeOptions speed = {.minimum = 0.0f, .maximum = 150.0f, .currentValueLabel = "75", .minimumValueLabel = "0", .maximumValueLabel = "150"};
                Gauge("Speed", 75.0f, speed);
                GaugeOptions thin = speed;
                thin.style = GaugeStyle::AccessoryLinearCapacity;
                Gauge("MPH", 75.0f, thin);
                GaugeOptions marker = speed;
                marker.style = GaugeStyle::AccessoryLinear;
                Gauge("MPH", 75.0f, marker);
                Row([&] {
                    GaugeOptions ring = speed;
                    ring.style = GaugeStyle::AccessoryCircular;
                    Gauge("MPH", 75.0f, ring);
                    ring.minimumValueLabel = nullptr;
                    ring.maximumValueLabel = nullptr;
                    Gauge("MPH", 75.0f, ring);
                    ring.style = GaugeStyle::AccessoryCircularCapacity;
                    Gauge("MPH", 75.0f, ring);
                    ring.tint = Theme::SystemGreen();
                    ring.currentValueLabel = "120";
                    Gauge("MPH", 120.0f, ring);
                });
            });
        });
    }

    static void PresentationsPane(ShowcaseModel& model) {
        Pane([&] {
            GroupBox("Alerts and sheets", [&] {
                Row([&] {
                    if (Button("Show Alert…"))
                        model.alert = true;
                    if (Button("Critical Alert…"))
                        model.critical = true;
                    if (Button("Close Document…"))
                        model.confirm = true;
                    if (Button("Sign In…"))
                        model.login = true;
                });
                Row([&] {
                    if (Button("Show Sheet…"))
                        model.sheet = true;
                    if (Button("Show Notification"))
                        model.banner = true;
                });
            });
            GroupBox("Menus, popovers and tooltips", [&] {
                ContextMenu([] {
                    Button("Copy");
                    Button("Paste");
                    Divider();
                    Button("Delete");
                }, [] { Text("Right-click here for a context menu.", {.foreground = Foreground::Secondary}); });
                Row([&] {
                    Help("A tooltip appears after a moment.", [] { Button("Hover for a Tooltip"); });
                    Popover(&model.popover, {.size = ImVec2(220.0f, 76.0f)}, [&] {
                        if (Button("Show Popover\xE2\x80\xA6"))
                            model.popover = true;
                    }, [] {
                        Padding(EdgeInsets::Symmetric(14.0f, 12.0f), [] {
                            VStack({.alignment = HorizontalAlignment::Leading, .spacing = 4.0f}, [] {
                                Text("Popover", {.font = Font::Style(TextStyle::Headline)});
                                Text("Content beside the view that opened it.", {.foreground = Foreground::Secondary, .wraps = true});
                            });
                        });
                    });
                });
            });
        });
        Alert("Delete “Projects”?", &model.alert, {{"Cancel", ButtonRole::Cancel}, {"Delete", ButtonRole::Destructive}}, {.message = "This item will be deleted immediately. You can’t undo this action."});
        Alert("The disk is almost full", &model.critical, {{"OK", ButtonRole::Default}}, {.message = "Save your work and free up space before continuing.", .critical = true});
        ConfirmationDialog("Do you want to save the changes you made to “Projects”?", &model.confirm, {{"Save", ButtonRole::Default}, {"Don’t Save"}}, {.message = "Your changes will be lost if you don’t save them."});
        const AlertTextField fields[] = {{"Username", &model.username}, {"Password", &model.secret, true}};
        Alert("Sign in to “Projects”", &model.login, {{"Cancel", ButtonRole::Cancel}, {"Sign In", ButtonRole::Default}}, {.message = "Enter your name and password for this server.", .textFields = fields});
        Sheet(&model.sheet, {.width = 360.0f}, [&] {
            Form([&] { Section([&] { TextField("Name", &model.name); }); });
            SheetFooter([&] {
                if (Button("Cancel"))
                    model.sheet = false;
                if (Button("Done", {.role = ButtonRole::Default}))
                    model.sheet = false;
            });
        });
        NotificationBanner(&model.banner, {.title = "Download Complete", .body = "“Projects.zip” is in your Downloads folder.", .timestamp = "now", .icon = {Symbols::FolderFill, IconPlate::Blue}});
    }

    static void Detail(ShowcaseModel& model) {
        switch (model.pane) {
            case ShowcasePane::Buttons:
                ButtonsPane(model);
                break;
            case ShowcasePane::Choices:
                ChoicesPane(model);
                break;
            case ShowcasePane::Values:
                ValuesPane(model);
                break;
            case ShowcasePane::Text:
                TextPane(model);
                break;
            case ShowcasePane::Form:
                FormPane(model);
                break;
            case ShowcasePane::Lists:
                ListsPane(model);
                break;
            case ShowcasePane::Boxes:
                BoxesPane(model);
                break;
            case ShowcasePane::Presentations:
                PresentationsPane(model);
                break;
            case ShowcasePane::Layout:
                LayoutPane(model);
                break;
            case ShowcasePane::Charts:
                ChartsPane(model);
                break;
        }
    }

    void ShowShowcase(bool* open, const ShowcaseOptions& options) {
        ShowcaseModel& model = Model();
        if (!model.paneChosen && options.initialPane) {
            for (const ShowcaseEntry& entry : Entries()) {
                if (std::string_view(options.initialPane) == entry.slug)
                    model.pane = entry.pane;
            }
        }
        model.paneChosen = true;
        const char* title = "";
        for (const ShowcaseEntry& entry : Entries()) {
            if (entry.pane == model.pane)
                title = entry.title;
        }
        Window("Showcase", open, {.size = options.size, .position = options.position, .minSize = ImVec2(600.0f, 400.0f)}, [&] {
            NavigationSplitView({.sidebarWidth = 190.0f, .title = title, .showsHistoryButtons = false, .sidebarVisible = &model.sidebarVisible}, [&] {
                int selection = int(model.pane);
                List([&] {
                    Section([&] {
                        for (const ShowcaseEntry& entry : Entries()) {
                            ContextMenu([] {
                                Button("Open in New Window");
                                Button("Copy Link");
                            }, [&] { NavigationLink(entry.title, &selection, int(entry.pane), {.icon = entry.icon}); });
                        }
                    });
                });
                model.pane = ShowcasePane(selection);
            }, [&] { Id(int(model.pane), [&] { Detail(model); }); });
        });
    }
} // namespace Cupertino
