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
        if (renderContext)
        {
            auto screenContext = Memory::MemberAt<ScreenContext*>(reinterpret_cast<char*>(renderContext), Offsets::MinecraftUIRenderContext_screenContext);
            if (!screenContext) {
                if (State::originalFunction) State::originalFunction(screenView, renderContext);
                return;
            }
            auto tessellator = Memory::MemberAt<Tessellator*>(reinterpret_cast<char*>(screenContext), Offsets::ScreenContext_tessellator);
            if (!tessellator) {
                if (State::originalFunction) State::originalFunction(screenView, renderContext);
                return;
            }

            tessellator->begin(mce::PrimitiveMode::TriangleStrip, 3);

            tessellator->vertex(0.f, 0.f, 0.f);
            tessellator->vertex(100.f, 0.f, 0.f);
            tessellator->vertex(100.f, 100.f, 0.f);
            tessellator->vertex(0.f, 100.f, 0.f);

            HashedString ui_fill("ui_fill_color");
            auto mat = MaterialPtr::createMaterial(ui_fill);

            MeshHelpers::renderMeshImmediately(screenContext, tessellator, mat);
            
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


