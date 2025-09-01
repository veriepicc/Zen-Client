module;
#include <map>

export module TextureGroup;

import SDK;
import TextureGroupBase;
import ResourceLocation;
import BedrockTexture;
import Memory;
import OffsetManager;
import TexturePtr;

export class TextureGroup : public mce::TextureGroupBase {
public:
    std::map<ResourceLocation, BedrockTexture, ResourceLocation::Less>& getLoadedTextures() {
        return Memory::MemberAt<std::map<ResourceLocation, BedrockTexture, ResourceLocation::Less>>(this, Offsets::TextureGroup_base + Offsets::TextureGroup_loadedTextures);
    }

    // Note: actual virtual getTexture is provided by the engine; we don't re-declare here to avoid mismatched vtable.
};

static SDK::CoreRegistration Register{"TextureGroup"};


