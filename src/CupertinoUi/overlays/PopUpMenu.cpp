#include "PopUpMenu.h"

#include "controls/IconPlate.h"
#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/Motion.h"
#include "core/State.h"
#include "core/Symbols.h"
#include "core/Theme.h"
#include "core/Typography.h"

#include <vector>

namespace Cupertino::PopUpMenu {
    using MenuContent::Entry;
    using MenuContent::EntryKind;

    // One open menu of the stack: the root, then each submenu opened from the one before it.
    struct Level {
        // The submenu entry it lists, or -1 for the root.
        int parent = -1;
        // Where it was drawn, in pixels; the pointer is tested against it.
        ImRect frame;
        int highlighted = -1;
        // How far a menu taller than the screen has scrolled, in pixels.
        float scroll = 0.0f;
    };

    struct MenuState {
        bool open = false;
        bool closing = false;
        bool armed = false;
        ImRect anchor;
        Placement placement = Placement::OverLabel;
        float trailingLimit = FLT_MAX;
        float minimumWidth = 0.0f;
        // Where the press that opened the menu went down, and whether the pointer has left it since.
        ImVec2 pressPosition;
        bool dragged = false;
        int selected = -1;
        int chosen = -1;
        float elapsed = 0.0f;
        std::vector<Level> levels;
        // The item the pointer rests on in a level with a deeper menu open, or on a closed submenu: after a moment the
        // submenus below that level close and a submenu under the pointer opens.
        int hoverLevel = -1;
        int hoverEntry = -1;
        float hoverTime = 0.0f;
    };

    // After a choice the highlight blinks off and on, then the menu fades; closing without a choice only fades.
    static constexpr float BlinkDuration = 0.1f;
    static constexpr float FadeDuration = 0.15f;
    // A press held this long on the menu, or dragged this far, picks the item under the pointer on release.
    static constexpr float HoldDuration = 0.3f;
    static constexpr float DragDistance = 3.0f;
    static constexpr float SubmenuDelay = 0.1f;

    void Open(ImGuiID id, const ImRect& anchor, int selected, Placement placement, float trailing_limit, float minimum_width) {
        MenuState& state = State::Get<MenuState>(id);
        state = MenuState{};
        state.open = true;
        state.anchor = anchor;
        state.placement = placement;
        state.trailingLimit = trailing_limit;
        state.minimumWidth = minimum_width;
        state.selected = selected;
        state.levels.push_back(Level{-1, ImRect(), selected});
        state.armed = !ImGui::IsMouseDown(ImGuiMouseButton_Left);
        state.pressPosition = ImGui::GetIO().MousePos;
    }

    bool IsOpen(ImGuiID id) {
        return State::Get<MenuState>(id).open;
    }

    void Close(ImGuiID id) {
        State::Get<MenuState>(id) = MenuState{};
    }

    static void BeginClosing(MenuState& state, int chosen) {
        state.closing = true;
        state.chosen = chosen;
        state.elapsed = 0.0f;
        if (chosen >= 0)
            state.levels.back().highlighted = chosen;
    }

    // A context menu sets its items larger than a pop-up's, which takes its button's font.
    static float ItemHeight(const MenuState& state) {
        const Metrics::MenuMetrics& metrics = Metrics::Menu();
        return state.placement == Placement::AtPoint ? metrics.contextItemHeight : state.placement == Placement::ComboBoxList ? metrics.listItemHeight : metrics.itemHeight;
    }

    static Font ItemFont(const MenuState& state) {
        return state.placement == Placement::AtPoint ? Font::System(Metrics::Menu().contextFontSize) : Font::Style(TextStyle::Body);
    }

    static Font HeaderFont() {
        return Font::System(Metrics::Menu().headerFontSize, FontWeight::Semibold);
    }

    static Font SubtitleFont() {
        return Font::System(Metrics::Menu().subtitleSize);
    }

    // An item's height in points: a line, and another for its subtitle.
    static float EntryHeight(const MenuState& state, const Entry& entry) {
        const Metrics::MenuMetrics& metrics = Metrics::Menu();
        if (entry.kind == EntryKind::Separator)
            return metrics.separatorHeight;
        return ItemHeight(state) + (entry.subtitle.empty() ? 0.0f : metrics.subtitleLine);
    }

    // The entries a level lists: all top-level entries for the root, the ones after it for a submenu.
    static int FirstChild(int parent) {
        return parent + 1;
    }

    static int EndOfChildren(std::span<const Entry> entries, int parent) {
        return parent < 0 ? int(entries.size()) : parent + 1 + entries[size_t(parent)].children;
    }

    static int NextSibling(std::span<const Entry> entries, int index) {
        return index + 1 + entries[size_t(index)].children;
    }

    static bool IsSelectable(const Entry& entry) {
        return entry.enabled && (entry.kind == EntryKind::Item || entry.kind == EntryKind::Submenu);
    }

    // While Option is held an alternate stands in for the item before it, which otherwise shows alone.
    static bool Hidden(std::span<const Entry> entries, int index, int end) {
        const bool option = ImGui::GetIO().KeyAlt;
        if (entries[size_t(index)].alternate)
            return !option;
        const int next = NextSibling(entries, index);
        return option && next < end && entries[size_t(next)].alternate;
    }

    // One modifier cell each, in the order ⌃ ⌥ ⇧ fn ⌘, and one for the key.
    static int ShortcutCells(const KeyboardShortcut& shortcut) {
        int cells = 1;
        for (const EventModifiers modifier : {EventModifiers::Control, EventModifiers::Option, EventModifiers::Shift, EventModifiers::Function, EventModifiers::Command})
            cells += Contains(shortcut.modifiers, modifier) ? 1 : 0;
        return cells;
    }

    struct Row {
        int entry = -1;
        // From the menu's top edge, in pixels.
        float top = 0.0f;
        float height = 0.0f;
    };

    // A level measured: its rows, its size and the columns every row shares, in pixels.
    struct LevelLayout {
        std::vector<Row> rows;
        ImVec2 size;
        // Left edge of titles without a symbol, and the widths of the image, symbol and shortcut key columns and of the widest
        // badge (0 without them).
        float titleX = 0.0f;
        float imageColumn = 0.0f;
        float symbolColumn = 0.0f;
        float keyColumn = 0.0f;
        float badgeColumn = 0.0f;
        float labelTop = 0.0f;
    };

    static Font BadgeFont() {
        return Font::System(Metrics::Menu().badgeFontSize, FontWeight::Semibold);
    }

    // A badge's capsule width in pixels: its text with the padding either side.
    static float BadgeWidth(const std::string& badge) {
        return Typography::Width(BadgeFont(), badge) + Px(2.0f * Metrics::Menu().badgePadding);
    }

    // The badge's capsule ending at the trailing edge's margin, centered on its item.
    static void DrawBadge(ImDrawList* draw, const ImRect& menu, const ImRect& item, const std::string& badge, bool highlighted) {
        const Metrics::MenuMetrics& metrics = Metrics::Menu();
        const Palette& colors = Theme::Colors();
        const float right = menu.Max.x - Px(metrics.badgeTrailing);
        const float half = Px(metrics.badgeHeight) * 0.5f;
        const ImRect capsule(right - BadgeWidth(badge), item.GetCenter().y - half, right, item.GetCenter().y + half);
        Draw::FillRoundedRect(draw, capsule, CornerRadii(half), colors.menuBadge, CornerStyle::Circular);
        Typography::Draw(draw, BadgeFont(), capsule, highlighted ? colors.selectedContent : colors.label, badge, TextAlignment::Center);
    }

    // Title x of an entry: headers and entries without a symbol start the title column, symbols push their titles past
    // the symbol column.
    static float TitleX(const LevelLayout& layout, const Entry& entry) {
        const bool pictured = entry.symbol || !entry.image.IsEmpty() || entry.paint;
        return pictured && entry.kind != EntryKind::Header ? layout.titleX + layout.symbolColumn + Px(Metrics::Menu().symbolSpacing) : layout.titleX;
    }

    static LevelLayout Measure(const MenuState& state, std::span<const Entry> entries, int parent, bool has_image, ImVec2 image_size) {
        const Metrics::MenuMetrics& metrics = Metrics::Menu();
        const Font font = ItemFont(state);
        const Font header_font = HeaderFont();
        const float inset = metrics.border + metrics.padding;
        const float item_height = ItemHeight(state);
        LevelLayout layout;
        bool checkable = false;
        bool submenus = false;
        int cells = 0;
        float symbol_width = 0.0f;
        float top = Px(inset);
        // Hidden alternates count for the columns and the width too, so the menu keeps its size as Option comes and goes.
        const int first = parent < 0 ? 0 : FirstChild(parent);
        const int end = EndOfChildren(entries, parent);
        for (int i = first; i < end; i = NextSibling(entries, i)) {
            const Entry& entry = entries[size_t(i)];
            if (!Hidden(entries, i, end)) {
                const float height = Px(EntryHeight(state, entry));
                layout.rows.push_back(Row{i, top, height});
                top += height;
            }
            checkable |= entry.checkable;
            submenus |= entry.kind == EntryKind::Submenu;
            if (!entry.shortcut.IsEmpty()) {
                cells = ImMax(cells, ShortcutCells(entry.shortcut));
                layout.keyColumn = ImMax(layout.keyColumn, Typography::Width(font, KeyLabel(entry.shortcut)));
            }
            if (entry.symbol)
                symbol_width = ImMax(symbol_width, Typography::SymbolWidth(entry.symbol, font));
            if (!entry.image.IsEmpty() || entry.paint)
                symbol_width = ImMax(symbol_width, Px(metrics.symbolColumn));
            if (!entry.badge.empty())
                layout.badgeColumn = ImMax(layout.badgeColumn, BadgeWidth(entry.badge));
        }
        layout.imageColumn = has_image ? Px(image_size.x + metrics.imageSpacing) : 0.0f;
        const float label_inset = state.placement == Placement::ComboBoxList ? metrics.listLabelInset : metrics.labelInset;
        layout.titleX = Px(inset + label_inset + (checkable ? metrics.checkmarkColumn : 0.0f)) + layout.imageColumn;
        layout.symbolColumn = symbol_width > 0.0f ? ImMax(Px(metrics.symbolColumn), symbol_width) : 0.0f;
        const float label_shift = state.placement == Placement::AtPoint ? metrics.contextLabelDrop : state.placement == Placement::ComboBoxList ? -metrics.listLabelRaise : 0.0f;
        layout.labelTop = Px((item_height - font.lineHeight) * 0.5f + label_shift);

        // Titles, then the shortcut cells or the submenu chevron at the trailing edge.
        float titles = 0.0f;
        for (int i = first; i < end; i = NextSibling(entries, i)) {
            const Entry& entry = entries[size_t(i)];
            if (entry.kind != EntryKind::Separator)
                titles = ImMax(titles, TitleX(layout, entry) + Typography::Width(entry.kind == EntryKind::Header ? header_font : font, entry.title));
            if (!entry.subtitle.empty())
                titles = ImMax(titles, TitleX(layout, entry) + Typography::Width(SubtitleFont(), entry.subtitle));
        }
        float trailing = Px(metrics.trailingInset);
        if (cells > 0)
            trailing = ImMax(trailing, Px(metrics.shortcutGap + metrics.shortcutPitch * (float(cells) - 0.5f) + metrics.shortcutMargin) + layout.keyColumn * 0.5f);
        if (submenus)
            trailing = ImMax(trailing, Px(metrics.chevronGap + metrics.chevronTrailing));
        if (layout.badgeColumn > 0.0f)
            trailing = ImMax(trailing, Px(metrics.badgeGap + metrics.badgeTrailing) + layout.badgeColumn);
        const float minimum = ImMax(metrics.minWidth, parent < 0 ? state.minimumWidth : 0.0f);
        layout.size = ImVec2(ImMax(Px(ImFloor(Pt(titles + trailing) + 0.5f)), Px(minimum)), top + Px(inset));
        return layout;
    }

    // Where the root sits: a pop-up puts the selected item's label exactly on the button's label, a pull-down hangs
    // under the button, a context menu opens at the pointer, a menu bar menu under its title; then it is kept on screen.
    static ImVec2 RootOrigin(const MenuState& state, const LevelLayout& layout) {
        ImVec2 origin = state.anchor.Min;
        if (state.placement == Placement::OverLabel) {
            float selected_top = layout.rows.empty() ? 0.0f : layout.rows.front().top;
            for (const Row& row : layout.rows) {
                if (row.entry == state.selected)
                    selected_top = row.top;
            }
            origin = ImVec2(state.anchor.Min.x - layout.titleX + layout.imageColumn, state.anchor.Min.y - selected_top - layout.labelTop);
        }
        if (state.placement == Placement::Below || state.placement == Placement::Suggestions)
            origin = ImVec2(state.anchor.Min.x, state.anchor.Max.y + Px(Metrics::Menu().pullDownGap));
        if (state.placement == Placement::MenuBar)
            origin = ImVec2(state.anchor.Min.x, state.anchor.Max.y);
        if (state.placement == Placement::ComboBoxList)
            return ImVec2(state.trailingLimit - layout.titleX, state.anchor.Max.y + Px(Metrics::Menu().listGap));
        origin.x = ImMin(origin.x, state.trailingLimit - layout.size.x);
        return origin;
    }

    // A submenu opens beside its item, over the parent's margin, with its first row level with the item; without room on
    // the right it opens on the left.
    static ImVec2 SubmenuOrigin(const ImRect& parent_frame, float item_top, const LevelLayout& layout) {
        const Metrics::MenuMetrics& metrics = Metrics::Menu();
        const float inset = Px(metrics.border + metrics.padding);
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 origin(parent_frame.Max.x - inset, parent_frame.Min.y + item_top - inset);
        if (origin.x + layout.size.x > viewport->Pos.x + viewport->Size.x - Px(metrics.padding))
            origin.x = parent_frame.Min.x + inset - layout.size.x;
        return origin;
    }

    static ImRect KeepOnScreen(ImVec2 origin, ImVec2 size) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const float margin = Px(Metrics::Menu().padding);
        origin.x = ImClamp(origin.x, viewport->Pos.x + margin, ImMax(viewport->Pos.x + margin, viewport->Pos.x + viewport->Size.x - size.x - margin));
        origin.y = ImClamp(origin.y, viewport->Pos.y + margin, ImMax(viewport->Pos.y + margin, viewport->Pos.y + viewport->Size.y - size.y - margin));
        // On whole pixels, so the hairline border and the rim inside it stay crisp.
        return ImRect(Draw::Snap(origin), Draw::Snap(origin) + Draw::Snap(size));
    }

    // How far a level can scroll: its rows less the height the screen leaves it.
    static float MaxScroll(const Level& level, const LevelLayout& layout) {
        return ImMax(0.0f, layout.size.y - level.frame.GetHeight());
    }

    // Measures every open level and places it, no taller than the screen; the frames are kept for testing the pointer.
    static std::vector<LevelLayout> Arrange(MenuState& state, std::span<const Entry> entries, bool has_image, ImVec2 image_size) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const float available = viewport->Size.y - 2.0f * Px(Metrics::Menu().padding);
        std::vector<LevelLayout> layouts;
        for (size_t l = 0; l < state.levels.size(); ++l) {
            Level& level = state.levels[l];
            layouts.push_back(Measure(state, entries, level.parent, has_image && l == 0, image_size));
            const LevelLayout& layout = layouts.back();
            ImVec2 origin = RootOrigin(state, layout);
            if (l > 0) {
                const Level& parent = state.levels[l - 1];
                float item_top = 0.0f;
                for (const Row& row : layouts[l - 1].rows) {
                    if (row.entry == level.parent)
                        item_top = row.top - parent.scroll;
                }
                origin = SubmenuOrigin(parent.frame, item_top, layout);
            }
            // A menu hanging under its button or title keeps to the room below when that holds a few items.
            float height = ImMin(layout.size.y, available);
            if (l == 0 && state.placement != Placement::OverLabel && state.placement != Placement::AtPoint) {
                const float below = viewport->Pos.y + viewport->Size.y - Px(Metrics::Menu().padding) - origin.y;
                if (below >= Px(5.0f * Metrics::Menu().itemHeight))
                    height = ImMin(height, below);
            }
            level.frame = KeepOnScreen(origin, ImVec2(layout.size.x, height));
            level.scroll = ImClamp(level.scroll, 0.0f, MaxScroll(level, layout));
        }
        return layouts;
    }

    // The bands at a scrolling level's ends that hold its overflow arrows, empty (zero height) where it cannot scroll further.
    static ImRect TopBand(const Level& level) {
        const Metrics::MenuMetrics& metrics = Metrics::Menu();
        const float height = level.scroll > 0.0f ? Px(metrics.border + metrics.padding + metrics.itemHeight) : 0.0f;
        return ImRect(level.frame.Min.x, level.frame.Min.y, level.frame.Max.x, level.frame.Min.y + height);
    }

    static ImRect BottomBand(const Level& level, const LevelLayout& layout) {
        const Metrics::MenuMetrics& metrics = Metrics::Menu();
        const float height = level.scroll < MaxScroll(level, layout) ? Px(metrics.border + metrics.padding + metrics.itemHeight) : 0.0f;
        return ImRect(level.frame.Min.x, level.frame.Max.y - height, level.frame.Max.x, level.frame.Max.y);
    }

    struct Hit {
        int level = -1;
        int entry = -1;
    };

    // The deepest level under the point, and the selectable entry there; the overflow arrows hold none.
    static Hit HitTest(const MenuState& state, const std::vector<LevelLayout>& layouts, std::span<const Entry> entries, ImVec2 point) {
        for (int l = int(state.levels.size()) - 1; l >= 0; --l) {
            const Level& level = state.levels[size_t(l)];
            const ImRect& frame = level.frame;
            if (!frame.Contains(point))
                continue;
            if (point.y < TopBand(level).Max.y || point.y >= BottomBand(level, layouts[size_t(l)]).Min.y)
                return Hit{l, -1};
            for (const Row& row : layouts[size_t(l)].rows) {
                const float top = frame.Min.y + row.top - level.scroll;
                if (point.y >= top && point.y < top + row.height)
                    return Hit{l, IsSelectable(entries[size_t(row.entry)]) ? row.entry : -1};
            }
            return Hit{l, -1};
        }
        return Hit{};
    }

    // A level taller than the screen scrolls with the wheel and while the pointer rests on one of its arrows; the keyboard
    // keeps its highlighted row in view.
    static void Scroll(MenuState& state, const std::vector<LevelLayout>& layouts) {
        const Metrics::MenuMetrics& metrics = Metrics::Menu();
        const ImGuiIO& io = ImGui::GetIO();
        for (size_t l = 0; l < state.levels.size(); ++l) {
            Level& level = state.levels[l];
            const float max_scroll = MaxScroll(level, layouts[l]);
            if (max_scroll <= 0.0f)
                continue;
            if (level.frame.Contains(io.MousePos)) {
                const float step = Px(metrics.itemHeight * metrics.overflowSpeed) * Motion::DeltaTime();
                level.scroll -= io.MouseWheel * Px(metrics.itemHeight);
                if (TopBand(level).Contains(io.MousePos))
                    level.scroll -= step;
                if (BottomBand(level, layouts[l]).Contains(io.MousePos))
                    level.scroll += step;
            }
            const float band = Px(metrics.border + metrics.padding + metrics.itemHeight);
            const bool keyed = ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_UpArrow);
            for (const Row& row : layouts[l].rows) {
                if (!keyed || row.entry != level.highlighted)
                    continue;
                level.scroll = ImClamp(level.scroll, row.top + row.height - level.frame.GetHeight() + band, row.top - band);
            }
            level.scroll = ImClamp(level.scroll, 0.0f, max_scroll);
        }
    }

    static void OpenSubmenu(MenuState& state, int level, int entry, std::span<const Entry> entries, bool highlight_first) {
        state.levels.resize(size_t(level) + 1);
        state.levels[size_t(level)].highlighted = entry;
        Level submenu{entry, ImRect(), -1};
        if (highlight_first) {
            for (int i = FirstChild(entry); i < EndOfChildren(entries, entry); i = NextSibling(entries, i)) {
                if (IsSelectable(entries[size_t(i)])) {
                    submenu.highlighted = i;
                    break;
                }
            }
        }
        state.levels.push_back(submenu);
        state.hoverLevel = -1;
    }

    // An item picks, closing the submenus past its own menu; a submenu opens at once; anything else does nothing.
    static int Activate(MenuState& state, const Hit& hit, std::span<const Entry> entries) {
        if (hit.entry < 0)
            return -1;
        if (entries[size_t(hit.entry)].kind == EntryKind::Submenu) {
            if (state.levels.size() <= size_t(hit.level) + 1 || state.levels[size_t(hit.level) + 1].parent != hit.entry)
                OpenSubmenu(state, hit.level, hit.entry, entries, false);
            return -1;
        }
        state.levels.resize(size_t(hit.level) + 1);
        return hit.entry;
    }

    // The next selectable entry of a level from the highlighted one, in the given direction.
    static int Step(std::span<const Entry> entries, const LevelLayout& layout, int highlighted, int direction) {
        int index = -1;
        for (size_t r = 0; r < layout.rows.size(); ++r) {
            if (layout.rows[r].entry == highlighted)
                index = int(r);
        }
        const int count = int(layout.rows.size());
        for (int r = index < 0 ? (direction > 0 ? 0 : count - 1) : index + direction; r >= 0 && r < count; r += direction) {
            if (IsSelectable(entries[size_t(layout.rows[size_t(r)].entry)]))
                return layout.rows[size_t(r)].entry;
        }
        return highlighted;
    }

    // Mouse and keyboard while the menu is open; returns the entry chosen this frame.
    static int HandleInput(MenuState& state, const std::vector<LevelLayout>& layouts, std::span<const Entry> entries) {
        const ImGuiIO& io = ImGui::GetIO();
        const Hit hit = HitTest(state, layouts, entries, io.MousePos);
        Level& deepest = state.levels.back();

        if (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f) {
            if (hit.level >= 0) {
                Level& level = state.levels[size_t(hit.level)];
                const bool child_open = state.levels.size() > size_t(hit.level) + 1;
                const bool on_open_submenu = child_open && state.levels[size_t(hit.level) + 1].parent == hit.entry;
                if (!child_open || hit.entry >= 0)
                    level.highlighted = hit.entry;
                const bool opens = hit.entry >= 0 && entries[size_t(hit.entry)].kind == EntryKind::Submenu && !on_open_submenu;
                if ((child_open && !on_open_submenu && hit.entry >= 0) || opens) {
                    if (state.hoverLevel != hit.level || state.hoverEntry != hit.entry) {
                        state.hoverLevel = hit.level;
                        state.hoverEntry = hit.entry;
                        state.hoverTime = 0.0f;
                    }
                } else {
                    state.hoverLevel = -1;
                }
            } else {
                // Off the menus the deepest one drops its highlight; the items of open submenus keep theirs.
                deepest.highlighted = -1;
                state.hoverLevel = -1;
            }
        }
        if (state.hoverLevel >= 0) {
            state.hoverTime += Motion::DeltaTime();
            if (state.hoverTime >= SubmenuDelay) {
                const int level = state.hoverLevel;
                const int entry = state.hoverEntry;
                state.levels.resize(size_t(level) + 1);
                if (entries[size_t(entry)].kind == EntryKind::Submenu)
                    OpenSubmenu(state, level, entry, entries, false);
                state.hoverLevel = -1;
            }
        }

        // The keyboard works on the deepest menu.
        Level& current = state.levels.back();
        const LevelLayout& current_layout = layouts[state.levels.size() - 1];
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
            current.highlighted = Step(entries, current_layout, current.highlighted, 1);
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
            current.highlighted = Step(entries, current_layout, current.highlighted, -1);
        const bool on_submenu = current.highlighted >= 0 && entries[size_t(current.highlighted)].kind == EntryKind::Submenu;
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) && on_submenu) {
            OpenSubmenu(state, int(state.levels.size()) - 1, current.highlighted, entries, true);
            return -1;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) && state.levels.size() > 1) {
            state.levels.pop_back();
            return -1;
        }
        const bool space_picks = state.placement != Placement::Suggestions;
        if ((ImGui::IsKeyPressed(ImGuiKey_Enter) || (space_picks && ImGui::IsKeyPressed(ImGuiKey_Space))) && current.highlighted >= 0) {
            if (on_submenu) {
                OpenSubmenu(state, int(state.levels.size()) - 1, current.highlighted, entries, true);
                return -1;
            }
            return current.highlighted;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            if (state.levels.size() > 1)
                state.levels.pop_back();
            else
                BeginClosing(state, -1);
            return -1;
        }

        if (!state.armed) {
            // The press that opened the menu picks the item it is released on after dragging to it or holding; a quick
            // click leaves the menu open, even when the menu had to move and put another item under the pointer.
            if (ImLengthSqr(io.MousePos - state.pressPosition) > Px(DragDistance) * Px(DragDistance))
                state.dragged = true;
            if (!io.MouseDown[ImGuiMouseButton_Left]) {
                if (hit.entry >= 0 && (state.dragged || state.elapsed > HoldDuration))
                    return Activate(state, hit, entries);
                state.armed = true;
            }
            return -1;
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && hit.entry >= 0)
            return Activate(state, hit, entries);
        // A context menu also takes the right button: released on an item after a moment, it picks the item.
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && hit.entry >= 0 && state.elapsed > HoldDuration)
            return Activate(state, hit, entries);
        if ((ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && hit.level < 0)
            BeginClosing(state, -1);
        return -1;
    }

    // The border is the frame's outermost half point, the rim a light line inside it.
    static void DrawChrome(ImDrawList* draw, const ImRect& menu) {
        const Metrics::MenuMetrics& metrics = Metrics::Menu();
        const Palette& colors = Theme::Colors();
        const CornerRadii radii(Px(metrics.radius));
        draw->PushClipRectFullScreen();
        Draw::DropShadows(draw, menu, radii, Theme::MenuShadows());
        draw->PopClipRect();
        Draw::FillRoundedRect(draw, menu, radii, colors.menuBackground);
        const float border = Px(metrics.border);
        const ImRect inner(menu.Min + ImVec2(border, border), menu.Max - ImVec2(border, border));
        Draw::StrokeRoundedRect(draw, inner, radii.Offset(-border), colors.menuRim, Environment().IsDark() ? Px(1.0f) : border, StrokeAlignment::Inside);
        Draw::StrokeRoundedRect(draw, menu, radii, colors.menuBorder, border, StrokeAlignment::Inside);
    }

    // The shortcut at the trailing edge: the key centered in the key column, the modifiers in cells before it in the order
    // ⌃ ⌥ ⇧ fn ⌘.
    static void DrawShortcut(ImDrawList* draw, const ImRect& menu, const ImRect& row, const KeyboardShortcut& shortcut, const Font& font, float label_top, float key_column, Rgba color) {
        const Metrics::MenuMetrics& metrics = Metrics::Menu();
        const float half = Px(metrics.shortcutPitch) * 0.5f;
        float center = menu.Max.x - Px(metrics.shortcutMargin) - key_column * 0.5f;
        const auto cell = [&](float x) {
            return ImRect(x - half, row.Min.y + label_top, x + half, row.Min.y + label_top + Px(font.lineHeight));
        };
        Typography::Draw(draw, font, cell(center), color, KeyLabel(shortcut), TextAlignment::Center);
        for (const EventModifiers modifier : {EventModifiers::Command, EventModifiers::Function, EventModifiers::Shift, EventModifiers::Option, EventModifiers::Control}) {
            if (!Contains(shortcut.modifiers, modifier))
                continue;
            center -= Px(metrics.shortcutPitch);
            if (modifier == EventModifiers::Function)
                Typography::DrawSymbol(draw, Symbols::Globe, font, cell(center), color);
            else
                Typography::Draw(draw, font, cell(center), color, ModifierGlyphs(modifier), TextAlignment::Center);
        }
    }

    static void DrawLevel(ImDrawList* draw, const MenuState& state, const Level& level, bool child_open, const LevelLayout& layout, std::span<const Entry> entries, bool highlight_visible, ItemImage image, ImVec2 image_size) {
        const Metrics::MenuMetrics& metrics = Metrics::Menu();
        const Palette& colors = Theme::Colors();
        const Font font = ItemFont(state);
        const Font header_font = HeaderFont();
        const Font checkmark_font = Font::System(9.0f, FontWeight::Bold).WithLineHeight(16.0f);
        const Font chevron_font = Font::System(13.0f, FontWeight::Semibold).ImageScale(SymbolScale::Small);
        const ImRect& menu = level.frame;
        const float inset = Px(metrics.border + metrics.padding);
        DrawChrome(draw, menu);
        const ImRect top_band = TopBand(level);
        const ImRect bottom_band = BottomBand(level, layout);
        const Font arrow_font = Font::System(metrics.overflowArrowSize);
        if (top_band.GetHeight() > 0.0f)
            Typography::DrawSymbol(draw, Symbols::ArrowtriangleUpFill, arrow_font, ImRect(menu.Min.x, top_band.Max.y - Px(metrics.itemHeight), menu.Max.x, top_band.Max.y), colors.label);
        if (bottom_band.GetHeight() > 0.0f)
            Typography::DrawSymbol(draw, Symbols::ArrowtriangleDownFill, arrow_font, ImRect(menu.Min.x, bottom_band.Min.y, menu.Max.x, bottom_band.Min.y + Px(metrics.itemHeight)), colors.label);
        draw->PushClipRect(ImVec2(menu.Min.x, top_band.Max.y), ImVec2(menu.Max.x, bottom_band.Min.y), true);
        for (const Row& row : layout.rows) {
            const Entry& entry = entries[size_t(row.entry)];
            const float row_top = menu.Min.y + row.top - level.scroll;
            if (row_top + row.height < top_band.Max.y || row_top > bottom_band.Min.y)
                continue;
            const ImRect item(menu.Min.x + inset, row_top, menu.Max.x - inset, row_top + row.height);
            if (entry.kind == EntryKind::Separator) {
                const float y = item.Min.y + Px(metrics.separatorLine);
                Draw::FillRect(draw, ImRect(menu.Min.x + Px(metrics.separatorInset), y, menu.Max.x - Px(metrics.separatorInset), y + Px(1.0f)), colors.menuSeparator);
                continue;
            }
            if (entry.kind == EntryKind::Header) {
                // On the items' baseline, in small gray type.
                const float top = item.Min.y + layout.labelTop + Typography::Baseline(font) - Typography::Baseline(header_font);
                Typography::Draw(draw, header_font, ImRect(menu.Min.x + layout.titleX, top, item.Max.x, top + Px(header_font.lineHeight)), colors.menuHeader, entry.title);
                continue;
            }
            const bool highlighted = row.entry == level.highlighted && (highlight_visible || child_open);
            if (highlighted)
                Draw::FillRoundedRect(draw, item, CornerRadii(Px(metrics.itemRadius)), colors.menuSelection);
            // The title's line: symbols, shortcuts, badges and the chevron stay on it when a subtitle follows.
            const ImRect line(item.Min.x, item.Min.y, item.Max.x, item.Min.y + Px(ItemHeight(state)));
            const Rgba color = highlighted ? colors.selectedContent : colors.LabelColor(entry.enabled);
            const float label_top = layout.labelTop;
            if (entry.checked) {
                const ImRect mark(item.Min + Px(ImVec2(3.0f, 4.0f)), item.Min + Px(ImVec2(14.0f, 20.0f)));
                Typography::DrawSymbol(draw, Symbols::Checkmark, checkmark_font, mark, color, TextAlignment::Leading);
            }
            if (image) {
                const ImVec2 min(menu.Min.x + layout.titleX - Px(image_size.x + metrics.imageSpacing), line.GetCenter().y - Px(image_size.y) * 0.5f);
                image(draw, ImRect(min, min + Px(image_size)), row.entry);
            }
            const ImRect column(menu.Min.x + layout.titleX, line.Min.y, menu.Min.x + layout.titleX + layout.symbolColumn, line.Max.y);
            if (entry.symbol)
                Typography::DrawSymbol(draw, entry.symbol, font, column, color);
            if (!entry.image.IsEmpty() || entry.paint) {
                const float half = Px(metrics.symbolColumn) * 0.5f;
                const ImVec2 center = column.GetCenter();
                const ImRect picture(center.x - half, center.y - half, center.x + half, center.y + half);
                if (entry.paint)
                    DrawIcon(draw, picture, Icon{.paint = entry.paint, .color = color});
                else
                    Draw::Image(draw, picture, entry.image);
            }
            const float title_x = menu.Min.x + TitleX(layout, entry);
            Typography::Draw(draw, font, ImRect(title_x, item.Min.y + label_top, item.Max.x, item.Min.y + label_top + Px(font.lineHeight)), color, entry.title);
            if (!entry.subtitle.empty()) {
                const Font subtitle_font = SubtitleFont();
                const float top = item.Min.y + label_top + Typography::Baseline(font) + Px(metrics.subtitleLine) - Typography::Baseline(subtitle_font);
                const Rgba secondary = highlighted ? colors.selectedContent : entry.enabled ? colors.secondaryLabel : colors.tertiaryLabel;
                Typography::Draw(draw, subtitle_font, ImRect(title_x, top, item.Max.x, top + Px(subtitle_font.lineHeight)), secondary, entry.subtitle);
            }
            const Rgba accessory = highlighted ? colors.selectedContent : colors.menuShortcut;
            if (!entry.shortcut.IsEmpty())
                DrawShortcut(draw, menu, line, entry.shortcut, font, label_top, layout.keyColumn, accessory);
            if (!entry.badge.empty())
                DrawBadge(draw, menu, line, entry.badge, highlighted);
            if (entry.kind == EntryKind::Submenu) {
                const float center = menu.Max.x - Px(metrics.chevronTrailing);
                const ImRect chevron(center - Px(5.0f), line.Min.y, center + Px(5.0f), line.Max.y);
                Typography::DrawSymbol(draw, Symbols::ChevronRight, chevron_font, chevron, highlighted ? colors.selectedContent : colors.LabelColor(entry.enabled));
            }
        }
        draw->PopClipRect();
    }

    int Show(ImGuiID id, std::span<const Entry> entries, ItemImage image, ImVec2 image_size) {
        MenuState& state = State::Get<MenuState>(id);
        if (!state.open || entries.empty())
            return -1;

        std::vector<LevelLayout> layouts = Arrange(state, entries, image != nullptr, image_size);
        state.elapsed += Motion::DeltaTime();
        int picked = -1;
        if (!state.closing) {
            const size_t levels = state.levels.size();
            const int parent = state.levels.back().parent;
            picked = HandleInput(state, layouts, entries);
            if (picked >= 0)
                BeginClosing(state, picked);
            if (state.levels.size() != levels || state.levels.back().parent != parent)
                layouts = Arrange(state, entries, image != nullptr, image_size);
            Scroll(state, layouts);
        }

        float opacity = 1.0f;
        bool highlight_visible = true;
        if (state.closing) {
            const float fade_start = state.chosen >= 0 ? BlinkDuration : 0.0f;
            highlight_visible = state.chosen < 0 || state.elapsed < BlinkDuration * 0.5f || state.elapsed >= BlinkDuration;
            opacity = 1.0f - ImSaturate((state.elapsed - fade_start) / FadeDuration);
            if (state.elapsed >= fade_start + FadeDuration) {
                state.open = false;
                return picked;
            }
        }

        // Suggestions leave the field its pointer and focus: no backdrop, and their window does not take the focus.
        const bool suggestions = state.placement == Placement::Suggestions;
        if (!suggestions)
            Interaction::Backdrop("##CupertinoMenuBackdrop");

        for (size_t l = 0; l < state.levels.size(); ++l) {
            const Level& level = state.levels[l];
            char name[40];
            ImFormatString(name, IM_ARRAYSIZE(name), "##CupertinoMenu%08X.%d", id, int(l));
            Interaction::BeginOverlay(name, level.frame, suggestions ? ImGuiWindowFlags_NoFocusOnAppearing : 0);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, opacity);
            const bool child_open = l + 1 < state.levels.size();
            DrawLevel(ImGui::GetWindowDrawList(), state, level, child_open, layouts[l], entries, highlight_visible, l == 0 ? image : nullptr, image_size);
            ImGui::PopStyleVar();
            ImGui::End();
        }
        return picked;
    }

    int Show(ImGuiID id, std::span<const char* const> items, ItemImage image, ImVec2 image_size) {
        const MenuState& state = State::Get<MenuState>(id);
        if (!state.open || items.empty())
            return -1;
        std::vector<Entry> entries(items.size());
        for (size_t i = 0; i < items.size(); ++i) {
            entries[i].title = items[i];
            entries[i].checkable = state.selected >= 0;
            entries[i].checked = int(i) == state.selected;
        }
        return Show(id, entries, image, image_size);
    }
} // namespace Cupertino::PopUpMenu
