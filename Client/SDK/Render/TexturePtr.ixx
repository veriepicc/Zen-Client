module;
#include <memory>
#include <string>

export module TexturePtr;

import SDK;
import ResourceLocation;
import BedrockTextureData;

export class TexturePtr {
public:
    std::shared_ptr<BedrockTextureData> clientTexture;
    std::shared_ptr<ResourceLocation> resourceLocation;

    TexturePtr() = default;

    [[nodiscard]] std::string GetFilePath() const {
        return resourceLocation ? resourceLocation->filePath : std::string{};
    }
};

static SDK::CoreRegistration Register{"TexturePtr"};


