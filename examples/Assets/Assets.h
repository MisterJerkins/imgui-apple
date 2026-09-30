#pragma once

#include "Cupertino.h"

#include <string>

namespace Examples {
    // A picture of the assets by its path there without the extension ("sidebar/siri"); empty without it.
    Cupertino::Bitmap Picture(const std::string& name);
    // The icon of a picture of the assets, or fallback without it.
    Cupertino::Icon PictureIcon(const std::string& name, const Cupertino::Icon& fallback);
} // namespace Examples
