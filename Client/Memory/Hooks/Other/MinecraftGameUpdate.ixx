module;
#include <iostream>

export module MinecraftGameUpdate;

import Memory;
import HookManager;
import SigManager;
import Global;

namespace Hooks::Other::MinecraftGameUpdate
{
    using UpdateFunction = std::uint64_t(*)(void* /*minecraftGame*/);
    static UpdateFunction originalFunction = nullptr;

    std::uint64_t Detour(void* self)
    {
        return originalFunction(self);
    }

    export inline bool Install()
    {
        void* target = SigManager::MinecraftGame_update;
        if (!target)
        {
            return false;
        }

        auto& hookManager = GetHookManager();
        bool ok = hookManager.hook<UpdateFunction>(
            target,
            Detour,
            &originalFunction
        );
        return ok;
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Other::MinecraftGameUpdate::Install };


