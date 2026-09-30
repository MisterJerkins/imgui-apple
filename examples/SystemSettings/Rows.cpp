#include "Settings.h"

#include <string>

namespace Examples::Settings {
    using namespace Cupertino;

    bool PictureLink(std::string_view title, const char* picture, const Icon& fallback, NavigationLinkOptions options) {
        options.icon = PictureIcon(std::string("sidebar/") + picture, fallback);
        return NavigationLink(title, options);
    }

    void TrailingButtons(const TrailingRow& row, const std::function<void()>& buttons) {
        Padding(EdgeInsets{row.top, 0.0f, row.bottom, row.trailing}, [&] {
            HStack({.spacing = row.spacing}, [&] {
                Spacer();
                buttons();
            });
        });
    }

    void TrailingButtons(const std::function<void()>& buttons) {
        TrailingButtons(TrailingRow{}, buttons);
    }

    bool InfoButton(std::string_view name) {
        return Button(("##info-" + std::string(name)).c_str(), {.style = ButtonStyle::Borderless, .symbol = Symbols::InfoCircle});
    }

    bool SwitchWithButton(const char* id, bool* on, const char* button) {
        bool clicked = false;
        HStack({.spacing = 10.5f}, [&] {
            WithControlSize(ControlSize::Mini, [&] { Toggle(id, on, {.style = ToggleStyle::Switch}); });
            Disabled(!*on, [&] { clicked = Button(button); });
        });
        return clicked;
    }

    void Dot(Rgba color, float diameter) {
        Canvas(ImVec2(diameter, diameter), [color](ImDrawList* draw, const ImRect& rect) { Draw::FillCircle(draw, rect.GetCenter(), rect.GetWidth() * 0.5f, color); });
    }

    bool SwitchRow(const char* title, const Icon& icon, bool* on, const ServiceState* state) {
        bool info = false;
        Frame({.height = 30.0f}, [&] {
            HStack({.spacing = 0.0f}, [&] {
                Padding(EdgeInsets{0.0f, 1.0f, 0.0f, 11.0f}, [&] { Image(icon, ImVec2(26.0f, 26.0f)); });
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 2.0f}, [&] {
                    Text(title);
                    if (state)
                        ServiceStatus(state->color, state->text);
                });
                Spacer();
                Padding(EdgeInsets{3.0f, 0.0f, 0.0f, 0.0f}, [&] {
                    HStack({.spacing = 9.0f}, [&] {
                        WithControlSize(ControlSize::Mini, [&] { Toggle((std::string("##") + title).c_str(), on, {.style = ToggleStyle::Switch}); });
                        info = InfoButton(title);
                    });
                });
            });
        });
        return info;
    }

    // The dot's diameter, where its center stands in its column, the column's width and the space before the text by the
    // text's style (@2x).
    void ServiceStatus(Rgba color, const char* state, TextStyle style) {
        const bool body = style == TextStyle::Body;
        const bool footnote = style == TextStyle::Footnote;
        const float dot = body ? 10.0f : footnote ? 7.0f : 8.0f;
        const float center = body ? 7.5f : footnote ? 4.5f : 5.0f;
        const float column = body ? 15.0f : footnote ? 8.0f : 10.0f;
        HStack({.spacing = body ? 3.75f : 3.5f}, [&] {
            Padding(EdgeInsets{0.0f, center - dot * 0.5f, 0.0f, column - center - dot * 0.5f}, [&] { Dot(color, dot); });
            Text(state, {.font = Font::Style(style), .foreground = Foreground::Secondary});
        });
    }
} // namespace Examples::Settings
