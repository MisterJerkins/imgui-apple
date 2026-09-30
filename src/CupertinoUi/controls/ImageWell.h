#pragma once

#include "IconPlate.h"

namespace Cupertino {
    struct ImageWellOptions {
        // The frame: the kit's well is 34 pt square; a larger one keeps a wider rim (Metrics::ImageWell).
        ImVec2 size = ImVec2(34.0f, 34.0f);
    };

    // AppKit's image well: a sunken rounded rect with the image inset in it; an empty icon leaves it empty. Shows the
    // image; takes no drops.
    void ImageWell(const char* id, const Icon& image, const ImageWellOptions& options = {});
} // namespace Cupertino
