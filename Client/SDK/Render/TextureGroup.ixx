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

namespace cg { class ImageBuffer {}; };
export class TextureGroup : public mce::TextureGroupBase {
public:
    std::map<ResourceLocation, BedrockTexture, ResourceLocation::Less>& getLoadedTextures() {
        return Memory::MemberAt<std::map<ResourceLocation, BedrockTexture, ResourceLocation::Less>>(this, Offsets::TextureGroup_base + Offsets::TextureGroup_loadedTextures);
    }

    BedrockTexture* uploadTexture(TextureGroup* group, ResourceLocation& resource, cg::ImageBuffer imageBuffer) {
        return Memory::CallFunc<BedrockTexture*, TextureGroup*, ResourceLocation&, cg::ImageBuffer>(SigManager::TextureGroup_uploadTexture_imageBuffer, this, resource, imageBuffer);
    }

    // Note: actual virtual getTexture is provided by the engine; we don't re-declare here to avoid mismatched vtable.
};

static SDK::CoreRegistration Register{"TextureGroup"};


