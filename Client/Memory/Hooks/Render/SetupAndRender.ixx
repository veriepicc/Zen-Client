module;
#include <cstdint>
#include <iostream>

export module SetupAndRender;

import HookManager;
import SigManager;
import Memory;
import MinecraftUIRenderContext;
import Math;
import PrimitiveMode;
import OffsetManager;
import ScreenContext;

namespace Hooks::Render::Detail
{
    using SetupAndRenderFn = void(*)(void*, MinecraftUIRenderContext*);
    inline SetupAndRenderFn g_originalSetupAndRender { nullptr };
    inline bool g_drewTestTriangleOnce { false };

    // Detours
    inline void SetupAndRender_Detour(void* screenView, MinecraftUIRenderContext* ctx)
    {
        // Draw once, before original, to avoid use-after-free of screenContext on UI transitions
        if (!g_drewTestTriangleOnce && ctx)
        {
            void* sc = Memory::MemberAt<void*>(reinterpret_cast<char*>(ctx), OffsetManager::MinecraftUIRenderContext_screenContext);
            if (!sc) {
                if (g_originalSetupAndRender) g_originalSetupAndRender(screenView, ctx);
                return;
            }
            void* tess = Memory::MemberAt<void*>(reinterpret_cast<char*>(sc), OffsetManager::ScreenContext_tessellator);
            if (!tess) {
                if (g_originalSetupAndRender) g_originalSetupAndRender(screenView, ctx);
                return;
            }

            void* beginFn = SigManager::Tessellator_begin_b ? SigManager::Tessellator_begin_b : SigManager::Tessellator_begin_a;
            void* colorFn = SigManager::Tessellator_colorF;
            void* vertexFn = SigManager::Tessellator_vertex;
            if (beginFn && colorFn && vertexFn)
            {
                Memory::CallFunc<void, void*, mce::PrimitiveMode, int, bool>(beginFn, tess, mce::PrimitiveMode::TriangleList, 3, false);
                Memory::CallFunc<void, void*, float, float, float, float>(colorFn, tess, 1.0f, 1.0f, 1.0f, 1.0f);
                Memory::CallFunc<void, void*, float, float, float>(vertexFn, tess, 10.0f, 10.0f, 0.0f);
                Memory::CallFunc<void, void*, float, float, float>(vertexFn, tess, 110.0f, 10.0f, 0.0f);
                Memory::CallFunc<void, void*, float, float, float>(vertexFn, tess, 60.0f, 110.0f, 0.0f);
                g_drewTestTriangleOnce = true;
            }
        }

        if (g_originalSetupAndRender)
            g_originalSetupAndRender(screenView, ctx);
    }
}

export namespace Hooks::Render::SetupAndRender
{
    inline bool Register()
    {
        // Ensure signature resolved
        void* target = SigManager::Setupandrender;
        if (!target)
        {
            std::cout << "[SetupAndRender] signature not resolved" << std::endl;
            return false;
        }

        auto& hm = GetHookManager();
        return hm.hook<Hooks::Render::Detail::SetupAndRenderFn>(
            target,
            Hooks::Render::Detail::SetupAndRender_Detour,
            &Hooks::Render::Detail::g_originalSetupAndRender
        );
    }
}


