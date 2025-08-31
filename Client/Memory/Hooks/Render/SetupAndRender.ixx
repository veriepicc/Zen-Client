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

            HashedString ui_fill("ui_fill_color");
            auto mat = MaterialPtr::createMaterial(ui_fill);

            auto start = Math::Vec2<float>(0.f, 0.f);
            auto end = Math::Vec2<float>(100.f, 100.f);
            float lineWidth = 3.f;

            float modX = 0 - (start.y - end.y);
            float modY = start.x - end.x;

            float len = sqrtf(modX * modX + modY * modY);

            modX /= len;
            modY /= len;
            modX *= lineWidth;
            modY *= lineWidth;

            tessellator->begin(mce::PrimitiveMode::TriangleStrip, 6);

            tessellator->vertex(start.x + modX, start.y + modY, 0);
            tessellator->vertex(start.x - modX, start.y - modY, 0);
            tessellator->vertex(end.x - modX, end.y - modY, 0);

            tessellator->vertex(start.x + modX, start.y + modY, 0);
            tessellator->vertex(end.x + modX, end.y + modY, 0);
            tessellator->vertex(end.x - modX, end.y - modY, 0);

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


