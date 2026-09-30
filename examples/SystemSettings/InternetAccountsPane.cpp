#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    // Internet Accounts as 512 Pixels captured it: the iCloud account with the services it syncs.
    void InternetAccountsPane(int* page) {
        Form([&] {
            Section([&] {
                if (NavigationLink("iCloud", {.icon = PaneIcon(Pane::ICloud), .description = "iCloud Drive, Mail, Contacts, Calendars, Safari, Reminders, Notes, News, Find My Mac, Stocks, Freeform, iCloud Photos, Home, Keychain, and Siri"}))
                    *page = int(Pane::InternetAccountsICloud);
            });
            TrailingButtons({.top = 10.0f, .spacing = 10.0f}, [] {
                Button("Add Account\xE2\x80\xA6");
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
