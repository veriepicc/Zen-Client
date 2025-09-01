export module TextureGroupBase;

import SDK;

export namespace mce {
    class TextureGroupBase {
    public:
        virtual ~TextureGroupBase() = default;
    };
}

static SDK::CoreRegistration Register{"TextureGroupBase"};


