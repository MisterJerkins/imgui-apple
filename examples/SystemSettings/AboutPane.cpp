#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    // The display of About's MacBook Air: the default blue, lighter toward the bottom (512 Pixels @2x).
    static void PaintBlueScreen(ImDrawList* draw, const ImRect& rect) {
        Draw::FillVerticalGradient(draw, rect, CornerRadii(0.0f), Rgba::Hex(0x3A8CCC), Rgba::Hex(0x5CADEF));
    }

    // The serial number as a capture hides it: a row of 9 pt gray tiles.
    static void PaintHiddenSerial(ImDrawList* draw, const ImRect& rect) {
        static const float shades[] = {0.052f, 0.039f, 0.026f, 0.056f, 0.444f, 0.474f, 0.138f, 0.336f, 0.358f, 0.0f, 0.009f};
        const float tile = Px(9.0f);
        for (int i = 0; i < IM_ARRAYSIZE(shades); ++i)
            Draw::FillRect(draw, ImRect(rect.Min.x + tile * float(i), rect.Min.y, rect.Min.x + tile * float(i + 1), rect.Max.y), Rgba::Black(shades[i]));
    }

    // The built-in display: a black-framed blue screen.
    static void PaintDisplay(ImDrawList* draw, const ImRect& frame, Rgba) {
        const ImVec2 center = frame.GetCenter();
        const ImRect screen(center - Px(ImVec2(9.5f, 6.5f)), center + Px(ImVec2(9.5f, 6.5f)));
        Draw::FillRoundedRect(draw, screen, CornerRadii(Px(1.5f)), Rgba::Hex(0x1C1C1E));
        Draw::FillVerticalGradient(draw, ImRect(screen.Min + Px(ImVec2(1.0f, 1.0f)), screen.Max - Px(ImVec2(1.0f, 1.0f))), CornerRadii(Px(0.5f)), Rgba::Hex(0x3A8CCC), Rgba::Hex(0x5CADEF));
    }

    // About, General's page, as 512 Pixels captured it: the Mac and its name, what it is made of, macOS and the display.
    void AboutPane(const Mac& mac) {
        Form([&] {
            Padding(EdgeInsets{51.5f, 0.0f, 9.5f, 0.0f}, [] {
                Frame({.maxWidth = Infinity}, [] {
                    VStack({.spacing = 26.0f}, [] {
                        Canvas(ImVec2(100.0f, 62.0f), [](ImDrawList* draw, const ImRect& rect) { PaintMacBookAir(draw, rect, PaintBlueScreen); });
                        Text("MacBook Air", {.font = Font::System(26.0f, FontWeight::Bold)});
                    });
                });
            });
            Section([&] {
                LabeledContent("Name", [&] { Text(mac.computerName); });
                LabeledContent("Chip", "Apple M1");
                LabeledContent("Memory", "8 GB");
                LabeledContent("Serial number", [&] {
                    if (mac.serialNumber)
                        Text(mac.serialNumber, {.foreground = Foreground::Secondary});
                    else
                        Canvas(ImVec2(99.0f, 9.0f), PaintHiddenSerial);
                });
                LabeledContent("Coverage Expired", [] { Button("Details\xE2\x80\xA6"); });
            });
            Section({.header = "macOS"}, [] {
                LabeledContent("macOS Sequoia", Icon{.paint = PaintSequoia}, [] { Text("Version 15.0", {.foreground = Foreground::Secondary}); });
            });
            Section({.header = "Displays"}, [] {
                LabeledContent("Built-in Retina Display", Icon{.paint = PaintDisplay}, [] { Text("13.3-inch (2560 \xC3\x97 1600)", {.foreground = Foreground::Secondary}); });
            });
            TrailingButtons([] {
                Button("Display Settings\xE2\x80\xA6");
            });
            Section({.header = "Storage"}, [] {
                LabeledContent("Macintosh HD", [] { Text("205.89 GB available of 245.11 GB", {.foreground = Foreground::Secondary}); });
            });
            TrailingButtons([] {
                Button("Storage Settings\xE2\x80\xA6");
            });
            TrailingButtons([] {
                Button("System Report\xE2\x80\xA6");
            });
        });
    }
} // namespace Examples::Settings
