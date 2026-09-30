#include "Settings.h"

#include "core/Metrics.h"

#include <algorithm>
#include <string>

namespace Examples::Settings {
    using namespace Cupertino;

    struct SpotlightCategory {
        const char* title;
        bool on = true;
    };

    struct SpotlightModel {
        std::vector<SpotlightCategory> categories = {
            {"Applications"}, {"Calculator"}, {"Contacts"}, {"Conversion"}, {"Definition"}, {"Documents"}, {"Events & Reminders"}, {"Folders"},
            {"Fonts"}, {"Images"}, {"Mail & Messages"}, {"Movies"}, {"Music"}, {"Other"}, {"PDF Documents"}, {"Presentations"},
            {"Siri Suggestions"}, {"Spreadsheets"}, {"System Settings"}, {"Tips"}, {"Websites"},
        };
        bool improveSearch = true;
    };

    // The categories as Spotlight lists them (512 Pixels @2x): checkbox rows 24 pt tall across the section, each under a
    // line from its title to the section's edge, the checkbox centered under it; the first row starts on the section's
    // own line, a point above the list.
    static void CategoryList(std::vector<SpotlightCategory>& categories) {
        const Metrics::FormMetrics& form = Metrics::Form();
        const float row_height = 24.0f;
        const float title_x = 29.0f;
        const float line = form.separatorThickness;
        const ImRect list = Layout::Place(Layout::Placement{.size = ImVec2(Layout::FullProposal().x, Px(row_height * float(categories.size()) - line)), .ignoresChildInsets = true, .fullWidthSeparator = true});
        if (Layout::IsMeasuring())
            return;
        ImDrawList* draw = ImGui::GetWindowDrawList();
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Overlay;
        spec.alignment = Alignment{HorizontalAlignment::Leading, VerticalAlignment::Center};
        for (size_t i = 0; i < categories.size(); ++i) {
            const float top = list.Min.y + Px(row_height * float(i) - line);
            if (i > 0)
                Draw::HorizontalLine(draw, list.Min.x + Px(title_x), list.Max.x, top, Px(form.separatorThickness), Theme::Colors().rowSeparator);
            const ImRect row(list.Min.x + Px(form.rowInset), top + Px(line), list.Max.x, top + Px(line + row_height));
            Layout::Region(ImGui::GetID(int(i)), row, spec, [&] { Toggle(categories[i].title, &categories[i].on, {.style = ToggleStyle::Checkbox}); });
        }
    }

    // The pane's header, its text 2 pt closer to the icon than in the other panes' headers: the icon 1 pt past the row
    // inset and 2 pt under it, the title 1.5 pt over the description (Spotlight @2x).
    static void Header() {
        HStack({.alignment = VerticalAlignment::Top, .spacing = 9.0f}, [] {
            Padding(EdgeInsets{2.0f, 1.0f, 0.0f, 0.0f}, [] {
                Image(PaneIcon(Pane::Spotlight), ImVec2(26.0f, 26.0f));
            });
            VStack({.alignment = HorizontalAlignment::Leading, .spacing = 1.5f}, [] {
                Text("Spotlight");
                Text("Spotlight helps you quickly find things on your computer and shows suggestions from the Internet, Music, App Store, movie showtimes, locations nearby, and more.", {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary, .wraps = true});
            });
        });
    }

    // Spotlight as 512 Pixels captured it: the pane's header, and the categories search results show, those the Mac
    // left on checked.
    void SpotlightPane(const Mac& mac) {
        static SpotlightModel model = [&] {
            SpotlightModel initial;
            for (SpotlightCategory& category : initial.categories)
                category.on = mac.spotlightResults.empty() || std::find_if(mac.spotlightResults.begin(), mac.spotlightResults.end(), [&](const char* title) { return std::string_view(title) == category.title; }) != mac.spotlightResults.end();
            return initial;
        }();
        Form([] {
            Section([] { Header(); });
            Section([] {
                Label("Search results", {.description = "Only selected categories will appear in Spotlight search results."});
                CategoryList(model.categories);
            });
            Section([] { Toggle("Help Apple Improve Search", &model.improveSearch, {.description = "Help improve Search by allowing Apple to store your Safari, Siri, Spotlight, Lookup, and #images search queries. The information collected is stored in a way that does not identify you and is used to improve search results."}); });
            TrailingButtons([] {
                Button("About Search & Privacy\xE2\x80\xA6");
                Button("Search Privacy\xE2\x80\xA6");
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
