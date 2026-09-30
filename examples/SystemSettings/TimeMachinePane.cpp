#include "Settings.h"

#include <vector>

namespace Examples::Settings {
    using namespace Cupertino;

    struct TimeMachineModel {
        std::vector<bool> disks = {false};
    };

    // The pane's header: the app's picture, larger than a pane plate (28 pt with its shadow), and the text 52 pt in with
    // its description in Subheadline (Time Machine @2x).
    static void TimeMachineHeader() {
        HStack({.alignment = VerticalAlignment::Top, .spacing = 11.0f}, [] {
            Padding(EdgeInsets{2.0f, 1.0f, 0.0f, 0.0f}, [] { Image(PictureIcon("pictures/time-machine", {Symbols::ClockArrowCirclepath, IconPlate::Green}), ImVec2(30.0f, 30.0f)); });
            VStack({.alignment = HorizontalAlignment::Leading, .spacing = 2.0f}, [] {
                Text("Time Machine");
                Text("Time Machine backs up your computer and keeps local snapshots and hourly backups for the past 24 hours, daily backups for the past month and weekly backups for all previous months. The oldest backups and any local snapshots are deleted as space is needed.", {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary, .wraps = true});
            });
        });
    }

    // A backup disk in the list: its picture, then its name over what it holds in Subheadline, 2 pt apart; the row keeps a
    // point more under its text than over it.
    static void BackupDisk(const char* name, std::initializer_list<const char*> details) {
        Padding(EdgeInsets{0.0f, 0.0f, 1.0f, 0.0f}, [&] {
            HStack({.alignment = VerticalAlignment::Top, .spacing = 8.5f}, [&] {
                Padding(EdgeInsets{0.0f, 2.0f, 0.0f, 0.0f}, [] { Image(PictureIcon("pictures/time-machine-disk", {.symbol = Symbols::ExternaldriveFillBadgeTimemachine, .color = Theme::Colors().secondaryLabel}), ImVec2(32.0f, 32.5f)); });
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 2.0f}, [&] {
                    Text(name);
                    for (const char* detail : details)
                        Text(detail, {.font = Font::Style(TextStyle::Subheadline), .foreground = Foreground::Secondary});
                });
            });
        });
    }

    // General > Time Machine as 512 Pixels captured it: what Time Machine does, the backup disk with + and − under it,
    // and the options.
    void TimeMachinePane() {
        static TimeMachineModel model;
        Form([] {
            Section([] { TimeMachineHeader(); });
            BorderedList("##disks", {.rows = 1, .rowHeight = 72.5f, .height = 96.5f, .alternatesRows = false, .showsAddRemove = true}, &model.disks, [](int) {
                BackupDisk("Time Machine", {"983.08 GB available", "Backups: 9/10/24, 10:42\xE2\x80\xAF" "AM \xE2\x80\x93 Today, 6:43\xE2\x80\xAFPM", "Next backup: Automatic (hourly)"});
            });
            TrailingButtons({.top = 10.0f, .spacing = 10.0f}, [] {
                Button("Options\xE2\x80\xA6");
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
