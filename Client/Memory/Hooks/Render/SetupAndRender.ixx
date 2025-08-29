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

namespace Hooks::Render::SetupAndRender
{
    using SetupAndRenderFunction = void(*)(void*, MinecraftUIRenderContext*);

    struct State
    {
        static inline SetupAndRenderFunction originalFunction = nullptr;
        static inline bool hasDrawnOnce = false;
    };

    inline void Detour(void* screenView, MinecraftUIRenderContext* renderContext)
    {
        if (!State::hasDrawnOnce && renderContext)
        {
            void* screenContext = Memory::MemberAt<void*>(reinterpret_cast<char*>(renderContext), Offsets::MinecraftUIRenderContext_screenContext);
            if (!screenContext) {
                if (State::originalFunction) State::originalFunction(screenView, renderContext);
                return;
            }
            void* tessellator = Memory::MemberAt<void*>(reinterpret_cast<char*>(screenContext), Offsets::ScreenContext_tessellator);
            if (!tessellator) {
                if (State::originalFunction) State::originalFunction(screenView, renderContext);
                return;
            }

            void* beginFunction = SigManager::Tessellator_begin_b ? SigManager::Tessellator_begin_b : SigManager::Tessellator_begin_a;
            void* colorFunction = SigManager::Tessellator_colorF;
            void* vertexFunction = SigManager::Tessellator_vertex;
            if (beginFunction && colorFunction && vertexFunction)
            {
                Memory::CallFunc<void, void*, mce::PrimitiveMode, int, bool>(beginFunction, tessellator, mce::PrimitiveMode::TriangleList, 3, false);
                Memory::CallFunc<void, void*, float, float, float, float>(colorFunction, tessellator, 1.0f, 1.0f, 1.0f, 1.0f);
                Memory::CallFunc<void, void*, float, float, float>(vertexFunction, tessellator, 10.0f, 10.0f, 0.0f);
                Memory::CallFunc<void, void*, float, float, float>(vertexFunction, tessellator, 110.0f, 10.0f, 0.0f);
                Memory::CallFunc<void, void*, float, float, float>(vertexFunction, tessellator, 60.0f, 110.0f, 0.0f);
                State::hasDrawnOnce = true;
            }
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


