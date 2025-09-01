module;
#include <cstdint>
#include <iostream>

export module SetupAndRender;

import HookManager;
import SigManager;
import Memory;
import MinecraftUIRenderContext;
import PrimitiveMode;
import OffsetManager;
import HookManager;

import MeshHelpers;
import MaterialPtr;
import ScreenContext;
import HashedString;
import Tesselator;
import Math;
import TexturePtr;
import Color;
import BedrockTextureData;
import RectangleArea;

namespace Hooks::Render::SetupAndRender
{
    using SetupAndRenderFunction = void(*)(void*, MinecraftUIRenderContext*);
    using DrawImageFunction = void(*)(MinecraftUIRenderContext*, BedrockTextureData*, Math::Vec2<float>*, Math::Vec2<float>*, Math::Vec2<float>*, Math::Vec2<float>*, bool);

    struct State
    {
        static inline SetupAndRenderFunction originalFunction = nullptr;
        static inline bool imageInitialized = false;
        static inline TexturePtr imageTexture{};
        static inline DrawImageFunction originalDrawImage = nullptr;
        static inline bool drewOnceThisFrame = false;
    };

    inline void DrawImageDetour(MinecraftUIRenderContext* ctx,
                                BedrockTextureData* tex,
                                Math::Vec2<float>* pos,
                                Math::Vec2<float>* size,
                                Math::Vec2<float>* uvPos,
                                Math::Vec2<float>* uvSize,
                                bool flag)
    {
        if (!State::drewOnceThisFrame && State::imageInitialized && State::imageTexture.clientTexture)
        {
            auto p = Math::Vec2<float>(50.f, 50.f);
            auto s = Math::Vec2<float>(256.f, 256.f);
            auto up = Math::Vec2<float>(0.f, 0.f);
            auto us = Math::Vec2<float>(1.f, 1.f);
            // Clip to a smaller rect to test scissor; save/restore around our draw only
            ctx->saveCurrentClippingRectangle();
            // Engine expects x,y,width,height. Set then enable.
            auto clip = SDK::RectangleArea::FromLTRB(80.f, 80.f, 220.f, 220.f);
            ctx->setClippingRectangle(clip);
            ctx->enableScissorTest(clip);
            State::originalDrawImage(ctx, State::imageTexture.clientTexture.get(), &p, &s, &up, &us, false);
            mce::Color color(1.f, 1.f, 1.f, 1.f);
            HashedString pass("ui_textured");
            ctx->flushImages(color, 1.f, pass);
            ctx->restoreSavedClippingRectangle();
            State::drewOnceThisFrame = true;
        }
        State::originalDrawImage(ctx, tex, pos, size, uvPos, uvSize, flag);
    }

    inline void Detour(void* screenView, MinecraftUIRenderContext* renderContext)
    {
        if (!renderContext) {
            if (State::originalFunction) State::originalFunction(screenView, renderContext);
            return;
        }

        // Per-frame begin: reset once-per-frame guard
        State::drewOnceThisFrame = false;

        // Initialize external PNG texture once
        if (!State::imageInitialized)
        {
            const std::string path = "C:/Users/veriepic/AppData/Local/Packages/Microsoft.MinecraftUWP_8wekyb3d8bbwe/RoamingState/image.png";
            // Strict PNG: simple extension check
            if (path.size() >= 4 && (path.ends_with(".png") || path.ends_with(".PNG"))) {
                auto tmp = renderContext->createTexture(path, /*external*/true, /*forceReload*/true);
                State::imageTexture = std::move(tmp);
                State::imageTexture.resourceLocation = std::make_shared<ResourceLocation>(ResourceLocation(path, true));
                if (!State::imageTexture.clientTexture) {
                    // Hint the engine to pull it in
                    ResourceLocation rl(path, true);
                    renderContext->touchTexture(rl);
                }
                State::imageInitialized = State::imageTexture.clientTexture != nullptr;
            }
        }

        // Install drawImage vfunc hook once so we can inject before other UI draws
        if (!State::originalDrawImage)
        {
            void** vtable = *reinterpret_cast<void***>(renderContext);
            void* target = vtable[7];
            auto& hookManager = GetHookManager();
            hookManager.hook<DrawImageFunction>(target, DrawImageDetour, &State::originalDrawImage);
        }

        if (State::originalFunction)
            State::originalFunction(screenView, renderContext);
    }
}

export namespace Hooks::Render::SetupAndRender
{
    inline bool Install()
    {
        void* target = SigManager::Setupandrender;
        if (!target)
        {
            std::cout << "[SetupAndRender] signature not resolved" << std::endl;
            return false;
        }

        auto& hookManager = GetHookManager();
        return hookManager.hook<Hooks::Render::SetupAndRender::SetupAndRenderFunction>(
            target,
            Hooks::Render::SetupAndRender::Detour,
            &Hooks::Render::SetupAndRender::State::originalFunction
        );
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Render::SetupAndRender::Install };


