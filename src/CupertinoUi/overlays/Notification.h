#pragma once

#include "controls/IconPlate.h"
#include "core/Bitmap.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <initializer_list>
#include <span>

namespace Cupertino {
    struct NotificationContent {
        const char* title = "";
        // A bold line under the title, such as the sender of a message.
        const char* subtitle = nullptr;
        const char* body = nullptr;
        // When it was delivered ("now", "1:38 PM"), at the trailing end of the title's line.
        const char* timestamp = nullptr;
        // The app's icon: a picture of its 40 pt canvas, or a plate and a symbol drawn as the icon's 32 pt body.
        Icon icon;
        // An attachment's thumbnail, 32 pt at the trailing side (the kit's Image variant).
        Bitmap attachment;
        // More notifications of its group under it, as Notification Center stacks them: a card for each, two at most.
        int stacked = 0;
    };

    struct NotificationAction {
        const char* title = "";
    };

    // Returned for a click on the banner itself, like UNNotificationDefaultActionIdentifier: the app shows what the
    // notification is about.
    inline constexpr int NotificationDefaultAction = -2;

    // A notification banner as macOS 15 draws it, placed as a view: the app icon, bold title, subtitle and body on the
    // notification material 344 pt wide, the actions as buttons under the text, a stacked group's cards beneath. Returns
    // the index of the action pressed this frame, NotificationDefaultAction for a click elsewhere on it, or -1.
    int NotificationBanner(const NotificationContent& content, std::span<const NotificationAction> actions = {});
    int NotificationBanner(const NotificationContent& content, std::initializer_list<NotificationAction> actions);

    // Presents the banner in the top-right corner of the screen while *is_presented is true, as macOS delivers a
    // notification: it slides in from the trailing edge, leaves on its own after 5 s without actions (a banner) and
    // stays until it is answered with them (an alert). A click on it or on an action dismisses it.
    int NotificationBanner(bool* is_presented, const NotificationContent& content, std::span<const NotificationAction> actions = {});
    int NotificationBanner(bool* is_presented, const NotificationContent& content, std::initializer_list<NotificationAction> actions);
} // namespace Cupertino
