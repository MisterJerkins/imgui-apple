#include "Notification.h"

#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/Motion.h"
#include "core/State.h"
#include "core/Theme.h"
#include "core/Typography.h"
#include "layout/Layout.h"

#include <algorithm>
#include <string>

namespace Cupertino {
    // Title and subtitle are semibold, narrower than the kit's bold (the Tahoe and Ars Technica banners @2x).
    static Font TitleFont() {
        return Font::Style(TextStyle::Body).Weight(FontWeight::Semibold);
    }

    // The banner laid out in points: the title cut to its line beside the timestamp, the body wrapped in its column, the
    // text at least two lines high, and the part above the actions, where the icon is centered.
    struct BannerLayout {
        std::string title;
        std::string subtitle;
        float bodyColumn = 0.0f;
        float bodyHeight = 0.0f;
        float textTop = 0.0f;
        float contentHeight = 0.0f;
        float height = 0.0f;
    };

    static BannerLayout LayOut(const NotificationContent& content, size_t actions) {
        const Metrics::NotificationMetrics& metrics = Metrics::Notification();
        const Font title_font = TitleFont();
        const float column = metrics.width - metrics.textLeading - metrics.textTrailing;
        const float timestamp = content.timestamp ? Pt(Typography::Width(Font::Style(TextStyle::Subheadline), content.timestamp)) + metrics.timestampSpacing : 0.0f;
        BannerLayout layout;
        layout.bodyColumn = content.attachment.IsEmpty() ? column : metrics.width - metrics.attachmentTrailing - metrics.attachment - metrics.attachmentSpacing - metrics.textLeading;
        layout.title = Typography::Truncate(title_font, content.title, Px(column - timestamp));
        if (content.subtitle)
            layout.subtitle = Typography::Truncate(title_font, content.subtitle, Px(layout.bodyColumn));
        if (content.body && *content.body)
            layout.bodyHeight = Pt(Typography::Measure(Font::Style(TextStyle::Body), content.body, Px(layout.bodyColumn)).y);
        const float text = title_font.lineHeight * (content.subtitle ? 2.0f : 1.0f) + layout.bodyHeight;
        layout.textTop = metrics.top + (std::max(text, metrics.minTextHeight) - text) * 0.5f;
        layout.contentHeight = metrics.top + std::max(text, metrics.minTextHeight) + metrics.bottom;
        layout.height = layout.contentHeight + (actions > 0 ? metrics.actionHeight + metrics.actionsBottom : 0.0f);
        return layout;
    }

    // How many cards of its group stand under a banner, and the points they add below it.
    static int StackCards(const NotificationContent& content) {
        return ImClamp(content.stacked, 0, Metrics::Notification().stackCards);
    }

    static float StackHeight(const NotificationContent& content) {
        return Metrics::Notification().stackPeek * float(StackCards(content));
    }

    // An action: a flat button in the material's vibrant fill with the label's text.
    static bool DrawAction(ImGuiID id, const ImRect& frame, const char* title) {
        const Palette& colors = Theme::Colors();
        const Interaction::Response response = Interaction::Button(id, frame);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const CornerRadii radii(Px(Metrics::Notification().actionRadius));
        Draw::FillRoundedRect(draw, frame, radii, response.held && response.hovered ? colors.notificationButtonPressed : colors.notificationButton);
        Typography::Draw(draw, Font::Style(TextStyle::Callout), frame, colors.label, title, TextAlignment::Center);
        if (response.focused)
            Draw::FocusRing(draw, frame, radii, colors.accent);
        return response.pressed;
    }

    // Draws the banner in frame (pixels) and runs its clicks; a presented banner casts its shadow past its window.
    static int DrawBanner(ImGuiID id, const ImRect& frame, const NotificationContent& content, const BannerLayout& layout, std::span<const NotificationAction> actions, bool unclipped_shadow) {
        const Metrics::NotificationMetrics& metrics = Metrics::Notification();
        const Palette& colors = Theme::Colors();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const CornerRadii radii(Px(metrics.radius));
        if (unclipped_shadow)
            draw->PushClipRectFullScreen();
        // The group's other notifications peek out under the banner, the farthest first; the banner's shadow falls on them.
        for (int card = StackCards(content); card >= 1; --card) {
            const float inset = Px(metrics.stackInset + metrics.stackInsetStep * float(card - 1));
            const float drop = Px(metrics.stackPeek * float(card));
            const ImRect rect(frame.Min.x + inset, frame.Min.y + drop, frame.Max.x - inset, frame.Max.y + drop);
            const CornerRadii card_radii(Px(metrics.stackRadius));
            Draw::DropShadows(draw, rect, card_radii, Theme::NotificationShadows());
            Draw::FillRoundedRect(draw, rect, card_radii, card == 1 ? colors.notificationStackNear : colors.notificationStackFar);
            Draw::StrokeRoundedRect(draw, rect, card_radii, colors.notificationRim, Px(1.0f), StrokeAlignment::Inside);
        }
        Draw::DropShadows(draw, frame, radii, Theme::NotificationShadows());
        if (unclipped_shadow)
            draw->PopClipRect();
        Draw::FillRoundedRect(draw, frame, radii, colors.notificationBackground);
        Draw::StrokeRoundedRect(draw, frame, radii, colors.notificationRim, Px(1.0f), StrokeAlignment::Inside);

        // The banner answers a click anywhere outside its actions, which overlap it.
        const Interaction::Response response = Interaction::Button(id, frame, ImGuiButtonFlags_AllowOverlap);
        int chosen = response.pressed ? NotificationDefaultAction : -1;

        const ImVec2 icon_min(frame.Min.x + Px(metrics.iconLeading), frame.Min.y + Px((layout.contentHeight - metrics.icon) * 0.5f));
        ImRect icon(icon_min, icon_min + Px(ImVec2(metrics.icon, metrics.icon)));
        if (content.icon.image.IsEmpty())
            icon.Expand(-Px(metrics.iconMargin));
        DrawIcon(draw, icon, content.icon);

        const Font title_font = TitleFont();
        const float left = frame.Min.x + Px(metrics.textLeading);
        float y = frame.Min.y + Px(layout.textTop);
        const float line = Px(title_font.lineHeight);
        Typography::DrawWrapped(draw, title_font, ImRect(left, y, frame.Max.x - Px(metrics.textTrailing), y + line), colors.notificationText, layout.title);
        if (content.timestamp) {
            // On the title's baseline, at the trailing edge of the column.
            const Font font = Font::Style(TextStyle::Subheadline);
            const float top = y + Typography::Baseline(title_font) - Typography::Baseline(font);
            Typography::Draw(draw, font, ImRect(left, top, frame.Max.x - Px(metrics.textTrailing), top + Px(font.lineHeight)), colors.notificationTimestamp, content.timestamp, TextAlignment::Trailing);
        }
        y += line;
        if (content.subtitle) {
            Typography::DrawWrapped(draw, title_font, ImRect(left, y, left + Px(layout.bodyColumn), y + line), colors.notificationText, layout.subtitle);
            y += line;
        }
        if (layout.bodyHeight > 0.0f)
            Typography::DrawWrapped(draw, Font::Style(TextStyle::Body), ImRect(left, y, left + Px(layout.bodyColumn), y + Px(layout.bodyHeight)), colors.notificationText, content.body);

        const float content_bottom = frame.Min.y + Px(layout.contentHeight);
        if (!content.attachment.IsEmpty()) {
            const ImVec2 max(frame.Max.x - Px(metrics.attachmentTrailing), content_bottom - Px(metrics.attachmentBottom));
            Draw::Image(draw, ImRect(max - Px(ImVec2(metrics.attachment, metrics.attachment)), max), content.attachment, Px(metrics.attachmentRadius));
        }

        // Actions share the text column's width, centered under it.
        if (!actions.empty()) {
            const float count = float(actions.size());
            const float top = content_bottom;
            const float row_left = frame.Min.x + Px(metrics.textLeading);
            const float gap = Px(metrics.actionGap);
            const float width = (frame.GetWidth() - 2.0f * Px(metrics.textLeading) - gap * (count - 1.0f)) / count;
            ImGui::PushID(int(id));
            for (size_t i = 0; i < actions.size(); ++i) {
                const float action_left = row_left + float(i) * (width + gap);
                if (DrawAction(ImGui::GetID(int(i)), ImRect(action_left, top, action_left + width, top + Px(metrics.actionHeight)), actions[i].title))
                    chosen = int(i);
            }
            ImGui::PopID();
        }
        return chosen;
    }

    int NotificationBanner(const NotificationContent& content, std::span<const NotificationAction> actions) {
        const ImGuiID id = Layout::NextViewId();
        const BannerLayout layout = LayOut(content, actions.size());
        const ImRect placed = Layout::Place(Px(ImVec2(Metrics::Notification().width, layout.height + StackHeight(content))));
        if (Layout::IsMeasuring())
            return -1;
        return DrawBanner(id, ImRect(placed.Min, placed.Min + Px(ImVec2(Metrics::Notification().width, layout.height))), content, layout, actions, false);
    }

    int NotificationBanner(const NotificationContent& content, std::initializer_list<NotificationAction> actions) {
        return NotificationBanner(content, std::span<const NotificationAction>(actions.begin(), actions.size()));
    }

    struct PresentedNotification {
        AnimatedFloat presentation;
        double shownAt = 0.0;
        bool shown = false;
    };

    int NotificationBanner(bool* is_presented, const NotificationContent& content, std::span<const NotificationAction> actions) {
        const Metrics::NotificationMetrics& metrics = Metrics::Notification();
        const ImGuiID id = ImGui::GetID(is_presented);
        PresentedNotification& state = State::Get<PresentedNotification>(id);
        const double now = ImGui::GetTime();
        if (*is_presented && !state.shown)
            state.shownAt = now;
        state.shown = *is_presented;
        const float presentation = state.presentation.Update(*is_presented ? 1.0f : 0.0f, Animation::EaseOut(metrics.slide));
        if (presentation <= 0.0f)
            return -1;

        // Under the menu bar, 16 pt from the screen's edge, slid in from past it.
        const BannerLayout layout = LayOut(content, actions.size());
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 size = Px(ImVec2(metrics.width, layout.height));
        const float resting = viewport->WorkPos.x + viewport->WorkSize.x - Px(metrics.screenInset) - size.x;
        const ImVec2 origin = Draw::Snap(ImVec2(resting + (1.0f - presentation) * (size.x + Px(metrics.screenInset)), viewport->WorkPos.y + Px(metrics.screenInset)));
        const ImRect frame(origin, origin + size);
        char name[40];
        ImFormatString(name, IM_ARRAYSIZE(name), "##CupertinoNotification%08X", id);
        Interaction::BeginOverlay(name, ImRect(frame.Min, frame.Max + ImVec2(0.0f, Px(StackHeight(content)))), ImGuiWindowFlags_NoFocusOnAppearing);
        int chosen = DrawBanner(ImGui::GetID("banner"), frame, content, layout, actions, true);
        // The pointer keeps a banner on the screen; one without actions leaves once it has shown for its time.
        if (ImGui::IsWindowHovered())
            state.shownAt = now;
        ImGui::End();
        if (!*is_presented)
            return -1;
        if (actions.empty() && now - state.shownAt >= double(metrics.duration))
            *is_presented = false;
        if (chosen != -1)
            *is_presented = false;
        return chosen;
    }

    int NotificationBanner(bool* is_presented, const NotificationContent& content, std::initializer_list<NotificationAction> actions) {
        return NotificationBanner(is_presented, content, std::span<const NotificationAction>(actions.begin(), actions.size()));
    }
} // namespace Cupertino
