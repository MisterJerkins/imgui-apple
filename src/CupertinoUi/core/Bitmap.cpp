#include "Bitmap.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stb_image.h"

#include <cstring>
#include <memory>
#include <unordered_map>

namespace Cupertino {
    struct LoadedBitmap {
        std::unique_ptr<ImTextureData> texture;
        Bitmap bitmap;
    };

    // The bitmap decoded under key, decoding it with decode (stbi_load or stbi_load_from_memory) the first time.
    template <typename Decode>
    static Bitmap Load(const std::string& key, float scale, const Decode& decode) {
        static std::unordered_map<std::string, LoadedBitmap> loaded;
        const auto found = loaded.find(key);
        if (found != loaded.end())
            return found->second.bitmap;

        LoadedBitmap& entry = loaded[key];
        int width = 0;
        int height = 0;
        int channels = 0;
        stbi_uc* pixels = decode(&width, &height, &channels);
        if (!pixels)
            return entry.bitmap;
        // Registered as a user texture, the renderer backend creates it with the next frame's textures.
        entry.texture = std::make_unique<ImTextureData>();
        entry.texture->Create(ImTextureFormat_RGBA32, width, height);
        std::memcpy(entry.texture->Pixels, pixels, size_t(width) * size_t(height) * 4);
        entry.texture->UseColors = true;
        stbi_image_free(pixels);
        ImGui::RegisterUserTexture(entry.texture.get());
        entry.bitmap = Bitmap{entry.texture->GetTexRef(), ImVec2(float(width), float(height)) / scale};
        return entry.bitmap;
    }

    Bitmap LoadBitmap(const std::string& path, float scale) {
        return Load(path, scale, [&](int* width, int* height, int* channels) { return stbi_load(path.c_str(), width, height, channels, 4); });
    }

    Bitmap LoadBitmap(const std::string& key, std::span<const unsigned char> file, float scale) {
        return Load(key, scale, [&](int* width, int* height, int* channels) {
            return file.empty() ? nullptr : stbi_load_from_memory(file.data(), int(file.size()), width, height, channels, 4);
        });
    }
} // namespace Cupertino
