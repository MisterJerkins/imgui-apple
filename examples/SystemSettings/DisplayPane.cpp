#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    // The state on Apple's Accessibility > Display guide art.
    struct DisplayModel {
        bool shakeToLocate = true;
        float pointerSize = 0.0f;
        Rgba outline = Rgba::White(1.0f);
        Rgba fill = Rgba::Black(1.0f);
        bool colorFilters = false;
        int filterType = 4;
    };

    static DisplayModel& Model() {
        static DisplayModel model;
        return model;
    }

    // The Color Filters preview: a row of colored pencils standing tip up, each with a wooden cone and cylinder
    // shading, cut off by the bottom of the frame.
    static void DrawPencils(ImDrawList* draw, const ImRect& frame) {
        static const unsigned Leads[] = {0xE8352B, 0xF07A1A, 0xF5E02A, 0x2EDB2E, 0x0F7A1A, 0x1A8A8A, 0x5AB8F0, 0x1A30E0, 0x6A1AE0, 0xB05AF0, 0xF01AE0, 0x8A1A7A, 0x7A1A3A, 0x7A3A10};
        const int count = IM_ARRAYSIZE(Leads);
        const float width = Px(25.2f);
        const float left = frame.GetCenter().x - width * float(count) * 0.5f;
        const float tip = frame.Min.y + Px(10.0f);
        const float lead = tip + Px(10.0f);
        const float collar = tip + Px(34.0f);
        const ImU32 wood_light = Rgba::Hex(0xF3D9B5).Packed();
        const ImU32 wood_dark = Rgba::Hex(0xD9B383).Packed();
        for (int i = 0; i < count; ++i) {
            const Rgba color = Rgba::Hex(Leads[i]);
            const float x0 = left + width * float(i);
            const float x1 = x0 + width;
            const float middle = (x0 + x1) * 0.5f;
            // Body: lit from the left, darker toward the right edge.
            const ImU32 light = Blend::Mix(color, Rgba::White(1.0f), 0.25f).Packed();
            const ImU32 dark = Blend::Mix(color, Rgba::Black(1.0f), 0.3f).Packed();
            draw->AddRectFilledMultiColor(ImVec2(x0, collar), ImVec2(middle, frame.Max.y), light, color.Packed(), color.Packed(), light);
            draw->AddRectFilledMultiColor(ImVec2(middle, collar), ImVec2(x1, frame.Max.y), color.Packed(), dark, dark, color.Packed());
            // Sharpened wood and the lead.
            const float lead_half = width * 0.18f;
            draw->AddTriangleFilled(ImVec2(x0, collar), ImVec2(middle - lead_half, lead), ImVec2(middle, collar), wood_light);
            draw->AddTriangleFilled(ImVec2(middle, collar), ImVec2(middle + lead_half, lead), ImVec2(x1, collar), wood_dark);
            draw->AddTriangleFilled(ImVec2(middle - lead_half, lead), ImVec2(middle + lead_half, lead), ImVec2(middle, collar), wood_light);
            draw->AddTriangleFilled(ImVec2(middle - lead_half, lead), ImVec2(middle, tip), ImVec2(middle + lead_half, lead), color.Packed());
        }
    }

    // A row as wide and tall as its section slot, without the row insets, like SwiftUI's .listRowInsets(EdgeInsets()).
    static void EdgeToEdgeRow(float height, void (*paint)(ImDrawList* draw, const ImRect& frame)) {
        Layout::Placement placement;
        placement.size = ImVec2(Layout::FullProposal().x, Px(height));
        placement.ignoresChildInsets = true;
        const ImRect frame = Layout::Place(placement);
        if (Layout::IsMeasuring())
            return;
        ImDrawList* draw = ImGui::GetWindowDrawList();
        draw->PushClipRect(frame.Min, frame.Max, true);
        paint(draw, frame);
        draw->PopClipRect();
    }

    void DisplayPane() {
        DisplayModel& model = Model();
        Form([&] {
            Section({.header = "Pointer"}, [&] {
                Toggle("Shake mouse pointer to locate", &model.shakeToLocate, {.description = "Quickly move the mouse pointer back and forth to make it bigger."});
                Slider("Pointer size", &model.pointerSize, 0.0f, 1.0f, {.width = 243.0f, .ticks = 7, .captions = {"Normal", "Large"}});
            });
            Section([&] {
                ColorPicker("Pointer outline color", &model.outline);
                ColorPicker("Pointer fill color", &model.fill);
                TrailingButtons([&] {
                    Button("Reset Colors");
                });
            });
            Section({.header = "Color Filters"}, [&] {
                EdgeToEdgeRow(140.0f, DrawPencils);
                Toggle("Color filters", &model.colorFilters);
                Picker("Filter type", &model.filterType, {"Grayscale", "Red/Green filter (Protanopia)", "Green/Red filter (Deuteranopia)", "Blue/Yellow filter (Tritanopia)", "Color Tint"});
            });
        });
    }
} // namespace Examples::Settings
