export module BedrockTextureData;

import SDK;

export enum class TextureLoadState : int {
    None = 0,
    LoadedBit = 1,
};

// Minimal surface to satisfy UI draw calls; flesh out as needed
export class BedrockTextureData {
public:
    TextureLoadState textureLoadState{TextureLoadState::None};
};

static SDK::CoreRegistration Register{"BedrockTextureData"};


