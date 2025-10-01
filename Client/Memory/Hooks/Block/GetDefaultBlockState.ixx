module;
#include <iostream>
#include <format>

export module GetDefaultBlockState;

import Memory;
import HookManager;
import SigManager;
import Utils;
import HashedString;
import OffsetManager;

using GetDefaultBlockStateFunction = void*(*)(void* /*registry*/, const HashedString& /*name*/);
static GetDefaultBlockStateFunction originalFunction = nullptr;

//const struct Block * BlockTypeRegistry::getDefaultBlockState(void* lookupMap, const struct HashedString* hashedName)
//lookupMap = base + BlockTypeRegistry_mBlockLookupMap_global
void* Detour(void* self, const HashedString& name)
{
    std::cout << std::format("GetDefaultBlockState called for block: {}, addr: {:p} {:x}", name.text, self, Utils::getBase() + Offsets::BlockTypeRegistry_mBlockLookupMap_global - 0x1000) << std::endl;
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

//static HookRegistry::Registration RegisterHook{ &Hooks::Block::GetDefaultBlockState::Install };


