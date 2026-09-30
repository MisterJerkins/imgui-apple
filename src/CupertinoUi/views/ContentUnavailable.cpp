#include "ContentUnavailable.h"

#include "controls/Text.h"
#include "core/Metrics.h"
#include "core/Symbols.h"
#include "layout/Layout.h"
#include "layout/Stacks.h"

#include <string>

namespace Cupertino {
    void ContentUnavailableView(const char* title, const ContentUnavailableOptions& options, const std::function<void()>& actions) {
        const Metrics::ContentUnavailableMetrics& metrics = Metrics::ContentUnavailable();
        Layout::ContainerSpec spec;
        spec.arrangement = Layout::Arrangement::Overlay;
        spec.alignment = Alignment{HorizontalAlignment::Center, VerticalAlignment::Center};
        spec.fillWidth = true;
        spec.fillHeight = true;
        spec.padding = EdgeInsets::All(metrics.padding);
        Layout::Container(spec, [&] {
            VStack({.alignment = HorizontalAlignment::Center, .spacing = 0.0f}, [&] {
                if (options.symbol) {
                    Image(options.symbol, {.font = Font::System(metrics.symbolSize), .foreground = Foreground::Tertiary});
                    Frame({.height = metrics.symbolSpacing}, [] {});
                }
                Text(title, {.font = Font::System(metrics.titleSize, FontWeight::Bold), .foreground = Foreground::Secondary, .alignment = TextAlignment::Center, .wraps = true});
                if (options.description) {
                    Frame({.height = metrics.descriptionSpacing}, [] {});
                    Text(options.description, {.font = Font::System(metrics.descriptionSize).WithLineHeight(metrics.descriptionLineHeight), .foreground = Foreground::Secondary, .alignment = TextAlignment::Center, .wraps = true});
                }
                if (actions) {
                    Frame({.height = metrics.actionsSpacing}, [] {});
                    HStack({.spacing = 8.0f}, actions);
                }
            });
        });
    }

    void ContentUnavailableSearch(std::string_view text) {
        const std::string title = "No Results for \xE2\x80\x9C" + std::string(text) + "\xE2\x80\x9D";
        ContentUnavailableView(title.c_str(), {.symbol = Symbols::Magnifyingglass, .description = "Check the spelling or try a new search."});
    }
} // namespace Cupertino
