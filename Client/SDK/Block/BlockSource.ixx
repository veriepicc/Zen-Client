module;
#include <string>

export module BlockSource;

import Paul;
import Memory;
import HashedString;
import Utils;
import OffsetManager;
import SigManager;

export class BlockLegacy {
public:
	std::string getName() {
		return Memory::MemberAt<std::string>(this, 0x98);
	}
};

export class Block {
public:
	BlockLegacy* getBlockLegacy() {
		return Memory::MemberAt<BlockLegacy*>(this, 0x78);
	}
};
export class BlockSource {
public:
	Block* getBlock(const Paul::BlockPos& pos) {
		return Memory::CallVFunc<Block*>(2, this, pos);
	}
};

export class BlockTypeRegistry {
public:
	static Block* getDefaultBlockState(const HashedString& name) {
		return Memory::CallFunc<Block*, void*, const HashedString&>(SigManager::BlockTypeRegistry_getDefaultBlockState, reinterpret_cast<void*>(Utils::getBase() + Offsets::BlockTypeRegistry_mBlockLookupMap_global - 0x1000), name);
	}
};