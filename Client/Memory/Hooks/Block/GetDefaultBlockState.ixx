module;
#include <iostream>

export module GetDefaultBlockState;

import Memory;
import HookManager;
import SigManager;
import HashedString;

using GetDefaultBlockStateFunction = void*(*)(void* /*registry*/, const HashedString* /*name*/);
static GetDefaultBlockStateFunction originalFunction = nullptr;

//const struct Block * BlockTypeRegistry::getDefaultBlockState(void* lookupMap, const struct HashedString* hashedName)
//lookupMap = base + BlockTypeRegistry_mBlockLookupMap_global
void* Detour(void* self, const HashedString* name)
{
    return originalFunction(self, name);
}

export namespace Hooks::Block::GetDefaultBlockState
{
    inline bool Install()
    {
        void* target = SigManager::BlockTypeRegistry_getDefaultBlockState;
        if (!target)
        {
            return false;
        }
        auto& hookManager = GetHookManager();
        return hookManager.hook<GetDefaultBlockStateFunction>(target, Detour, &originalFunction);
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Block::GetDefaultBlockState::Install };


