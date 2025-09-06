module;
#include <iostream>

export module RenderLevel;

import Memory;
import HookManager;
import SigManager;
import OffsetManager;
import LevelRenderer;
import ScreenContext;

using RenderLevelFunction = void(*)(LevelRenderer*, ScreenContext*, void*);
RenderLevelFunction originalFunction = nullptr;

void LevelRenderer_renderLevel(LevelRenderer* a1, ScreenContext* screenContext, void* a3) {
    //std::cout << "level renderer from hook: " << std::hex << a1 << std::endl;
    originalFunction(a1, screenContext, a3);
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
        return hookManager.hook<RenderLevelFunction>(
            target,
            LevelRenderer_renderLevel,
            &originalFunction
        );
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Render::RenderLevel::Install };