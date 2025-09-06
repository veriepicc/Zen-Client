export module ClientInstance;

import Memory;
import SigManager;
import OffsetManager;
import LevelRenderer;

export class ClientInstance {
public:
	LevelRenderer* getLevelRenderer() {
		return Memory::CallVFunc<LevelRenderer*>(Offsets::ClientInstance_getLevelRenderer_idx, this);
	}
};