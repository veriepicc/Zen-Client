export module ClientInstance;

import Memory;
import SigManager;
import OffsetManager;
import LevelRenderer;
import BlockSource;

export class ClientInstance {
public:
	LevelRenderer* getLevelRenderer() {
		return Memory::CallVFunc<LevelRenderer*>(Offsets::ClientInstance_getLevelRenderer_idx, this);
	}
	BlockSource* getRegion() {
		return Memory::CallVFunc<BlockSource*>(30, this);
	}
};