#pragma once

#include "core/Typography.h"

#include <string>
#include <vector>

namespace Cupertino {
    struct TokenFieldOptions {
        // Shown while there are no tokens and no text.
        const char* placeholder = nullptr;
        // Size in points; a width of 0 takes the offered width, a height of 0 one line.
        float width = 0.0f;
        float height = 0.0f;
        Font font = Font::Style(TextStyle::Callout);
    };

    // NSTokenField, as Finder's tags field and Mail's address fields: a square white field in a 1 pt border, its entries
    // as rounded tokens in lines from the top-leading corner with the text being typed after them. Return or a comma
    // turns the text into a token, Delete in an empty field removes the last one. Returns true when the tokens change.
    bool TokenField(const char* label, std::vector<std::string>* tokens, const TokenFieldOptions& options = {});
} // namespace Cupertino
