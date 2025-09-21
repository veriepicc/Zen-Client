module;
#include <iostream>
#include <format>

export module RenderLevel;

import Memory;
import HookManager;
import SigManager;
import OffsetManager;
import LevelRenderer;
import ScreenContext;
import Module;

using RenderLevelFunction = void(*)(LevelRenderer*, ScreenContext*, void*);
RenderLevelFunction originalFunction = nullptr;

void LevelRenderer_renderLevel(LevelRenderer* a1, ScreenContext* screenContext, void* a3) {
    if (originalFunction)
        originalFunction(a1, screenContext, a3);

    if (a1 && screenContext)
    {
        Modules::WorldRenderTick(a1, screenContext);
    }
}

export namespace Hooks::Render::RenderLevel {
    inline bool Install()
    {
        void* target = SigManager::LevelRenderer_renderLevel;
        if (!target)
        {
            return false;
        }

        auto& hookManager = GetHookManager();
        bool ok = hookManager.hook<RenderLevelFunction>(
            target,
            LevelRenderer_renderLevel,
            &originalFunction
        );
        return ok;
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Render::RenderLevel::Install };