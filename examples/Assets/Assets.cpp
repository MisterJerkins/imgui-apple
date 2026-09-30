#include "Assets.h"

#include "Examples.h"

namespace Examples {
    namespace Baked {
#if __has_include("BakedAssets.inc")
#include "BakedAssets.inc"
#endif
    } // namespace Baked

    // Where the pictures come from: a folder while developing, otherwise the ones baked into the build.
    struct AssetSource {
        std::string directory;
        bool baked = true;
    };

    static AssetSource& Source() {
        static AssetSource source;
        return source;
    }

    void SetAssetsDirectory(const std::string& directory) {
        Source().directory = directory;
    }

    void UseBakedAssets(bool enabled) {
        Source().baked = enabled;
    }

    static std::span<const Cupertino::BakedFile> BakedAssets() {
#if __has_include("BakedAssets.inc")
        return Baked::BakedAssetFiles;
#else
        return {};
#endif
    }

    Cupertino::Bitmap Picture(const std::string& name) {
        const AssetSource& source = Source();
        if (!source.directory.empty())
            return Cupertino::LoadBitmap(source.directory + "/" + name + ".png");
        if (!source.baked)
            return {};
        const std::string file = name + ".png";
        const std::span<const unsigned char> bytes = Cupertino::FindBakedFile(BakedAssets(), file);
        return bytes.empty() ? Cupertino::Bitmap{} : Cupertino::LoadBitmap(file, bytes);
    }

    Cupertino::Icon PictureIcon(const std::string& name, const Cupertino::Icon& fallback) {
        const Cupertino::Bitmap picture = Picture(name);
        return picture.IsEmpty() ? fallback : Cupertino::Icon{.image = picture};
    }
} // namespace Examples
