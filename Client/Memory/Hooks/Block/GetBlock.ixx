module;
#include <iostream>

export module GetBlock;

import Memory;
import HookManager;
import SigManager;
import BlockSource;
import Paul;

using GetBlockFunction = Block*(*)(void*, const Paul::BlockPos*);
static GetBlockFunction originalFunction = nullptr;

Block* BlockTessCache_GetBlock(void* self, const Paul::BlockPos* pos)
{
    (void)pos;
    return originalFunction(self, pos);
}

export namespace Hooks::Block::GetBlock
{
    inline bool Install()
    {
        void* target = SigManager::BlockTessellatorCache_getBlock;
        if (!target)
        {
            std::cout << "[GetBlock] signature not resolved" << std::endl;
            return false;
        }
        auto& hookManager = GetHookManager();
        bool ok = hookManager.hook<GetBlockFunction>(target, BlockTessCache_GetBlock, &originalFunction);
        if (ok) std::cout << "[GetBlock] hook installed" << std::endl;
        return ok;
    }
}

//static HookRegistry::Registration RegisterHook{ &Hooks::Block::GetBlock::Install };


