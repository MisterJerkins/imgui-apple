"""Builds SF Symbols fonts for the small and large image scales from SF Pro Text.

SF Pro maps every symbol codepoint (U+100000 and up) to its ".medium" glyph; the ".small" and ".large" variants have
no codepoints. This tool keeps only the symbol glyphs of one scale and maps the same codepoints to them, so ImGui can
load each scale as an ordinary font. Output: reference/fonts/derived/SF-Symbols-{Small,Large}-{Weight}.otf
Usage: tools/.venv/Scripts/python.exe tools/fonts/symbol_scales.py
"""

import os

from fontTools import subset
from fontTools.ttLib import TTFont
from fontTools.ttLib.tables._c_m_a_p import CmapSubtable

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
FONTS = os.path.join(REPO, "reference", "fonts")
OUTPUT = os.path.join(FONTS, "derived")
WEIGHTS = ["Regular", "Medium", "Semibold", "Bold", "Heavy"]
SCALES = ["Small", "Large"]


def build(weight, scale):
    font = TTFont(os.path.join(FONTS, f"SF-Pro-Text-{weight}.otf"))
    names = set(font.getGlyphOrder())
    suffix = "." + scale.lower()
    mapping = {}
    for code, glyph in font.getBestCmap().items():
        if code >= 0x100000 and glyph.endswith(".medium"):
            target = glyph[: -len(".medium")] + suffix
            if target in names:
                mapping[code] = target

    options = subset.Options()
    options.glyph_names = True
    options.notdef_outline = True
    options.layout_features = []
    options.name_IDs = ["*"]
    options.drop_tables += ["GSUB", "GPOS", "GDEF", "trak", "morx", "feat", "kern", "meta"]
    subsetter = subset.Subsetter(options)
    subsetter.populate(glyphs=sorted(set(mapping.values())))
    subsetter.subset(font)

    table = CmapSubtable.newSubtable(12)
    table.platformID = 3
    table.platEncID = 10
    table.language = 0
    table.cmap = mapping
    font["cmap"].tables = [table]
    path = os.path.join(OUTPUT, f"SF-Symbols-{scale}-{weight}.otf")
    font.save(path)
    return path, len(mapping)


def main():
    os.makedirs(OUTPUT, exist_ok=True)
    for scale in SCALES:
        for weight in WEIGHTS:
            path, count = build(weight, scale)
            print(f"{count} symbols -> {os.path.relpath(path, REPO)} ({os.path.getsize(path) // 1024} KB)")


if __name__ == "__main__":
    main()
