#include "TokenField.h"

#include "core/Draw.h"
#include "core/Environment.h"
#include "core/Interaction.h"
#include "core/Metrics.h"
#include "core/State.h"
#include "core/TextInput.h"
#include "core/Theme.h"
#include "layout/Layout.h"

namespace Cupertino {
    struct TokenFieldState {
        std::string text;
    };

    bool TokenField(const char* label, std::vector<std::string>* tokens, const TokenFieldOptions& options) {
        const Metrics::TokenFieldMetrics& metrics = Metrics::TokenField();
        const Font& font = options.font;
        const float line = font.lineHeight + 2.0f * metrics.tokenInset;
        const float width = options.width > 0.0f ? options.width : Pt(Layout::Proposal().x);
        const float height = options.height > 0.0f ? options.height : line + 2.0f * metrics.lineTop;
        const ImRect frame = Layout::Place(Layout::Placement{.size = Px(ImVec2(width, height)), .baseline = Px(metrics.lineTop + metrics.tokenInset) + Typography::Baseline(font)});
        if (Layout::IsMeasuring())
            return false;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const Palette& colors = Theme::Colors();
        const bool enabled = Environment().enabled;
        {
            const Interaction::DisabledFade fade;
            Draw::FillRect(draw, frame, colors.fieldBackground);
            Draw::StrokeRoundedRect(draw, frame, CornerRadii(0.0f), colors.separator, Px(metrics.border), StrokeAlignment::Inside);
        }

        // Tokens run in lines from the top-leading corner; the text being typed takes the rest of the last line.
        const ImGuiID id = ImGui::GetID(label);
        TokenFieldState& state = State::Get<TokenFieldState>(id);
        const float left = frame.Min.x + Px(metrics.textInset);
        const float right = frame.Max.x - Px(metrics.textInset);
        ImVec2 pen(left, frame.Min.y + Px(metrics.lineTop));
        for (const std::string& token : *tokens) {
            const float token_width = Typography::Width(font, token) + Px(2.0f * metrics.tokenPadding);
            if (pen.x + token_width > right && pen.x > left)
                pen = ImVec2(left, pen.y + Px(line + metrics.lineSpacing));
            const ImRect plate(pen, pen + ImVec2(token_width, Px(line)));
            Draw::FillRoundedRect(draw, plate, CornerRadii(Px(metrics.tokenRadius)), colors.tokenBackground);
            Typography::Draw(draw, font, plate, colors.LabelColor(enabled), token, TextAlignment::Center);
            pen.x = plate.Max.x + Px(metrics.tokenSpacing);
        }

        bool changed = false;
        const ImRect input(pen.x + (tokens->empty() ? 0.0f : Px(metrics.tokenPadding)), pen.y, right, pen.y + Px(line));
        const TextInput::Style style = {.font = font, .color = colors.LabelColor(enabled), .placeholder = tokens->empty() && options.placeholder ? options.placeholder : ""};
        if (enabled) {
            const bool empty_before = state.text.empty();
            const TextInput::Result result = TextInput::Edit(ImGui::GetID("input"), input, &state.text, style);
            if (result.focused)
                Draw::FocusRing(draw, frame, CornerRadii(0.0f), colors.accent);
            const size_t comma = state.text.find(',');
            if ((result.submitted || comma != std::string::npos) && !state.text.empty()) {
                std::string entry = state.text.substr(0, comma);
                while (!entry.empty() && entry.back() == ' ')
                    entry.pop_back();
                if (!entry.empty()) {
                    tokens->push_back(entry);
                    changed = true;
                }
                state.text.clear();
            } else if (result.focused && empty_before && state.text.empty() && !tokens->empty() && ImGui::IsKeyPressed(ImGuiKey_Backspace, false)) {
                tokens->pop_back();
                changed = true;
            }
        } else if (tokens->empty() && options.placeholder) {
            Typography::Draw(draw, font, input, colors.tertiaryLabel, options.placeholder);
        }
        return changed;
    }
} // namespace Cupertino
