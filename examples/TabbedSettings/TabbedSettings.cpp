#include "Examples.h"

#include "Cupertino.h"

namespace Examples {
    using namespace Cupertino;

    struct TvSettingsModel {
        int tab = 1;
        bool checkDownloads = true;
        bool downloadMovies = false;
        bool downloadShows = true;
        bool libraryCheckboxes = false;
        int continueWatching = 0;
        int listSize = 1;
        bool playNextEpisode = true;
        bool playRecommendation = true;
        bool sportsScores = true;
        int streaming = 0;
        int downloads = 0;
        bool multichannel = false;
        int audioLanguage = 0;
        int hdmi = 0;
        bool subtitlesWhenMuted = true;
        bool subtitlesOnSkipBack = true;
        bool viewingHistory = true;
        bool keepOrganized = true;
        bool copyToMedia = true;
        bool deleteWatched = false;
        bool keepOnTop = false;
    };

    // The form's grid (TV Settings @2x): labels end 207 pt from the window's edge, controls start at 215, and a
    // checkbox's title stands 20.5 pt after its box.
    constexpr float LabelColumn = 207.0f;
    constexpr float ControlColumn = 215.0f;
    constexpr float CheckboxTitle = 20.5f;

    static const TextOptions& Description() {
        static const TextOptions options{.font = Font::Style(TextStyle::Subheadline), .wraps = true};
        return options;
    }

    // A control over its description, both in the control column.
    static void Described(const std::function<void()>& control, const char* description, float spacing) {
        VStack({.alignment = HorizontalAlignment::Leading, .spacing = spacing}, [&] {
            control();
            Frame({.width = 400.0f, .alignment = {HorizontalAlignment::Leading, VerticalAlignment::Top}}, [&] { Text(description, Description()); });
        });
    }

    // A checkbox with its description indented to its title.
    static void DescribedCheckbox(const char* title, bool* on, const char* description) {
        VStack({.alignment = HorizontalAlignment::Leading, .spacing = 8.0f}, [&] {
            Toggle(title, on);
            Padding(EdgeInsets{0.0f, CheckboxTitle, 0.0f, 0.0f}, [&] {
                Frame({.width = 360.0f, .alignment = {HorizontalAlignment::Leading, VerticalAlignment::Top}}, [&] { Text(description, Description()); });
            });
        });
    }

    // The languages to download, a bordered table with + and − under its rows as AppKit sets one outside forms.
    static void LanguageList() {
        static const char* const languages[] = {"Original Audio Language", "Mac Language: English"};
        Frame({.width = 385.0f, .height = 65.0f}, [] {
            Table("##languages", {.style = TableStyle::Bordered, .columns = {{}}, .rows = 2, .showsAddRemove = true, .alternatesRows = false, .rowHeight = 19.0f}, nullptr, [](const TableCell& cell) { TableText(cell, languages[cell.row]); });
        });
    }

    static void Divider(float leading) {
        Padding(EdgeInsets{0.0f, leading, 0.0f, 20.0f}, [] { Cupertino::Divider(); });
    }

    // Help at the leading edge, Cancel and OK at the trailing one, 19 pt under the last line of every tab.
    static void Footer() {
        Padding(EdgeInsets{19.0f, 15.0f, 0.0f, 20.0f}, [] {
            HStack([] {
                HelpButton();
                Spacer();
                Button("Cancel", {.role = ButtonRole::Cancel, .minWidth = 62.0f});
                Button("OK", {.role = ButtonRole::Default, .minWidth = 62.0f});
            });
        });
    }

    static void GeneralTab(TvSettingsModel& model) {
        ColumnsForm({.labelWidth = LabelColumn, .spacing = ControlColumn - LabelColumn}, [&] {
            Padding(EdgeInsets{20.0f, 0.0f, 0.0f, 0.0f}, [&] {
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 10.0f}, [&] {
                    LabeledContent("Downloads:", [&] { Toggle("Always check for available downloads", &model.checkDownloads); });
                    LabeledContent("Automatic Downloads:", [&] {
                        HStack({.spacing = 20.0f}, [&] {
                            Toggle("Movies", &model.downloadMovies);
                            Toggle("TV Shows", &model.downloadShows);
                        });
                    });
                    LabeledContent("Library:", [&] { DescribedCheckbox("Checkboxes in Library", &model.libraryCheckboxes, "Only checked items sync to your devices."); });
                    LabeledContent("Continue Watching:", [&] { Picker("##continue-watching", &model.continueWatching, {"Still Frame", "Poster Art"}, {.style = PickerStyle::RadioGroup}); });
                    LabeledContent("List Size:", [&] { Picker("##list-size", &model.listSize, {"Small", "Medium", "Large"}); });
                });
            });
            Padding(EdgeInsets{12.0f, 0.0f, 0.0f, 0.0f}, [] { Divider(85.0f); });
            Padding(EdgeInsets{12.0f, 0.0f, 0.0f, 0.0f}, [&] {
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 10.0f}, [&] {
                    LabeledContent("Autoplay:", [&] {
                        VStack({.alignment = HorizontalAlignment::Leading, .spacing = 8.0f}, [&] {
                            Toggle("Play Next Episode", &model.playNextEpisode);
                            DescribedCheckbox("Play a Recommendation", &model.playRecommendation, "When a series, movie or sporting event ends, play something recommended for you.");
                        });
                    });
                    LabeledContent("Sports:", [&] { Toggle("Show Sports Scores", &model.sportsScores); });
                });
            });
            Padding(EdgeInsets{12.0f, 0.0f, 0.0f, 0.0f}, [] { Divider(15.0f); });
            Footer();
        });
    }

    static void PlaybackTab(TvSettingsModel& model) {
        ColumnsForm({.labelWidth = LabelColumn, .spacing = ControlColumn - LabelColumn}, [&] {
            Padding(EdgeInsets{13.0f, 0.0f, 0.0f, 0.0f}, [&] {
                LabeledContent("Streaming Options:", [&] { Described([&] { Picker("##streaming", &model.streaming, {"High Quality (Up To 4K)", "Good Quality", "Data Saver"}); }, "Stream uses more data.", 7.0f); });
            });
            Padding(EdgeInsets{13.5f, 0.0f, 0.0f, 0.0f}, [&] {
                LabeledContent("Download Options:", [&] { Described([&] { Picker("##downloads", &model.downloads, {"Most Compatible HD (1080p)", "Highest Quality (Up To 4K)"}); }, "Format most-compatible for this device.", 7.0f); });
            });
            Padding(EdgeInsets{14.0f, 0.0f, 0.0f, 0.0f}, [&] {
                Disabled(true, [&] { LabeledContent("Download\nAudio Languages", LanguageList); });
            });
            Padding(EdgeInsets{7.5f, LabelColumn - 11.0f, 0.0f, 0.0f}, [&] { Toggle("Download Multichannel Audio", &model.multichannel); });
            Padding(EdgeInsets{8.0f, 0.0f, 0.0f, 0.0f}, [] { Divider(85.0f); });
            Padding(EdgeInsets{7.0f, 0.0f, 0.0f, 0.0f}, [&] {
                LabeledContent("Audio Language", [&] { Described([&] { Picker("##audio-language", &model.audioLanguage, {"Auto", "English", "French", "German", "Spanish"}, {.width = 231.0f}); }, "If you choose Auto, videos will automatically play in their default or original language. If you set a different audio language, Apple TV will play dubbed audio in that language when available.", 7.0f); });
            });
            Padding(EdgeInsets{8.0f, 0.0f, 0.0f, 0.0f}, [&] {
                LabeledContent("HDMI Passthrough", [&] {
                    VStack({.alignment = HorizontalAlignment::Leading, .spacing = 7.0f}, [&] {
                        Picker("##hdmi", &model.hdmi, {"Off", "Prefer HDMI Passthrough"}, {.width = 186.0f});
                        // The link follows the description as its next line, 2 pt closer than the description's own lines.
                        VStack({.alignment = HorizontalAlignment::Leading, .spacing = -2.0f}, [&] {
                            Frame({.width = 400.0f, .alignment = {HorizontalAlignment::Leading, VerticalAlignment::Top}}, [&] { Text("Play Dolby Atmos and other Dolby Audio formats using HDMI Passthrough when connected to a supported device.", Description()); });
                            Button("About HDMI Passthrough\xE2\x80\xA6", {.style = ButtonStyle::Link, .font = Font::Style(TextStyle::Subheadline)});
                        });
                    });
                });
            });
            Padding(EdgeInsets{8.0f, 0.0f, 0.0f, 0.0f}, [] { Divider(85.0f); });
            Padding(EdgeInsets{9.0f, 0.0f, 0.0f, 0.0f}, [&] {
                LabeledContent("Automatic Subtitles:", [&] {
                    VStack({.alignment = HorizontalAlignment::Leading, .spacing = 8.0f}, [&] {
                        DescribedCheckbox("Show When Muted", &model.subtitlesWhenMuted, "Automatically turn on subtitles when the volume is muted or turned all the way down.");
                        DescribedCheckbox("Show on Skip Back", &model.subtitlesOnSkipBack, "Temporarily turn on subtitles when you skip back, up to 30 seconds.");
                    });
                });
            });
            Padding(EdgeInsets{10.0f, 0.0f, 0.0f, 0.0f}, [] { Divider(15.0f); });
            Padding(EdgeInsets{12.0f, ControlColumn, 0.0f, 0.0f}, [&] { DescribedCheckbox("Use Viewing History", &model.viewingHistory, "TV shows and movies played on this Mac will influence your \xE2\x80\x9CWatch Now\xE2\x80\x9D recommendations and update your \xE2\x80\x9C" "Continue Watching\xE2\x80\x9D across your devices."); });
            Padding(EdgeInsets{10.0f, 0.0f, 0.0f, 0.0f}, [] { Divider(15.0f); });
            Footer();
        });
    }

    static void FilesTab(TvSettingsModel& model) {
        ColumnsForm({.labelWidth = LabelColumn, .spacing = ControlColumn - LabelColumn}, [&] {
            Padding(EdgeInsets{20.0f, 0.0f, 0.0f, 0.0f}, [&] {
                LabeledContent("Media folder location:", [&] {
                    VStack({.alignment = HorizontalAlignment::Leading, .spacing = 8.0f}, [&] {
                        const Icon disk = {.symbol = Symbols::Internaldrive, .color = Theme::SystemGray()};
                        const Icon folder = {.paint = PaintFolderIcon};
                        Frame({.width = 200.0f}, [&] { PathControl("##media-folder", {{"Macintosh HD", disk}, {"Users", folder}, {"john", folder}, {"Movies", folder}, {"TV", folder}, {"Media", folder}}, {.style = PathControlStyle::PopUp}); });
                        HStack({.spacing = 12.0f}, [] {
                            Button("Change\xE2\x80\xA6");
                            Button("Reset");
                        });
                    });
                });
            });
            Padding(EdgeInsets{12.0f, 0.0f, 0.0f, 0.0f}, [] { Divider(85.0f); });
            Padding(EdgeInsets{12.0f, ControlColumn, 0.0f, 0.0f}, [&] {
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 10.0f}, [&] {
                    DescribedCheckbox("Keep Media folder organized", &model.keepOrganized, "Automatically move imported videos into folders for the TV app.");
                    DescribedCheckbox("Copy files to Media folder when adding to library", &model.copyToMedia, "Copy an item to the Media folder when you drag it to the TV window or choose File > Import.");
                    DescribedCheckbox("Automatically delete watched movies and TV shows", &model.deleteWatched, "Movies and TV shows are deleted after you play them.");
                });
            });
            Padding(EdgeInsets{12.0f, 0.0f, 0.0f, 0.0f}, [] { Divider(15.0f); });
            Footer();
        });
    }

    static void AdvancedTab(TvSettingsModel& model) {
        ColumnsForm({.labelWidth = LabelColumn, .spacing = ControlColumn - LabelColumn}, [&] {
            Padding(EdgeInsets{20.0f, 0.0f, 0.0f, 0.0f}, [&] {
                VStack({.alignment = HorizontalAlignment::Leading, .spacing = 12.0f}, [&] {
                    LabeledContent("Remotes:", [] { Described([] { Button("Forget All Remotes"); }, "Remotes paired with this library stop controlling it.", 7.0f); });
                    LabeledContent("Dialog Warnings:", [] { Button("Reset All Warnings"); });
                    LabeledContent("TV Store Cache:", [] { Button("Reset Cache"); });
                    LabeledContent("Play History:", [] { Described([] { Button("Clear Play History\xE2\x80\xA6"); }, "Clear what you\xE2\x80\x99ve watched from your devices, and the Up Next and Continue Watching rows.", 7.0f); });
                });
            });
            Padding(EdgeInsets{12.0f, 0.0f, 0.0f, 0.0f}, [] { Divider(85.0f); });
            Padding(EdgeInsets{12.0f, ControlColumn, 0.0f, 0.0f}, [&] { Toggle("Keep video playback on top of all other windows", &model.keepOnTop); });
            Padding(EdgeInsets{12.0f, 0.0f, 0.0f, 0.0f}, [] { Divider(15.0f); });
            Footer();
        });
    }

    void TabbedSettings(bool* open, const Placement& placement) {
        static TvSettingsModel model;
        // The window takes each tab's height, as a settings window does.
        static const float heights[] = {465.5f, 737.0f, 398.0f, 388.0f};
        const WindowOptions options = {.size = placement.SizeOr(ImVec2(640.0f, heights[model.tab])), .position = placement.position, .minimizable = false, .zoomable = false, .compactTitleBar = true};
        Window("TV Settings", open, options, [&] {
            TabBar(&model.tab, {{"General", Symbols::Gearshape}, {"Playback", Symbols::PlayCircle}, {"Files", Symbols::Folder}, {"Advanced", Symbols::Gearshape2}});
            Id(model.tab, [&] {
                if (model.tab == 0)
                    GeneralTab(model);
                else if (model.tab == 1)
                    PlaybackTab(model);
                else if (model.tab == 2)
                    FilesTab(model);
                else
                    AdvancedTab(model);
            });
        });
    }
} // namespace Examples
