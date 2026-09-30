#include "Settings.h"

#include <array>
#include <cmath>
#include <string>

// Sidebar glyphs of macOS 15 that SF Symbols does not have, measured on the native @2x sidebars. Each painter gets the
// symbol frame of its plate (26 of the plate's 40 units) and works in fractions of the plate.
namespace Examples::Settings {
    using namespace Cupertino;

    // The plate of a symbol frame, in fractions of the plate.
    static DrawingSpace PlateSpace(const ImRect& frame) {
        const float size = frame.GetWidth() * 40.0f / 26.0f;
        return {frame.GetCenter() - ImVec2(size, size) * 0.5f, size};
    }

    // Half a disc on the side of the angle range, filled.
    static void FillHalfDisc(ImDrawList* draw, ImVec2 center, float radius, float from, float to, Rgba color) {
        draw->PathArcTo(center, radius, from, to, 24);
        draw->PathFillConvex(color.Packed());
    }

    void PaintBluetoothRune(ImDrawList* draw, const ImRect& frame, Rgba color) {
        const ImVec2 center = frame.GetCenter() - ImVec2(frame.GetWidth() * 0.023f, 0.0f);
        const float half_width = frame.GetWidth() * 0.26f;
        const float half_height = frame.GetHeight() * 0.37f;
        const auto at = [&](float x, float y) { return center + ImVec2(x * half_width, y * half_height); };
        const ImVec2 points[] = {at(-1.0f, -0.46f), at(1.0f, 0.46f), at(0.0f, 1.0f), at(0.0f, -1.0f), at(1.0f, -0.46f), at(-1.0f, 0.46f)};
        draw->AddPolyline(points, IM_ARRAYSIZE(points), color.Packed(), frame.GetWidth() * 0.096f, ImDrawFlags_None);
    }

    // A globe on a network stand: the stem ends in a node on a horizontal bar.
    void PaintVpn(ImDrawList* draw, const ImRect& frame, Rgba color) {
        const DrawingSpace plate = PlateSpace(frame);
        const float globe = plate.Length(0.43f);
        const ImVec2 globe_center = plate.At(0.5f, 0.375f);
        Typography::DrawSymbol(draw, Symbols::Globe, Font::System(Pt(globe) / 0.9f), ImRect(globe_center - ImVec2(globe, globe), globe_center + ImVec2(globe, globe)), color);
        draw->AddLine(plate.At(0.5f, 0.6f), plate.At(0.5f, 0.8f), color.Packed(), plate.Length(0.05f));
        draw->AddLine(plate.At(0.225f, 0.8f), plate.At(0.775f, 0.8f), color.Packed(), plate.Length(0.035f));
        draw->AddCircleFilled(plate.At(0.5f, 0.8f), plate.Length(0.075f), color.Packed(), 16);
    }

    // A ring around a disc split into a light left and a dark right half, with the halves swapped in its middle.
    void PaintAppearance(ImDrawList* draw, const ImRect& frame, Rgba color) {
        const DrawingSpace plate = PlateSpace(frame);
        const ImVec2 center = plate.At(0.5f, 0.5f);
        draw->AddCircle(center, plate.Length(0.3125f), color.Packed(), 48, plate.Length(0.0375f));
        const float inner = plate.Length(0.125f);
        const float outer = plate.Length(0.29f);
        draw->PathArcTo(center, (inner + outer) * 0.5f, IM_PI * 0.5f, IM_PI * 1.5f, 32);
        draw->PathStroke(color.Packed(), ImDrawFlags_None, outer - inner);
        FillHalfDisc(draw, center, inner, -IM_PI * 0.5f, IM_PI * 0.5f, color);
    }

    // A screen with the moon and stars, 70% of the plate wide.
    void PaintScreenSaver(ImDrawList* draw, const ImRect& frame, Rgba color) {
        const DrawingSpace plate = PlateSpace(frame);
        const float stroke = plate.Length(0.035f);
        const ImVec2 half(stroke * 0.5f, stroke * 0.5f);
        draw->AddRect(plate.At(0.15f, 0.225f) + half, plate.At(0.85f, 0.75f) - half, color.Packed(), 0.0f, ImDrawFlags_None, stroke);
        const float glyph = plate.Length(0.3f);
        const ImVec2 glyph_center = plate.At(0.465f, 0.49f);
        Typography::DrawSymbol(draw, Symbols::MoonStarsFill, Font::System(Pt(glyph)), ImRect(glyph_center - ImVec2(glyph, glyph) * 0.5f, glyph_center + ImVec2(glyph, glyph) * 0.5f), color);
    }

    // A flower of eight outlined petals around an outlined center.
    void PaintWallpaper(ImDrawList* draw, const ImRect& frame, Rgba color) {
        const DrawingSpace plate = PlateSpace(frame);
        const ImVec2 center = plate.At(0.5f, 0.49f);
        const float stroke = plate.Length(0.036f);
        for (int i = 0; i < 8; ++i) {
            const float angle = float(i) * IM_PI * 0.25f;
            draw->AddCircle(center + ImVec2(std::cos(angle), std::sin(angle)) * plate.Length(0.23f), plate.Length(0.128f), color.Packed(), 24, stroke);
        }
        draw->AddCircle(center, plate.Length(0.09f), color.Packed(), 24, stroke);
    }

    // Apple Intelligence: an angular gradient running clockwise from red at the top through pink, purple, blue and cyan
    // to a pale yellow and orange, lighter toward the middle, under a white rosette of six loops (colors sampled @2x).
    void PaintAppleIntelligence(ImDrawList* draw, const ImRect& frame, Rgba) {
        static constexpr Rgba Hues[] = {
            Rgba::Hex(0xF97353), Rgba::Hex(0xFE5F58), Rgba::Hex(0xFD4272), Rgba::Hex(0xF9559B), Rgba::Hex(0xF46CC8), Rgba::Hex(0xEE89F2),
            Rgba::Hex(0xD39AFC), Rgba::Hex(0xAAA4FA), Rgba::Hex(0x60A6FB), Rgba::Hex(0x53BEF1), Rgba::Hex(0x86C9E6), Rgba::Hex(0xE8CA90),
            Rgba::Hex(0xF3DC92), Rgba::Hex(0xFFBD2E), Rgba::Hex(0xF5AA37), Rgba::Hex(0xF79245),
        };
        const DrawingSpace plate = PlateSpace(frame);
        const ImRect bounds(plate.At(0.0f, 0.0f), plate.At(1.0f, 1.0f));
        const CornerRadii radii(plate.Length(0.225f));
        Draw::FillRoundedRect(draw, bounds, radii, Rgba::Hex(0xE9A0B0));
        // The fan runs to the plate's contour, a little inside its antialiased edge.
        ImVector<ImVec2> contour;
        const float inset = 0.75f;
        Draw::RoundedRectContour(contour, ImRect(bounds.Min + ImVec2(inset, inset), bounds.Max - ImVec2(inset, inset)), radii.Offset(-inset), CornerStyle::Continuous, 8);
        const ImVec2 center = bounds.GetCenter();
        const int hues = IM_ARRAYSIZE(Hues);
        Draw::FillShaded(draw, center, Rgba::Hex(0xCFA9B8), std::span<const ImVec2>(contour.Data, size_t(contour.Size)), [&](ImVec2 point) {
            const float turn = (std::atan2(point.x - center.x, center.y - point.y) / (2.0f * IM_PI) + 1.0f) * float(hues);
            const int index = int(turn) % hues;
            return Blend::Mix(Hues[index], Hues[(index + 1) % hues], turn - std::floor(turn));
        });
        // Six loops around the middle, drawn as one closed line.
        std::array<ImVec2, 96> rosette;
        for (size_t i = 0; i < rosette.size(); ++i) {
            const float t = 2.0f * IM_PI * float(i) / float(rosette.size());
            const float radius = 0.235f + 0.075f * std::cos(6.0f * t);
            rosette[i] = plate.At(0.5f + radius * std::sin(t), 0.5f - radius * std::cos(t));
        }
        draw->AddPolyline(rosette.data(), int(rosette.size()), Rgba::White(1.0f).Packed(), ImDrawFlags_Closed, plate.Length(0.055f));
    }

    // A padlock over a row of four password dots (Privacy & Security @2x: 22 x 26 of the plate's 40 units).
    void PaintLockScreen(ImDrawList* draw, const ImRect& frame, Rgba color) {
        const DrawingSpace plate = PlateSpace(frame);
        const float stroke = plate.Length(0.055f);
        draw->PathArcTo(plate.At(0.5f, 0.285f), plate.Length(0.105f), IM_PI, 2.0f * IM_PI, 16);
        draw->PathLineTo(plate.At(0.605f, 0.4f));
        draw->PathStroke(color.Packed(), ImDrawFlags_None, stroke);
        draw->PathLineTo(plate.At(0.395f, 0.4f));
        draw->PathLineTo(plate.At(0.395f, 0.285f));
        draw->PathStroke(color.Packed(), ImDrawFlags_None, stroke);
        Draw::FillRoundedRect(draw, ImRect(plate.At(0.34f, 0.38f), plate.At(0.66f, 0.63f)), CornerRadii(plate.Length(0.04f)), color);
        for (int i = 0; i < 4; ++i)
            draw->AddCircleFilled(plate.At(0.26f + 0.16f * float(i), 0.775f), plate.Length(0.042f), color.Packed(), 12);
    }

    // AppleCare & Warranty: the apple in a deep red on a white plate, 10 by 12 pt on a 20 pt plate (General @2x, Ars
    // Technica).
    void PaintAppleCare(ImDrawList* draw, const ImRect& frame, Rgba) {
        Typography::DrawSymbol(draw, Symbols::AppleLogo, Font::System(Pt(frame.GetWidth())), frame, Rgba::Hex(0xBE2E37));
    }

    // AirDrop: a dot in three rings that open 30 degrees to either side of straight down, on a white plate (General @2x,
    // Ars Technica).
    void PaintAirDrop(ImDrawList* draw, const ImRect& frame, Rgba color) {
        const DrawingSpace plate = PlateSpace(frame);
        const ImVec2 center = plate.At(0.5f, 0.49f);
        const float gap = IM_PI / 6.0f;
        draw->AddCircleFilled(center, plate.Length(0.05f), color.Packed(), 16);
        for (const float radius : {0.125f, 0.225f, 0.325f}) {
            draw->PathArcTo(center, plate.Length(radius), IM_PI * 0.5f + gap, IM_PI * 2.5f - gap, 40);
            draw->PathStroke(color.Packed(), ImDrawFlags_None, plate.Length(0.05f));
        }
    }

    // FileVault: a house under a roof line with a chimney, a safe's dial in its middle.
    void PaintFileVault(ImDrawList* draw, const ImRect& frame, Rgba color) {
        const DrawingSpace plate = PlateSpace(frame);
        const float stroke = plate.Length(0.05f);
        const ImVec2 roof[] = {plate.At(0.19f, 0.47f), plate.At(0.5f, 0.2f), plate.At(0.81f, 0.47f)};
        draw->AddPolyline(roof, IM_ARRAYSIZE(roof), color.Packed(), ImDrawFlags_None, stroke);
        draw->AddLine(plate.At(0.7f, 0.3f), plate.At(0.7f, 0.19f), color.Packed(), stroke);
        Draw::FillRoundedRect(draw, ImRect(plate.At(0.3f, 0.43f), plate.At(0.7f, 0.8f)), CornerRadii(plate.Length(0.05f)), color);
        const ImVec2 dial = plate.At(0.5f, 0.605f);
        draw->AddCircleFilled(dial, plate.Length(0.14f), Rgba::Hex(0xA1A1A4).Packed(), 24);
        for (int i = 0; i < 8; ++i) {
            const float angle = float(i) * IM_PI * 0.25f;
            draw->AddCircleFilled(dial + ImVec2(std::cos(angle), std::sin(angle)) * plate.Length(0.09f), plate.Length(0.022f), color.Packed(), 8);
        }
    }

    // macOS Sequoia's icon over the whole plate, in a white rim: the crest of the wallpaper, deep blue at the lower left
    // through purple to peach at the upper right (Software Update @2x, colors sampled).
    void PaintSequoia(ImDrawList* draw, const ImRect& frame, Rgba) {
        static constexpr Rgba Stops[] = {Rgba::Hex(0x012C9B), Rgba::Hex(0x0355BD), Rgba::Hex(0x1082D7), Rgba::Hex(0x9F76B5), Rgba::Hex(0xF58A54), Rgba::Hex(0xFBBB7B), Rgba::Hex(0xFCD397)};
        const DrawingSpace plate = PlateSpace(frame);
        const ImRect rect(plate.At(0.0f, 0.0f), plate.At(1.0f, 1.0f));
        const ImVec2 center = rect.GetCenter();
        const float radius = rect.GetWidth() * 0.5f;
        Draw::DropShadow(draw, rect, CornerRadii(radius), Shadow{Rgba::Black(0.2f), ImVec2(0.0f, 0.5f), 1.0f}, CornerStyle::Circular);
        Draw::FillCircle(draw, center, radius, Rgba::White(1.0f));
        const float inner = radius - Px(1.5f);
        std::array<ImVec2, 64> contour;
        for (size_t i = 0; i < contour.size(); ++i) {
            const float angle = 2.0f * IM_PI * float(i) / float(contour.size());
            contour[i] = center + ImVec2(std::cos(angle), std::sin(angle)) * inner;
        }
        const int last = IM_ARRAYSIZE(Stops) - 1;
        const auto color_at = [&](ImVec2 point) {
            // Along the diagonal from the lower left to the upper right.
            const float t = ImSaturate(0.5f + ((point.x - center.x) - (point.y - center.y)) / (2.8284f * inner));
            const float position = t * float(last);
            const int index = ImMin(int(position), last - 1);
            return Blend::Mix(Stops[index], Stops[index + 1], position - float(index));
        };
        Draw::FillCircle(draw, center, inner, color_at(center));
        Draw::FillShaded(draw, center, color_at(center), contour, color_at);
    }

    const Icon& PaneIcon(Pane pane) {
        static const Icon none;
        static const Icon wifi = {Symbols::Wifi, IconPlate::Blue, SymbolScale::Large, 0.92f};
        static const Icon bluetooth = {.plate = IconPlate::Blue, .paint = PaintBluetoothRune};
        static const Icon network = {Symbols::Network, IconPlate::Blue};
        static const Icon battery = {Symbols::Battery100percent, IconPlate::Green, SymbolScale::Medium};
        static const Icon vpn = {.plate = IconPlate::Blue, .paint = PaintVpn};
        static const Icon general = {Symbols::Gear, IconPlate::Gray, SymbolScale::Large, 0.92f};
        static const Icon accessibility = {Symbols::Accessibility, IconPlate::Blue};
        static const Icon appearance = {.plate = IconPlate::Black, .paint = PaintAppearance};
        static const Icon control_center = {Symbols::Switch2, IconPlate::Gray, SymbolScale::Large, 0.9f};
        static const Icon desktop_dock = {Symbols::MenubarDockRectangle, IconPlate::Black, SymbolScale::Large, 0.85f};
        static const Icon displays = {Symbols::SunMaxFill, IconPlate::Blue, SymbolScale::Large, 0.9f};
        static const Icon screen_saver = {.plate = IconPlate::Cyan, .paint = PaintScreenSaver};
        static const Icon siri = PictureIcon("sidebar/siri", {.paint = PaintAppleIntelligence});
        static const Icon spotlight = {Symbols::Magnifyingglass, IconPlate::Gray, SymbolScale::Large, 0.85f};
        static const Icon wallpaper = {.plate = IconPlate::Cyan, .paint = PaintWallpaper};
        static const Icon notifications = {Symbols::BellBadgeFill, IconPlate::Red, SymbolScale::Large, 0.9f};
        static const Icon sound = {Symbols::SpeakerWave3Fill, IconPlate::Pink, SymbolScale::Large, 0.78f};
        static const Icon focus = {Symbols::MoonFill, IconPlate::Purple, SymbolScale::Large, 0.83f};
        static const Icon screen_time = {Symbols::Hourglass, IconPlate::Purple, SymbolScale::Large, 0.97f};
        static const Icon family = {Symbols::Person2Fill, IconPlate::White};
        static const Icon lock_screen = {.plate = IconPlate::Black, .paint = PaintLockScreen};
        static const Icon privacy = {Symbols::HandRaisedFill, IconPlate::Blue, SymbolScale::Large, 0.935f};
        static const Icon touch_id = PictureIcon("sidebar/touch-id", {.symbol = Symbols::Touchid, .plate = IconPlate::White, .color = Rgba::Hex(0xF7385A)});
        static const Icon login_password = {Symbols::LockFill, IconPlate::Gray, SymbolScale::Large, 0.8f};
        static const Icon users = {Symbols::Person2Fill, IconPlate::Blue, SymbolScale::Large, 0.82f};
        static const Icon internet_accounts = {Symbols::At, IconPlate::Blue, SymbolScale::Large, 0.91f};
        static const Icon game_center = PictureIcon("sidebar/game-center", {.symbol = Symbols::GamecontrollerFill, .plate = IconPlate::White, .color = Rgba::Hex(0x8E5AF7)});
        static const Icon icloud = PictureIcon("sidebar/icloud", {.symbol = Symbols::IcloudFill, .plate = IconPlate::White, .color = Rgba::Hex(0x3A8EF6)});
        static const Icon airpods = {Symbols::Airpodspro, IconPlate::Gray, SymbolScale::Large, 0.78f};
        static const Icon wallet = PictureIcon("sidebar/wallet", {Symbols::WalletPassFill, IconPlate::Black});
        static const Icon keyboard = {Symbols::KeyboardFill, IconPlate::Gray, SymbolScale::Large, 0.85f};
        static const Icon mouse = {Symbols::MagicmouseFill, IconPlate::Gray};
        static const Icon trackpad = {Symbols::RectangleAndHandPointUpLeftFill, IconPlate::Gray, SymbolScale::Large, 0.87f};
        static const Icon printers = {Symbols::PrinterFill, IconPlate::Gray, SymbolScale::Large, 0.85f};
        switch (pane) {
            case Pane::WiFi:
                return wifi;
            case Pane::Bluetooth:
                return bluetooth;
            case Pane::Network:
                return network;
            case Pane::Battery:
                return battery;
            case Pane::Vpn:
                return vpn;
            case Pane::General:
            case Pane::Sharing:
            case Pane::SoftwareUpdate:
                return general;
            case Pane::Accessibility:
                return accessibility;
            case Pane::Appearance:
                return appearance;
            case Pane::ControlCenter:
                return control_center;
            case Pane::DesktopAndDock:
                return desktop_dock;
            case Pane::Displays:
                return displays;
            case Pane::ScreenSaver:
                return screen_saver;
            case Pane::Siri:
                return siri;
            case Pane::Spotlight:
                return spotlight;
            case Pane::Wallpaper:
                return wallpaper;
            case Pane::Notifications:
                return notifications;
            case Pane::Sound:
                return sound;
            case Pane::Focus:
                return focus;
            case Pane::ScreenTime:
                return screen_time;
            case Pane::Family:
                return family;
            case Pane::LockScreen:
                return lock_screen;
            case Pane::PrivacyAndSecurity:
                return privacy;
            case Pane::TouchIdAndPassword:
                return touch_id;
            case Pane::LoginPassword:
                return login_password;
            case Pane::UsersAndGroups:
                return users;
            case Pane::InternetAccounts:
                return internet_accounts;
            case Pane::GameCenter:
                return game_center;
            case Pane::ICloud:
                return icloud;
            case Pane::AirPods:
                return airpods;
            case Pane::WalletAndApplePay:
                return wallet;
            case Pane::Keyboard:
                return keyboard;
            case Pane::Mouse:
                return mouse;
            case Pane::Trackpad:
                return trackpad;
            case Pane::PrintersAndScanners:
                return printers;
        }
        return none;
    }

    Icon SidebarIcon(const Mac& mac, Pane pane) {
        for (const std::vector<SidebarEntry>& group : mac.groups) {
            for (const SidebarEntry& entry : group) {
                if (entry.pane == pane)
                    return entry.icon;
            }
        }
        return PaneIcon(pane);
    }
} // namespace Examples::Settings
