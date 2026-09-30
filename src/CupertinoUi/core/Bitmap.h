#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include <span>
#include <string>

namespace Cupertino {
    // A bitmap image, like an image asset: its pixels live in a texture the renderer backend uploads; size is in points.
    struct Bitmap {
        ImTextureRef texture;
        ImVec2 size;

        bool IsEmpty() const {
            return size.x <= 0.0f || size.y <= 0.0f;
        }
    };

    // Reads an image file (PNG, JPEG, BMP) once per path and hands its pixels to the renderer backend; scale is the
    // file's pixels per point, 2 for an @2x asset. Returns an empty bitmap when the file cannot be read.
    Bitmap LoadBitmap(const std::string& path, float scale = 2.0f);
    // The same from the file's bytes in memory (a baked file), decoded once per key.
    Bitmap LoadBitmap(const std::string& key, std::span<const unsigned char> file, float scale = 2.0f);
} // namespace Cupertino
