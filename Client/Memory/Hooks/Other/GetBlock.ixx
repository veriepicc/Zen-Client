module;
#include <iostream>

export module GetBlock;

import Memory;
import HookManager;
import SigManager;
import OffsetManager;
import LevelRenderer;
import ScreenContext;
import BlockSource;
import Global;
import Math;

using GetBlockFunction = Block*(*)(void*, Math::BlockPos*);
GetBlockFunction originalFunction = nullptr;

Block* BlockTessCache_GetBlock(void* a1, Math::BlockPos* pos) {
	static Block* dirtBlock = nullptr;
    if (!dirtBlock) {
        auto ret = originalFunction(a1, pos);
		auto name = ret ? ret->getBlockLegacy()->getName() : "null";
        if (name == "dirt") {
			dirtBlock = ret;
        }
        return ret;
        //std::cout << "[GetBlock] pos=(" << pos->x << "," << pos->y << "," << pos->z << ")" << " name: " << ret->getBlockLegacy()->getName() << std::endl;
    }
    else {
        return dirtBlock;
    }
}

export namespace Hooks::Other::GetBlock {
    inline bool Install()
    {
        void* target = SigManager::BlockTessellatorCache_getBlock;
        if (!target)
        {
            std::cout << "[GetBlock] signature not resolved" << std::endl;
            return false;
        }

        auto& hookManager = GetHookManager();
        bool ok = hookManager.hook<GetBlockFunction>(
            target,
            BlockTessCache_GetBlock,
            &originalFunction
        );
        if (ok) std::cout << "[GetBlock] hook installed" << std::endl;
        return ok;
    }
}

//static HookRegistry::Registration RegisterHook{ &Hooks::Other::GetBlock::Install };