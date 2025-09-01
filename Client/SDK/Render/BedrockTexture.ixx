module;
#include <memory>

export module BedrockTexture;

import SDK;
import BedrockTextureData;

// Minimal wrapper for texture group map
export class BedrockTexture {
public:
    std::shared_ptr<BedrockTextureData> bedrockTextureData;

    void unload() {
        if (bedrockTextureData) bedrockTextureData->textureLoadState = TextureLoadState::None;
    }
    bool operator!=(const BedrockTexture& rhs) const {
        return bedrockTextureData.get() != rhs.bedrockTextureData.get();
    }
};

static SDK::CoreRegistration Register{"BedrockTexture"};


