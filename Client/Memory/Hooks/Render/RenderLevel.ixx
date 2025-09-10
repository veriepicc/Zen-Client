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
    static bool loggedOnce = false;
    static int frameCounter = 0;
    if (originalFunction)
        originalFunction(a1, screenContext, a3);

    if (a1 && screenContext)
    {
        if (!loggedOnce)
        {
            std::cout << "[RenderLevel] first call ok lr=" << a1 << " sc=" << screenContext << std::endl;
            loggedOnce = true;
        }
        if ((++frameCounter % 120) == 0)
        {
            auto cam = a1->getLevelRendererPlayer()->getCameraPos();
            std::cout << std::format("[RenderLevel] cam=({}, {}, {})", cam.x, cam.y, cam.z) << std::endl;
        }
        Modules::WorldRenderTick(a1, screenContext);
    }
}

export namespace Hooks::Render::RenderLevel {
    inline bool Install()
    {
        void* target = SigManager::LevelRenderer_renderLevel;
        if (!target)
        {
            std::cout << "[RenderLevel] signature not resolved" << std::endl;
            return false;
        }

        auto& hookManager = GetHookManager();
        bool ok = hookManager.hook<RenderLevelFunction>(
            target,
            LevelRenderer_renderLevel,
            &originalFunction
        );
        if (ok) std::cout << "[RenderLevel] hook installed" << std::endl;
        return ok;
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Render::RenderLevel::Install };