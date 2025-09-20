module;
#include <string>

export module BlockSource;

import Paul;
import Memory;

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