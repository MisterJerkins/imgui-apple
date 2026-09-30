#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    struct PrintersModel {
        int defaultPrinter = 0;
        int paperSize = 0;
    };

    // The generic printer image (Printers & Scanners @2x), 31 pt: a sheet in the feeder, the silver body with its
    // lights, and a printed page in the output tray.
    static void PaintPrinter(ImDrawList* draw, const ImRect& rect) {
        const DrawingSpace space = PointSpace(rect);
        Draw::FillRect(draw, space.Box(7.0f, 0.5f, 23.5f, 11.0f), Rgba::Hex(0xF4F4F4));
        Draw::StrokeRoundedRect(draw, space.Box(7.0f, 0.5f, 23.5f, 11.0f), CornerRadii(0.0f), Rgba::Black(0.3f), Px(0.5f));
        Draw::DropShadow(draw, space.Box(0.5f, 9.5f, 30.5f, 28.0f), CornerRadii(Px(3.0f)), Shadow{Rgba::Black(0.3f), ImVec2(0.0f, 0.5f), 1.5f});
        Draw::FillVerticalGradient(draw, space.Box(0.5f, 9.5f, 30.5f, 28.0f), CornerRadii(Px(3.0f)), Rgba::Hex(0xE2E2E2), Rgba::Hex(0x8E8E8E));
        Draw::FillRect(draw, space.Box(3.0f, 17.5f, 28.0f, 19.0f), Rgba::Hex(0x3A3A3A));
        for (const float x : {25.0f, 26.75f})
            Draw::FillCircle(draw, space.At(x, 14.0f), Px(0.6f), Rgba::Hex(0x4A4A4A));
        Draw::FillRect(draw, space.Box(5.5f, 18.5f, 25.5f, 30.5f), Rgba::Hex(0xEEF1F6));
        Draw::StrokeRoundedRect(draw, space.Box(5.5f, 18.5f, 25.5f, 30.5f), CornerRadii(0.0f), Rgba::Black(0.25f), Px(0.5f));
        Draw::FillRect(draw, space.Box(8.0f, 21.0f, 17.0f, 28.5f), Rgba::Hex(0xB9CBE6));
        draw->AddTriangleFilled(space.At(17.0f, 21.0f), space.At(23.0f, 21.0f), space.At(17.0f, 28.5f), Rgba::Hex(0xF2B33D).Packed());
    }

    // Printers & Scanners as 512 Pixels captured it: the defaults, the printer with its state, and the button that adds
    // a printer.
    void PrintersPane() {
        static PrintersModel model;
        Form([] {
            Section([] {
                Picker("Default printer", &model.defaultPrinter, {"Last Printer Used", "Brother HL-2270DW series"});
                Picker("Default paper size", &model.paperSize, {"US Letter", "US Legal", "A4", "A5", "Envelope #10"});
            });
            Section({.header = "Printers"}, [] {
                NavigationLink("Brother HL-2270DW series", [] {
                    HStack({.spacing = 11.5f}, [] {
                        Canvas(ImVec2(31.0f, 31.0f), PaintPrinter);
                        VStack({.alignment = HorizontalAlignment::Leading, .spacing = 2.0f}, [] {
                            Text("Brother HL-2270DW series");
                            ServiceStatus(Theme::SystemGreen(), "Idle, Last Used", TextStyle::Footnote);
                        });
                    });
                });
            });
            // The buttons keep 20 pt from the last section and 10 pt from each other.
            TrailingButtons({.top = 10.0f, .spacing = 10.0f}, [] {
                Button("Add Printer, Scanner, or Fax\xE2\x80\xA6");
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
