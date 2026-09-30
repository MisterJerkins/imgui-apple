"""Bakes the files the build carries as arrays of words: the fonts the library loads and the pictures of the examples.

Fonts: SF Pro Text and Display, SF Mono and the derived symbol fonts from reference/fonts, subset to the text an app
shows and the SF Symbols its code names, into src/CupertinoUi/core/generated/BakedFonts.inc. SF Pro carries every
SF Symbol (U+100000 and up), about 6 MB a face; the bake keeps Latin, Cyrillic, Greek, punctuation, arrows and math,
plus the symbols the source trees name (Symbols::Name, or codepoints 0x10XXXX outside the symbol table). The library
loads them from memory when Configuration::fontsDirectory is empty; --otf DIR also writes the subset files.
Pictures: every PNG under reference/assets, whole, into examples/Assets/BakedAssets.inc for Examples::Picture.
A part whose folder is missing keeps its baked file as it is.
Usage: tools/.venv/Scripts/python.exe tools/bake.py [SOURCE_DIR ...] [--fonts DIR] [--assets DIR] [--otf DIR] [--all-symbols]
"""

import argparse
import io
import os
import re

from fontTools import subset
from fontTools.ttLib import TTFont

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
FONTS = os.path.join(REPO, "reference", "fonts")
ASSETS = os.path.join(REPO, "reference", "assets")
FONTS_OUTPUT = os.path.join(REPO, "src", "CupertinoUi", "core", "generated", "BakedFonts.inc")
ASSETS_OUTPUT = os.path.join(REPO, "examples", "Assets", "BakedAssets.inc")
SYMBOLS_HEADER = os.path.join(REPO, "src", "CupertinoUi", "core", "Symbols.h")
SOURCES = [os.path.join(REPO, name) for name in ("src", "showcase", "examples")]
WEIGHTS = ["Regular", "Medium", "Semibold", "Bold", "Heavy"]
TEXT_RANGES = [
    (0x0020, 0x024F),  # Latin, Latin-1, Latin Extended-A and B
    (0x0370, 0x03FF),  # Greek
    (0x0400, 0x052F),  # Cyrillic and its supplement
    (0x1E00, 0x1EFF),  # Latin Extended Additional
    (0x2000, 0x206F),  # General Punctuation
    (0x20A0, 0x20CF),  # Currency
    (0x2100, 0x21FF),  # Letterlike symbols and arrows
    (0x2200, 0x22FF),  # Math operators
    (0x2300, 0x23FF),  # Technical: ⌘ ⌥ ⌃ ⇧ ⌫
    (0x25A0, 0x25FF),  # Geometric shapes
    (0x2600, 0x27BF),  # Symbols and dingbats
]


def symbol_table():
    table = {}
    pattern = re.compile(r"inline constexpr unsigned (\w+) = (0x[0-9A-Fa-f]+);")
    with open(SYMBOLS_HEADER, encoding="utf-8") as header:
        for line in header:
            match = pattern.search(line)
            if match:
                table[match.group(1)] = int(match.group(2), 16)
    return table


def used_symbols(directories, table):
    used = set()
    pattern = re.compile(r"Symbols::(\w+)")
    literal = re.compile(r"\b0x10[0-9A-Fa-f]{4}\b")
    codepoints = set(table.values())
    for directory in directories:
        for root, _, files in os.walk(directory):
            for name in files:
                path = os.path.abspath(os.path.join(root, name))
                # The symbol table itself names them all, and the bakes hold only numbers.
                if not name.endswith((".cpp", ".h", ".inc")) or path in (SYMBOLS_HEADER, FONTS_OUTPUT, ASSETS_OUTPUT):
                    continue
                with open(path, encoding="utf-8", errors="ignore") as source:
                    text = source.read()
                for match in pattern.finditer(text):
                    if match.group(1) in table:
                        used.add(table[match.group(1)])
                for match in literal.finditer(text):
                    if int(match.group(0), 16) in codepoints:
                        used.add(int(match.group(0), 16))
    return used


def subset_font(path, codepoints):
    font = TTFont(path)
    options = subset.Options()
    options.layout_features = ["*"]
    options.name_IDs = ["*"]
    options.notdef_outline = True
    options.glyph_names = False
    subsetter = subset.Subsetter(options)
    subsetter.populate(unicodes=codepoints)
    subsetter.subset(font)
    data = io.BytesIO()
    font.save(data)
    return data.getvalue()


def words(data):
    """The bytes as little-endian 32-bit words, eight to a line: an array of words compiles far faster than bytes."""
    padded = data + b"\0" * (-len(data) % 4)
    values = [int.from_bytes(padded[i:i + 4], "little") for i in range(0, len(padded), 4)]
    lines = []
    for i in range(0, len(values), 8):
        lines.append("    " + ", ".join(f"0x{value:08X}" for value in values[i:i + 8]) + ",")
    return "\n".join(lines)


def identifier(name):
    """A file's path as a C++ name: "sidebar/icloud-drive.png" -> SidebarIcloudDrive."""
    stem = os.path.splitext(name)[0]
    return "".join(part[:1].upper() + part[1:] for part in re.split(r"[^0-9A-Za-z]+", stem) if part)


def write_table(path, comment, table, files):
    """Writes the files (name, bytes) as word arrays and a table of Cupertino::BakedFile sorted by name."""
    files = sorted(files)
    arrays = [f"inline constexpr unsigned int {identifier(name)}[] = {{\n{words(data)}\n}};\n" for name, data in files]
    entries = [f'    {{"{name}", {identifier(name)}, {len(data)}}},' for name, data in files]
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="\n") as out:
        out.write(comment)
        out.write("\n".join(arrays))
        out.write(f"\ninline constexpr Cupertino::BakedFile {table}[] = {{\n" + "\n".join(entries) + "\n};\n")
    total = sum(len(data) for _, data in files)
    print(f"{len(files)} files, {total / 1048576:.1f} MB -> {os.path.relpath(path, REPO)}")


def bake_fonts(args):
    table = symbol_table()
    symbols = set(table.values()) if args.all_symbols else used_symbols(args.sources, table)
    text = {code for first, last in TEXT_RANGES for code in range(first, last + 1)}
    faces = []
    for weight in WEIGHTS:
        for family in ("Text", "Display"):
            faces.append((f"SF-Pro-{family}-{weight}.otf", text | symbols))
        # SF Mono draws text alone; symbols in it come from SF Pro.
        faces.append((f"SF-Mono-{weight}.otf", text))
        # The layers font maps codepoints of its own and is small: it is kept whole.
        for scale in ("Small", "Large", "Layers"):
            faces.append((f"derived/SF-Symbols-{scale}-{weight}.otf", None if scale == "Layers" else symbols))

    files = []
    for name, codepoints in faces:
        path = os.path.join(args.fonts, name)
        if not os.path.exists(path):
            continue
        if codepoints is None:
            with open(path, "rb") as file:
                data = file.read()
        else:
            data = subset_font(path, codepoints)
        if args.otf:
            target = os.path.join(args.otf, name)
            os.makedirs(os.path.dirname(target), exist_ok=True)
            with open(target, "wb") as file:
                file.write(data)
        files.append((name, data))
        print(f"{name}: {os.path.getsize(path) // 1024} KB -> {len(data) // 1024} KB")
    if not files:
        print(f"No fonts in {args.fonts}: {os.path.relpath(FONTS_OUTPUT, REPO)} stays as it is")
        return
    comment = "// Generated by tools/bake.py from SF Pro, SF Mono and the derived symbol fonts, subset to the text ranges and the\n"
    comment += f"// {len(symbols)} symbols the sources name; do not edit.\n\n"
    write_table(FONTS_OUTPUT, comment, "BakedFontFiles", files)


def bake_assets(args):
    files = []
    for root, _, names in os.walk(args.assets):
        for name in names:
            if name.endswith(".png"):
                path = os.path.join(root, name)
                with open(path, "rb") as file:
                    files.append((os.path.relpath(path, args.assets).replace("\\", "/"), file.read()))
    if not files:
        print(f"No pictures in {args.assets}: {os.path.relpath(ASSETS_OUTPUT, REPO)} stays as it is")
        return
    write_table(ASSETS_OUTPUT, "// Generated by tools/bake.py from the pictures of reference/assets; do not edit.\n\n", "BakedAssetFiles", files)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("sources", nargs="*", default=[path for path in SOURCES if os.path.isdir(path)])
    parser.add_argument("--fonts", default=FONTS)
    parser.add_argument("--assets", default=ASSETS)
    parser.add_argument("--otf")
    parser.add_argument("--all-symbols", action="store_true")
    args = parser.parse_args()
    bake_fonts(args)
    bake_assets(args)


if __name__ == "__main__":
    main()
