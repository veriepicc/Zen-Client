module;
#include <iostream>

export module GetDefaultBlockState;

import Memory;
import HookManager;
import SigManager;
import HashedString;

using GetDefaultBlockStateFunction = void*(*)(void* /*registry*/, const HashedString* /*name*/, int /*a2*/);
static GetDefaultBlockStateFunction originalFunction = nullptr;

void* Detour(void* self, const HashedString* name, int a2)
{
    return originalFunction(self, name, a2);
}

export namespace Hooks::Block::GetDefaultBlockState
{
    inline bool Install()
    {
        void* target = SigManager::BlockTypeRegistry_getDefaultBlockState;
        if (!target)
        {
            std::cout << "[GetDefaultBlockState] signature not resolved" << std::endl;
            return false;
        }
        auto& hookManager = GetHookManager();
        bool ok = hookManager.hook<GetDefaultBlockStateFunction>(target, Detour, &originalFunction);
        if (ok) std::cout << "[GetDefaultBlockState] hook installed" << std::endl;
        return ok;
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Block::GetDefaultBlockState::Install };


