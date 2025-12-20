module;
#include <cstdint>

export module ClientInstance;

import Memory;
import SigManager;
import OffsetManager;
import LevelRenderer;
import BlockSource;
import LocalPlayer;
import GLMatrix;
import Paul;

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
	GLMatrix getViewMatrix() {
		return Memory::MemberAt<GLMatrix>(this, Offsets::ClientInstance_viewMatrix);
	}
	Paul::Vec2f getFov() {
		return {
			Memory::MemberAt<float>(this, Offsets::ClientInstance_fovX),
			Memory::MemberAt<float>(this, Offsets::ClientInstance_fovY)
		};
	}
};