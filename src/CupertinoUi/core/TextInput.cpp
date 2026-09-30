#include "TextInput.h"

#include "Draw.h"
#include "Environment.h"
#include "Metrics.h"
#include "State.h"
#include "Theme.h"

#include <cctype>
#include <cmath>
#include <vector>

namespace Cupertino::TextInput {
    struct Snapshot {
        std::string text;
        int caret = 0;
        int anchor = 0;
    };

    // Caret and selection anchor are codepoint indices, so secure text (a bullet per codepoint) maps one to one. Undo and
    // redo keep the text as it was before each edit, NSTextView's way: a run of typing is one step, and any other edit
    // or a caret move ends the run. column is the x (from the text's left) that moves between lines keep, or negative.
    struct EditState {
        int caret = 0;
        int anchor = 0;
        float scroll = 0.0f;
        float column = -1.0f;
        double moved = 0.0;
        bool dragging = false;
        std::vector<Snapshot> undo;
        std::vector<Snapshot> redo;
        bool typing = false;
    };

    // A line of laid-out text: its codepoints [start, end) and where the next line starts, past a line feed or at a wrap.
    struct Line {
        int start = 0;
        int end = 0;
        int next = 0;
    };

    static constexpr size_t UndoLimit = 100;

    // Letters, digits and everything beyond ASCII form words; spaces and punctuation separate them.
    static std::vector<bool> WordMask(std::string_view text) {
        std::vector<bool> words;
        for (const char c : text) {
            const unsigned char byte = static_cast<unsigned char>(c);
            if (!Typography::IsContinuationByte(c))
                words.push_back(byte >= 0x80 || std::isalnum(byte) || byte == '_');
        }
        return words;
    }

    // macOS word moves: left to the start of the previous word, right to the end of the next one.
    static int WordLeft(const std::vector<bool>& words, int index) {
        while (index > 0 && !words[size_t(index - 1)])
            --index;
        while (index > 0 && words[size_t(index - 1)])
            --index;
        return index;
    }

    static int WordRight(const std::vector<bool>& words, int index) {
        const int count = int(words.size());
        while (index < count && !words[size_t(index)])
            ++index;
        while (index < count && words[size_t(index)])
            ++index;
        return index;
    }

    // The first completion longer than text that begins with it, ASCII case aside, or nullptr.
    static const char* Completion(std::string_view text, std::span<const char* const> completions) {
        for (const char* completion : completions) {
            const std::string_view candidate(completion);
            if (candidate.size() <= text.size())
                continue;
            size_t i = 0;
            while (i < text.size() && std::tolower((unsigned char)text[i]) == std::tolower((unsigned char)candidate[i]))
                ++i;
            if (i == text.size())
                return completion;
        }
        return nullptr;
    }

    // What the field shows: the text, or a bullet per codepoint.
    static std::string_view Shown(const std::string& text, bool secure, std::string& bullets) {
        if (!secure)
            return text;
        bullets.clear();
        for (int i = Typography::CodepointCount(text); i > 0; --i)
            bullets += "\xE2\x80\xA2";
        return bullets;
    }

    // Wrapped text breaks at line feeds and, past width, after the last space of the line or inside a word too long for
    // one, NSTextView's word wrapping; spaces hang at the end of their line. A single line stays one.
    static std::vector<Line> Lines(std::string_view text, const std::vector<float>& offsets, bool wraps, float width) {
        const int count = int(offsets.size()) - 1;
        if (!wraps)
            return {Line{0, count, count}};
        std::vector<Line> lines;
        int start = 0;
        int space = -1;
        int index = 0;
        for (size_t byte = 0; byte < text.size(); ++index) {
            const char c = text[byte];
            ++byte;
            while (byte < text.size() && Typography::IsContinuationByte(text[byte]))
                ++byte;
            if (c == '\n') {
                lines.push_back({start, index, index + 1});
                start = index + 1;
                space = -1;
                continue;
            }
            if (width > 0.0f && c != ' ' && index > start && offsets[size_t(index + 1)] - offsets[size_t(start)] > width) {
                const int wrap = space > start ? space : index;
                lines.push_back({start, wrap, wrap});
                start = wrap;
                space = -1;
            }
            if (c == ' ')
                space = index + 1;
        }
        lines.push_back({start, count, count});
        return lines;
    }

    // The line an index shows on: at a wrap it starts the next line, before a line feed it ends its own.
    static int LineOf(const std::vector<Line>& lines, int index) {
        for (size_t l = 0; l + 1 < lines.size(); ++l) {
            if (index < lines[l].next)
                return int(l);
        }
        return int(lines.size()) - 1;
    }

    // The last index a click can reach on a line: before the space a wrap leaves at its end.
    static int ReachableEnd(const std::vector<Line>& lines, int l) {
        const Line& line = lines[size_t(l)];
        const bool wrapped = line.next == line.end && size_t(l) + 1 < lines.size();
        return wrapped ? ImMax(line.start, line.end - 1) : line.end;
    }

    float TextHeight(const Font& font, std::string_view text, float width) {
        static std::vector<float> offsets;
        Typography::Offsets(font, text, offsets);
        return float(Lines(text, offsets, true, width).size()) * Px(font.lineHeight);
    }

    Result Edit(ImGuiID id, const ImRect& rect, std::string* text, const Style& style) {
        ImGuiContext& g = *GImGui;
        ImGuiIO& io = g.IO;
        Result result;
        const bool enabled = Environment().enabled;
        const bool multiline = style.multiline && !style.secure;
        if (!ImGui::ItemAdd(rect, id, nullptr, ImGuiItemFlags_Inputable | (enabled ? ImGuiItemFlags_None : ImGuiItemFlags_Disabled)))
            return result;

        EditState& state = State::Get<EditState>(id);
        std::string bullets;
        std::string_view shown = Shown(*text, style.secure, bullets);
        static std::vector<float> offsets;
        Typography::Offsets(style.font, shown, offsets);
        int count = int(offsets.size()) - 1;
        const float room = rect.GetWidth();
        std::vector<Line> lines = Lines(shown, offsets, multiline, room);
        state.caret = ImClamp(state.caret, 0, count);
        state.anchor = ImClamp(state.anchor, 0, count);

        // Focus follows ImGui's text input: the active item owns the arrows and editing keys until a click elsewhere.
        const bool hovered = ImGui::ItemHoverable(rect, id, g.LastItemData.ItemFlags);
        if (hovered)
            ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
        const bool clicked = hovered && io.MouseClicked[ImGuiMouseButton_Left];
        // Tabbing into a field starts editing with all its text selected, as NSTextField does.
        const bool tabbed = g.NavActivateId == id && (g.NavActivateFlags & ImGuiActivateFlags_PreferInput) && enabled && g.ActiveId != id;
        if (tabbed) {
            ImGui::SetActiveID(id, g.CurrentWindow);
            ImGui::SetFocusID(id, g.CurrentWindow);
            state.anchor = 0;
            state.caret = count;
        }
        if (clicked && g.ActiveId != id) {
            ImGui::SetActiveID(id, g.CurrentWindow);
            ImGui::SetFocusID(id, g.CurrentWindow);
            ImGui::FocusWindow(g.CurrentWindow);
        } else if (g.ActiveId == id && ((io.MouseClicked[ImGuiMouseButton_Left] && !hovered) || !enabled)) {
            ImGui::ClearActiveID();
        }
        const bool active = g.ActiveId == id;
        const double now = ImGui::GetTime();
        if (active) {
            for (const ImGuiKey key : {ImGuiKey_LeftArrow, ImGuiKey_RightArrow, ImGuiKey_Delete, ImGuiKey_Backspace, ImGuiKey_Home, ImGuiKey_End})
                ImGui::SetKeyOwner(key, id);
            if (multiline) {
                ImGui::SetKeyOwner(ImGuiKey_UpArrow, id);
                ImGui::SetKeyOwner(ImGuiKey_DownArrow, id);
                g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Up) | (1 << ImGuiDir_Down);
            }
            if (clicked)
                ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
            g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
        }

        // A single line starts where its alignment puts it, or scrolled while longer than the field; wrapped lines at the
        // left edge.
        const auto origin = [&] {
            const float width = offsets.back();
            if (multiline)
                return rect.Min.x;
            if (width > room)
                return rect.Min.x - state.scroll;
            return style.alignment == TextAlignment::Center ? rect.Min.x + (room - width) * 0.5f : style.alignment == TextAlignment::Trailing ? rect.Max.x - width : rect.Min.x;
        };
        const float line_height = Px(style.font.lineHeight);
        const auto line_x = [&](int index, int l) {
            return offsets[size_t(index)] - offsets[size_t(lines[size_t(l)].start)];
        };
        // The index nearest to x (from the text's left) on a line.
        const auto nearest_on = [&](int l, float x) {
            const int start = lines[size_t(l)].start;
            int nearest = start;
            for (int i = start + 1; i <= ReachableEnd(lines, l); ++i) {
                if (ImFabs(line_x(i, l) - x) < ImFabs(line_x(nearest, l) - x))
                    nearest = i;
            }
            return nearest;
        };
        const auto hit = [&](ImVec2 point) {
            const int l = multiline ? ImClamp(int(std::floor((point.y - rect.Min.y) / line_height)), 0, int(lines.size()) - 1) : 0;
            return nearest_on(l, point.x - origin());
        };

        // Mouse: a click places the caret (Shift extends), a double click selects a word, a triple click the line or
        // paragraph.
        if (active && clicked) {
            state.typing = false;
            state.column = -1.0f;
            const int index = hit(io.MousePos);
            const int clicks = io.MouseClickedCount[ImGuiMouseButton_Left];
            if (clicks >= 3) {
                const Line& line = lines[size_t(LineOf(lines, index))];
                state.anchor = multiline ? line.start : 0;
                state.caret = multiline ? ImMin(line.next, count) : count;
                // A paragraph runs from line feed to line feed across its wraps.
                while (multiline && state.anchor > 0 && shown[Typography::CodepointOffset(shown, state.anchor - 1)] != '\n')
                    --state.anchor;
                while (multiline && state.caret < count && shown[Typography::CodepointOffset(shown, state.caret - 1)] != '\n')
                    ++state.caret;
            } else if (clicks == 2) {
                const std::vector<bool> words = WordMask(*text);
                const int at = ImMin(index, count - 1);
                if (at >= 0 && words[size_t(at)]) {
                    state.anchor = at;
                    while (state.anchor > 0 && words[size_t(state.anchor - 1)])
                        --state.anchor;
                    state.caret = at;
                    while (state.caret < count && words[size_t(state.caret)])
                        ++state.caret;
                }
            } else {
                state.caret = index;
                if (!io.KeyShift)
                    state.anchor = index;
            }
            state.dragging = clicks == 1;
            state.moved = now;
        }
        if (state.dragging) {
            if (active && io.MouseDown[ImGuiMouseButton_Left]) {
                state.caret = hit(io.MousePos);
                state.moved = now;
            } else {
                state.dragging = false;
            }
        }

        // Keyboard: moves, deletion, clipboard and typed characters; Option (Ctrl off macOS) works by words, Command by
        // lines, and in wrapped text up and down keep their column.
        if (active) {
            const bool mac = io.ConfigMacOSXBehaviors;
            const bool by_word = mac ? io.KeyAlt : io.KeyCtrl;
            const bool by_line = mac && io.KeySuper;
            const bool extend = io.KeyShift;
            const std::vector<bool> words = WordMask(*text);
            const auto move = [&](int target) {
                state.caret = ImClamp(target, 0, count);
                if (!extend)
                    state.anchor = state.caret;
                state.moved = now;
                state.typing = false;
                state.column = -1.0f;
            };
            const auto replace = [&](std::string_view insert, bool typing = false) {
                if (!(typing && state.typing)) {
                    state.undo.push_back({*text, state.caret, state.anchor});
                    if (state.undo.size() > UndoLimit)
                        state.undo.erase(state.undo.begin());
                }
                state.redo.clear();
                state.typing = typing;
                state.column = -1.0f;
                const int from = ImMin(state.caret, state.anchor);
                const size_t begin = Typography::CodepointOffset(*text, from);
                const size_t end = Typography::CodepointOffset(*text, ImMax(state.caret, state.anchor));
                text->replace(begin, end - begin, insert);
                state.caret = state.anchor = from + Typography::CodepointCount(insert);
                count = Typography::CodepointCount(*text);
                state.moved = now;
                result.changed = true;
            };
            const bool selected = state.caret != state.anchor;
            const auto selection = [&] {
                const size_t begin = Typography::CodepointOffset(*text, ImMin(state.caret, state.anchor));
                return text->substr(begin, Typography::CodepointOffset(*text, ImMax(state.caret, state.anchor)) - begin);
            };
            const int caret_line = LineOf(lines, state.caret);
            const int line_start = multiline ? lines[size_t(caret_line)].start : 0;
            const int line_end = multiline ? ReachableEnd(lines, caret_line) : count;

            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
                if (selected && !extend && !by_word && !by_line)
                    move(ImMin(state.caret, state.anchor));
                else
                    move(by_line ? line_start : by_word ? WordLeft(words, state.caret) : state.caret - 1);
            } else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
                if (selected && !extend && !by_word && !by_line)
                    move(ImMax(state.caret, state.anchor));
                else
                    move(by_line ? line_end : by_word ? WordRight(words, state.caret) : state.caret + 1);
            } else if (multiline && (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_DownArrow))) {
                const int direction = ImGui::IsKeyPressed(ImGuiKey_UpArrow) ? -1 : 1;
                const int target = caret_line + direction;
                const float column = state.column >= 0.0f ? state.column : line_x(state.caret, caret_line);
                if (by_line || target < 0 || target >= int(lines.size()))
                    move(direction < 0 ? 0 : count);
                else
                    move(nearest_on(target, column));
                state.column = column;
            } else if (ImGui::IsKeyPressed(ImGuiKey_Home)) {
                move(0);
            } else if (ImGui::IsKeyPressed(ImGuiKey_End)) {
                move(count);
            } else if (ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
                if (!selected)
                    state.anchor = by_line ? line_start : by_word ? WordLeft(words, state.caret) : ImMax(state.caret - 1, 0);
                replace("");
            } else if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
                if (!selected)
                    state.anchor = by_word ? WordRight(words, state.caret) : ImMin(state.caret + 1, count);
                replace("");
            }

            // ⌘Z undoes, ⇧⌘Z (or Ctrl+Y on Windows keyboards) redoes.
            const auto restore = [&](std::vector<Snapshot>& from, std::vector<Snapshot>& to) {
                if (from.empty())
                    return;
                to.push_back({*text, state.caret, state.anchor});
                *text = from.back().text;
                state.caret = from.back().caret;
                state.anchor = from.back().anchor;
                from.pop_back();
                count = Typography::CodepointCount(*text);
                state.typing = false;
                state.column = -1.0f;
                state.moved = now;
                result.changed = true;
            };
            if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z, ImGuiInputFlags_Repeat, id))
                restore(state.undo, state.redo);
            if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z, ImGuiInputFlags_Repeat, id) || ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Y, ImGuiInputFlags_Repeat, id))
                restore(state.redo, state.undo);

            if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_A, ImGuiInputFlags_None, id)) {
                state.anchor = 0;
                state.caret = count;
            }
            if (!style.secure && selected && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_C, ImGuiInputFlags_None, id))
                ImGui::SetClipboardText(selection().c_str());
            if (!style.secure && selected && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_X, ImGuiInputFlags_None, id)) {
                ImGui::SetClipboardText(selection().c_str());
                replace("");
            }
            if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_V, ImGuiInputFlags_Repeat, id)) {
                // A single line keeps the first line of the clipboard; wrapped text all of it, with its line ends as line
                // feeds.
                const std::string_view clipboard = ImGui::GetClipboardText() ? ImGui::GetClipboardText() : "";
                if (multiline) {
                    std::string pasted;
                    for (size_t i = 0; i < clipboard.size(); ++i) {
                        if (clipboard[i] != '\r')
                            pasted += clipboard[i];
                        else if (i + 1 >= clipboard.size() || clipboard[i + 1] != '\n')
                            pasted += '\n';
                    }
                    replace(pasted);
                } else {
                    replace(clipboard.substr(0, clipboard.find_first_of("\r\n")));
                }
            }

            if (!io.InputQueueCharacters.empty()) {
                if (!(io.KeyCtrl && !io.KeyAlt)) {
                    std::string typed;
                    for (const ImWchar character : io.InputQueueCharacters) {
                        if (character < 32 || character == 127)
                            continue;
                        char utf8[5];
                        typed.append(utf8, size_t(ImTextCharToUtf8(utf8, character)));
                    }
                    if (!typed.empty())
                        replace(typed, true);
                    const char* completion = !typed.empty() && !multiline && state.caret == count ? Completion(*text, style.completions) : nullptr;
                    if (completion) {
                        const int typed_count = count;
                        text->append(completion + text->size());
                        count = Typography::CodepointCount(*text);
                        state.anchor = typed_count;
                        state.caret = count;
                    }
                }
                io.InputQueueCharacters.resize(0);
            }

            const ImGuiInputFlags enter_flags = multiline ? ImGuiInputFlags_Repeat : ImGuiInputFlags_None;
            if (ImGui::Shortcut(ImGuiKey_Enter, enter_flags, id) || ImGui::Shortcut(ImGuiKey_KeypadEnter, enter_flags, id)) {
                if (multiline) {
                    replace("\n");
                } else {
                    result.submitted = true;
                    state.anchor = 0;
                    state.caret = count;
                }
            }
            // Tab ends editing without claiming the key, so that ImGui's tabbing moves the focus on in the same press.
            if (ImGui::Shortcut(ImGuiKey_Escape, ImGuiInputFlags_None, id) || ImGui::IsKeyPressed(ImGuiKey_Tab, false))
                ImGui::ClearActiveID();
        }
        // A preview draws the field focused, with the insertion point after the text or all of it selected.
        const InteractionPreview preview = enabled ? Environment().interactionPreview : InteractionPreview::None;
        const bool previewed = preview == InteractionPreview::Focused || preview == InteractionPreview::TextSelected;
        const bool focused = g.ActiveId == id || previewed;
        result.focused = focused;

        if (result.changed) {
            shown = Shown(*text, style.secure, bullets);
            Typography::Offsets(style.font, shown, offsets);
            count = int(offsets.size()) - 1;
            lines = Lines(shown, offsets, multiline, room);
        }
        // Scrolling keeps the caret in view while editing; an idle field shows the start of a long text.
        const float width = offsets.back();
        if (multiline || width <= room || !focused)
            state.scroll = 0.0f;
        else
            state.scroll = ImClamp(ImClamp(state.scroll, offsets[size_t(state.caret)] - room + Px(1.0f), offsets[size_t(state.caret)]), 0.0f, width - room + Px(1.0f));

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const float left = origin();
        const float top = multiline ? rect.Min.y : rect.GetCenter().y - line_height * 0.5f;
        const int caret_index = previewed ? count : state.caret;
        const int anchor_index = preview == InteractionPreview::TextSelected ? 0 : previewed ? count : state.anchor;
        const int selection_from = ImMin(caret_index, anchor_index);
        const int selection_to = ImMax(caret_index, anchor_index);
        // The insertion point at either edge shows whole.
        const ImRect clip(rect.Min - ImVec2(Px(1.0f), 0.0f), rect.Max + ImVec2(Px(1.0f), 0.0f));
        draw->PushClipRect(clip.Min, clip.Max, true);
        const ImRect visible(draw->GetClipRectMin(), draw->GetClipRectMax());
        for (size_t l = 0; l < lines.size(); ++l) {
            const Line& line = lines[l];
            const float line_top = top + float(l) * line_height;
            if (line_top > visible.Max.y || line_top + line_height < visible.Min.y)
                continue;
            // A selection that runs on past a line's end fills to the right edge, as NSTextView does.
            const int from = ImMax(selection_from, line.start);
            const int to = ImMin(selection_to, line.end);
            const bool through = selection_to > line.end && selection_from <= line.end && l + 1 < lines.size();
            if (focused && selection_from != selection_to && (from < to || through)) {
                const float x0 = left + line_x(from, int(l));
                const float x1 = through ? ImMax(rect.Max.x, left + line_x(to, int(l))) : left + line_x(to, int(l));
                Draw::FillRect(draw, ImRect(x0, line_top, x1, line_top + line_height), colors.textSelection);
            }
            const size_t begin = Typography::CodepointOffset(shown, line.start);
            const std::string_view run = shown.substr(begin, Typography::CodepointOffset(shown, line.end) - begin);
            if (!run.empty()) {
                const float run_left = left + offsets[size_t(line.start)] - (multiline ? offsets[size_t(line.start)] : 0.0f);
                const ImRect frame = multiline ? ImRect(run_left, line_top, rect.Max.x, line_top + line_height) : ImRect(run_left, rect.Min.y, run_left + width, rect.Max.y);
                Typography::Draw(draw, style.font, frame, style.color, run);
            }
        }
        if (shown.empty() && !style.placeholder.empty())
            Typography::Draw(draw, style.font, multiline ? ImRect(rect.Min.x, top, rect.Max.x, top + line_height) : rect, colors.tertiaryLabel, style.placeholder, style.alignment);
        // The insertion point blinks in the accent color and hides while text is selected.
        const int caret_line = LineOf(lines, caret_index);
        const float caret = left + line_x(caret_index, caret_line);
        const float caret_top = top + (multiline ? float(caret_line) * line_height : 0.0f);
        if (focused && caret_index == anchor_index && (previewed || std::fmod(now - state.moved, 1.0) < 0.5)) {
            const float half = Px(Metrics::TextField().caretWidth) * 0.5f;
            Draw::FillRoundedRect(draw, ImRect(caret - half, caret_top, caret + half, caret_top + line_height), CornerRadii(half), colors.accent, CornerStyle::Circular);
        }
        draw->PopClipRect();

        if (g.ActiveId == id) {
            // The scrolling window follows the insertion point as it moves between wrapped lines.
            if (multiline && state.moved == now)
                ImGui::ScrollToRectEx(g.CurrentWindow, ImRect(caret, caret_top, caret + 1.0f, caret_top + line_height), ImGuiScrollFlags_KeepVisibleEdgeY);
            // Tells the platform where typing happens, which also makes some backends send characters at all.
            ImGuiPlatformImeData& ime = g.PlatformImeData;
            ime.WantVisible = true;
            ime.WantTextInput = true;
            ime.InputPos = ImVec2(caret - 1.0f, caret_top);
            ime.InputLineHeight = line_height;
            ime.ViewportId = g.CurrentWindow->Viewport->ID;
        }
        return result;
    }
} // namespace Cupertino::TextInput
