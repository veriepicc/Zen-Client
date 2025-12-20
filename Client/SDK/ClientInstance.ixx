module;
#include <cstdint>

export module ClientInstance;

import Memory;
import SigManager;
import OffsetManager;
import LevelRenderer;
import BlockSource;
import LocalPlayer;

export class ClientInstance {
public:
	LevelRenderer* getLevelRenderer() {
		return Memory::CallVFunc<LevelRenderer*>(Offsets::ClientInstance_getLevelRenderer_idx, this);
	}
	LocalPlayer* getLocalPlayer() {
		static int index = 0;
		if (index == 0) {
			if (void* sig = SigManager::ClientInstance_getLocalPlayerIndex) {
				index = *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(sig) + 9) / 8;
			} else {
				index = 28;
			}
		}
		return Memory::CallVFunc<LocalPlayer*>(index, this); 
	}
	BlockSource* getRegion() {
		return Memory::CallVFunc<BlockSource*>(30, this);
	}
};