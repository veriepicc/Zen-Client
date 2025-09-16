module;
#include <iostream>

export module MCGameUpdate;

import Memory;
import HookManager;
import SigManager;
import OffsetManager;
import LevelRenderer;
import ScreenContext;
import BlockSource;
import Global;
import Math;
import Module;

using MCGameUpdateFunction = uint64_t(*)(void*);
MCGameUpdateFunction originalFunction = nullptr;

uint64_t MCGame_Update(void* a1) {
	auto enable = Modules::getRegistry().findByName("ClickGui")->enabledRef();
    if (enable) {
        auto ci = Global::getClientInstance();
        if (auto lr = ci->getLevelRenderer()) {
			printf("MCGame_Update: dirty all chunks\n");
            auto v6 = *(uint64_t**)(lr + 0x30);
            for (auto i = (uint64_t*)*v6; i != v6; i = (uint64_t*)*i)
                Memory::CallFunc<void>(SigManager::RenderChunkCoordinator_dirtyAll, i[3], true, true, true);
        }
    }
	return originalFunction(a1);
}

export namespace Hooks::Other::MCGameUpdate {
    inline bool Install()
    {
        void* target = SigManager::MinecraftGame_update;
        if (!target)
        {
            std::cout << "[MCGameUpdate] signature not resolved" << std::endl;
            return false;
        }

        auto& hookManager = GetHookManager();
        bool ok = hookManager.hook<MCGameUpdateFunction>(
            target,
            MCGame_Update,
            &originalFunction
        );
        if (ok) std::cout << "[MCGameUpdate] hook installed" << std::endl;
        return ok;
    }
}

//static HookRegistry::Registration RegisterHook{ &Hooks::Other::MCGameUpdate::Install };