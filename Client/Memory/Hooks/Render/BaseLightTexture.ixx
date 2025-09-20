module;
#include <memory>
#include <iostream>

export module BaseLightTexture;

import HookManager;
import SigManager;

class BaseLightData {
public:
    char mSunriseColor[16]; //mce::Color
    float mGamma;
    float mSkyDarken;
    int mDimensionType;
    float mDarkenWorldAmount;
    float mPreviousDarkenWorldAmount;
    bool mNightvisionActive;
    float mNightvisionScale;
    bool mUnderwaterVision;
    float mUnderwaterScale;
    float mSkyFlashTime;
};

using CreateBaseLightFunction = std::unique_ptr<BaseLightData>*(*)(__int64, __int64*, __int64, __int64);
CreateBaseLightFunction originalFunction = nullptr;

std::unique_ptr<BaseLightData>* BaseLightTextureImageBuilder_createBaseLightTextureData(__int64 _this, __int64* result, __int64 client, __int64 currentData) {
    auto ret = originalFunction(_this, result, client, currentData);
    ret->get()->mNightvisionActive = true;
    ret->get()->mNightvisionScale = 1.f;

    result = (__int64*)(ret);
    return ret;
}

export namespace Hooks::Render::BaseLight {
    inline bool Install()
    {
        void* target = SigManager::BaseLightTextureImageBuilder_createBaseLightTextureData;
        if (!target)
        {
            std::cout << "[CreateBaseLight] signature not resolved" << std::endl;
            return false;
        }

        auto& hookManager = GetHookManager();
        return hookManager.hook<CreateBaseLightFunction>(
            target,
            BaseLightTextureImageBuilder_createBaseLightTextureData,
            &originalFunction
        );
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Render::BaseLight::Install };