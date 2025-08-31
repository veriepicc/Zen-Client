#pragma once
#include <cstdint>
#include <iostream>

#include "../HookManager.hpp"
#include <Memory/Memory.hpp>
#include <Memory/SigManager.hpp>

#include <SDK/Render/MinecraftUIRenderContext.hpp>
#include <SDK/Render/MaterialPtr.hpp>
#include <SDK/Render/MeshHelper.hpp>

#include <SDK/Tessellator/ScreenContext.hpp>
#include <SDK/Tessellator/Tessellator.hpp>

#include <Module/Module.hpp>


namespace Hooks::Render::SetupAndRender
{
    using SetupAndRenderFunction = void(*)(void*, MinecraftUIRenderContext*);

    struct State
    {
        static inline SetupAndRenderFunction originalFunction = nullptr;
    };

    inline void Detour(void* screenView, MinecraftUIRenderContext* renderContext)
    {
        if (renderContext)
        {
            auto screenContext = Memory::MemberAt<ScreenContext*>(renderContext, Offsets::MinecraftUIRenderContext_screenContext);
            if (!screenContext) {
                if (State::originalFunction) State::originalFunction(screenView, renderContext);
                return;
            }
            auto tessellator = screenContext->getTessellator();
            if (!tessellator) {
                if (State::originalFunction) State::originalFunction(screenView, renderContext);
                return;
            }

            for (auto module : Modules::All()) {
                module->onRender(renderContext, screenContext);
            }

            static MaterialPtr* fillColor = nullptr;
            
            tessellator->begin(mce::PrimitiveMode::TriangleStrip, 3);
            
            tessellator->vertex(0.f, 0.f, 0.f);
            tessellator->vertex(100.f, 0.f, 0.f);
            tessellator->vertex(100.f, 100.f, 0.f);
            tessellator->vertex(0.f, 100.f, 0.f);
            
            HashedString fillColorStr("ui_fill_color");
            if (!fillColor) fillColor = MaterialPtr::createMaterial(fillColorStr);
            
            if (fillColor && tessellator && screenContext) {
                std::cout << "called" << std::endl;
                MeshHelper::renderMeshImmediately(screenContext, tessellator, fillColor);
            }
        }

        if (State::originalFunction)
            State::originalFunction(screenView, renderContext);
    }
}

namespace Hooks::Render::SetupAndRender
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


