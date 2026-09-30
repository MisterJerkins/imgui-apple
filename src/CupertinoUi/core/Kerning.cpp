#include "Kerning.h"

#include <algorithm>
#include <span>

namespace Cupertino::Kerning {
    struct KerningPair {
        unsigned int pair;
        short value;
    };

#include "generated/KerningTables.inc"

    static std::span<const KerningPair> PairsOf(Table table) {
        switch (table) {
            case Table::TextRegular:
                return SfProTextRegular;
            case Table::TextMedium:
                return SfProTextMedium;
            case Table::TextSemibold:
                return SfProTextSemibold;
            case Table::TextBold:
                return SfProTextBold;
            case Table::TextHeavy:
                return SfProTextHeavy;
            case Table::DisplayRegular:
                return SfProDisplayRegular;
            case Table::DisplayMedium:
                return SfProDisplayMedium;
            case Table::DisplaySemibold:
                return SfProDisplaySemibold;
            case Table::DisplayBold:
                return SfProDisplayBold;
            case Table::None:
                break;
        }
        return {};
    }

    int Adjustment(Table table, unsigned left, unsigned right) {
        if (left > 0xFFFF || right > 0xFFFF)
            return 0;
        const std::span<const KerningPair> pairs = PairsOf(table);
        const unsigned int key = (left << 16) | right;
        const auto it = std::lower_bound(pairs.begin(), pairs.end(), key, [](const KerningPair& pair, unsigned int value) { return pair.pair < value; });
        return it != pairs.end() && it->pair == key ? it->value : 0;
    }
} // namespace Cupertino::Kerning
