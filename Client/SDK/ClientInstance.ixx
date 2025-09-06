export module ClientInstance;

import Memory;
import SigManager;
import OffsetManager;
import LevelRenderer;

export class ClientInstance {
public:
	LevelRenderer* getLevelRenderer() {
		return Memory::MemberAt<LevelRenderer*>(this, 0xE8);
	}
};