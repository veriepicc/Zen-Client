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

namespace Hooks::Render::SetupAndRender
{
    using SetupAndRenderFunction = void(*)(void*, MinecraftUIRenderContext*);

    struct State
    {
        static inline SetupAndRenderFunction originalFunction = nullptr;
        static inline bool imageInitialized = false;
        static inline TexturePtr imageTexture{};
    };

    inline void Detour(void* screenView, MinecraftUIRenderContext* renderContext)
    {
        if (State::originalFunction)
            State::originalFunction(screenView, renderContext);

        if (!renderContext) return;

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
                  //  renderContext->touchTexture(rl);
                }
                State::imageInitialized = State::imageTexture.clientTexture != nullptr;
            }
        }

        // Draw if texture is ready
        if (State::imageInitialized)
        {
            auto pos = Math::Vec2<float>(50.f, 50.f);
            auto size = Math::Vec2<float>(256.f, 256.f);
            auto uvPos = Math::Vec2<float>(0.f, 0.f);
            auto uvSize = Math::Vec2<float>(1.f, 1.f);

            renderContext->drawImage(State::imageTexture, pos, size, uvPos, uvSize);

            mce::Color color(1.f, 1.f, 1.f, 1.f);
            HashedString pass("ui_textured");
            renderContext->flushImages(color, 1.f, pass);
        }
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


