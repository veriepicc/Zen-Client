module;
#include <iostream>

export module GetDefaultBlockState;

import Memory;
import HookManager;
import SigManager;
import OffsetManager;
import HashedString;
import Global;
import Module;

using GetDefaultBlockStateFunction = uint64_t*(*)(HashedString*, int);
GetDefaultBlockStateFunction originalFunction = nullptr;

uint64_t* Detour(HashedString* name, int a2) {
	std::cout << "[GetDefaultBlockState] name=" << name->text << std::endl;
    return originalFunction(name, a2);
}

export namespace Hooks::Other::GetDefaultBlockState {
    inline bool Install()
    {
        void* target = SigManager::BlockTypeRegistry_getDefaultBlockState;
        if (!target)
        {
            std::cout << "[GetDefaultBlockState] signature not resolved" << std::endl;
            return false;
        }

        auto& hookManager = GetHookManager();
        bool ok = hookManager.hook<GetDefaultBlockStateFunction>(
            target,
            Detour,
            &originalFunction
        );
        if (ok) std::cout << "[GetDefaultBlockState] hook installed" << std::endl;
        return ok;
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Other::GetDefaultBlockState::Install };