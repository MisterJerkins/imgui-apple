#pragma once

#include <span>
#include <string_view>

namespace Cupertino {
    // A file compiled into the program by tools/bake.py: its path in the folder it came from and its bytes, kept as
    // little-endian words. The tool writes a table's files sorted by name.
    struct BakedFile {
        const char* name;
        const unsigned int* words;
        unsigned int size;
    };

    // The bytes of the file of a sorted table with that name; empty when there is none.
    std::span<const unsigned char> FindBakedFile(std::span<const BakedFile> files, std::string_view name);

    // The fonts baked into this build (SF Pro Text and Display, SF Mono, the symbol scales and layers); empty when it
    // was built without them.
    std::span<const BakedFile> BakedFonts();
} // namespace Cupertino
