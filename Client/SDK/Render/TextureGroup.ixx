module;
#include <map>
#include <optional>
#include <string_view>

export module TextureGroup;

import SDK;
import TextureGroupBase;
import ResourceLocation;
import BedrockTexture;
import Memory;
import OffsetManager;
import SigManager;
import TexturePtr;
import Paul;

export namespace cg {
    export class ImageBuffer {
    public:
        void* blob{nullptr};
        int width{0};
        int height{0};
        int bytesPerPixel{0};
        ImageBuffer() = default;
        ImageBuffer(void* data, int w, int h, int bpp) : blob(data), width(w), height(h), bytesPerPixel(bpp) {}
    };
};
export class TextureGroup : public mce::TextureGroupBase {
public:
    std::map<ResourceLocation, BedrockTexture, ResourceLocation::Less>& getLoadedTextures() {
        return Memory::MemberAt<std::map<ResourceLocation, BedrockTexture, ResourceLocation::Less>>(this, Offsets::TextureGroup_base + Offsets::TextureGroup_loadedTextures);
    }

    BedrockTexture* uploadTexture(ResourceLocation& resource, cg::ImageBuffer imageBuffer) {
        return Memory::CallFunc<BedrockTexture*, TextureGroup*, ResourceLocation&, cg::ImageBuffer>(SigManager::TextureGroup_uploadTexture_imageBuffer, this, resource, imageBuffer);
    }

    void unloadAllTextures(__int64 a2 = 0, __int64 a3 = 0, __int64 a4 = 0) {
        Memory::CallFunc<void, TextureGroup*, __int64, __int64, __int64>(SigManager::TextureGroup_unloadAllTextures, this, a2, a3, a4);
    }

    // Note: actual virtual getTexture is provided by the engine; we don't re-declare here to avoid mismatched vtable.
};

static SDK::CoreRegistration Register{"TextureGroup"};


