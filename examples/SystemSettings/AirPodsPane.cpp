#include "Settings.h"

#include <cmath>

namespace Examples::Settings {
    using namespace Cupertino;

    struct AirPodsModel {
        int noiseControl = 3;
        int leftHold = 0;
        int rightHold = 1;
        bool cycles[4] = {true, true, true, true};
        int mute = 0;
        int endCall = 1;
    };

    static AirPodsModel& Model() {
        static AirPodsModel model;
        return model;
    }

    static const char* const ListeningModes[] = {"Off", "Transparency", "Adaptive", "Noise Cancellation"};
    static const char* const HoldActions[] = {"Noise Control", "Siri", "Off"};
    static const char* const Presses[] = {"Press Once", "Press Twice"};

    // The listening modes' glyphs (AirPods @2x), in a 20 pt frame: a head and shoulders with a gray ring for Off, a
    // ring of dots for Transparency, two sparkles for Adaptive and a dark ring for Noise Cancellation.
    static void PaintListeningMode(ImDrawList* draw, const ImRect& frame, int mode) {
        const float unit = frame.GetWidth() / 20.0f;
        const DrawingSpace space = {frame.Min, unit};
        const Palette& colors = Theme::Colors();
        const ImU32 ink = colors.label.Packed();
        const bool adaptive = mode == 2;
        const ImVec2 head = adaptive ? space.At(8.5f, 9.0f) : space.At(10.0f, 7.5f);
        draw->AddCircleFilled(head, 3.3f * unit, ink, 24);
        const ImVec2 shoulders = adaptive ? space.At(8.5f, 19.0f) : space.At(10.0f, 18.5f);
        draw->PathEllipticalArcTo(shoulders, ImVec2(7.0f * unit, 6.0f * unit), 0.0f, IM_PI, 2.0f * IM_PI, 24);
        draw->PathFillConvex(ink);
        if (mode == 0 || mode == 3) {
            draw->PathArcTo(head, 6.2f * unit, 0.75f * IM_PI, 2.25f * IM_PI, 32);
            draw->PathStroke(mode == 0 ? colors.tertiaryLabel.Packed() : ink, ImDrawFlags_None, 1.1f * unit);
        } else if (mode == 1) {
            for (int i = 0; i < 10; ++i) {
                const float angle = 0.8f * IM_PI + float(i) * 0.155f * IM_PI;
                draw->AddCircleFilled(head + ImVec2(std::cos(angle), std::sin(angle)) * 6.3f * unit, 0.8f * unit, ink, 8);
            }
        } else {
            const auto sparkle = [&](ImVec2 center, float radius) {
                const float waist = radius * 0.28f;
                const ImVec2 points[] = {center + ImVec2(0.0f, -radius), center + ImVec2(waist, -waist), center + ImVec2(radius, 0.0f), center + ImVec2(waist, waist), center + ImVec2(0.0f, radius), center + ImVec2(-waist, waist), center + ImVec2(-radius, 0.0f), center + ImVec2(-waist, -waist)};
                draw->AddConcavePolyFilled(points, IM_ARRAYSIZE(points), ink);
            };
            sparkle(space.At(15.5f, 6.5f), 2.9f * unit);
            sparkle(space.At(12.4f, 3.0f), 1.8f * unit);
        }
    }

    // L and R cut out of discs in the label color before the press-and-hold pop-ups, dark ones in light mode.
    static void SideBadge(const char* side) {
        Canvas(ImVec2(13.0f, 13.0f), [side](ImDrawList* draw, const ImRect& rect) {
            Draw::FillCircle(draw, rect.GetCenter(), rect.GetWidth() * 0.5f, Theme::Colors().label);
            Typography::Draw(draw, Font::System(7.5f, FontWeight::Semibold), rect, Environment().IsDark() ? Rgba::Black(0.85f) : Rgba::White(1.0f), side, TextAlignment::Center);
        });
    }

    // One side's press and hold: its badge before the pop-up.
    static void PressAndHold(const char* side, const char* id, int* action) {
        HStack({.spacing = 11.0f}, [&] {
            SideBadge(side);
            Picker(id, action, HoldActions);
        });
    }

    // A listening mode to cycle through: its glyph, its name in Callout and a checkbox 3.75 pt under it,
    // centered in a quarter of the row.
    static void ListeningModeColumn(int mode, bool* on) {
        Frame({.maxWidth = Infinity}, [&] {
            VStack({.alignment = HorizontalAlignment::Center, .spacing = 0.0f}, [&] {
                Canvas(ImVec2(20.0f, 20.0f), [mode](ImDrawList* draw, const ImRect& rect) { PaintListeningMode(draw, rect, mode); });
                Text(mode == 3 ? "Noise\nCancellation" : ListeningModes[mode], {.font = Font::Style(TextStyle::Callout), .alignment = TextAlignment::Center, .wraps = true});
                Padding(EdgeInsets{3.75f, 0.0f, 0.0f, 0.0f}, [&] {
                    ImGui::PushID(mode);
                    Toggle("##cycle", on, {.style = ToggleStyle::Checkbox});
                    ImGui::PopID();
                });
            });
        });
    }

    // Bluetooth > AirPods Pro: the name, noise control, press and hold with the modes it cycles through, and call
    // controls (AirPods @2x).
    void AirPodsPane() {
        AirPodsModel& model = Model();
        Form([&] {
            // This pane starts 3 pt further down than the others (AirPods @2x).
            Padding(EdgeInsets{3.0f, 0.0f, 0.0f, 0.0f}, [] {
                Section([] { LabeledContent("Name", [] { Text("AirPods Pro"); }); });
            });
            Section([&] {
                Picker("Noise Control", &model.noiseControl, ListeningModes, {.itemImage = PaintListeningMode, .itemImageSize = ImVec2(13.0f, 13.0f)});
            });
            Section([&] {
                LabeledContent("Press and Hold", [&] {
                    HStack({.spacing = 17.0f}, [&] {
                        PressAndHold("L", "##left", &model.leftHold);
                        PressAndHold("R", "##right", &model.rightHold);
                    });
                });
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 4.25f}, [&] {
                    Text("Press and Hold to Cycle Between");
                    // Four equal columns, 4 pt in from the row's text.
                    Padding(EdgeInsets::Symmetric(4.0f, 0.0f), [&] {
                        HStack({.spacing = 0.0f}, [&] {
                            for (int mode = 0; mode < 4; ++mode)
                                ListeningModeColumn(mode, &model.cycles[mode]);
                        });
                    });
                });
            });
            Section({.header = "Call Controls"}, [&] {
                LabeledContent("Answer Call", [] { Text("Press Once", {.foreground = Foreground::Tertiary}); });
                Picker("Mute & Unmute", &model.mute, Presses);
                Picker("End Call", &model.endCall, Presses);
            });
        });
    }
} // namespace Examples::Settings
