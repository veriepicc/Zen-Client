module;
#include <unordered_map>
#include <memory>

export module LevelRenderer;

import SigManager;
import OffsetManager;
import Memory;
import Paul;

export class LevelRendererPlayer {
public:
	Paul::Vec3<float> getCameraPos() {
		return Memory::MemberAt<Paul::Vec3<float>>(this, Offsets::LevelRendererPlayer_cameraPos);
	}
};

export class LevelRenderer {
public:
	LevelRendererPlayer* getLevelRendererPlayer() {
		return Memory::MemberAt<LevelRendererPlayer*>(this, Offsets::LevelRenderer_LevelRendererPlayer);
	}
	//char pad[0x28];
	//RenderChunkMap renderChunkCoordinators;

	//std::unordered_map<int, std::unique_ptr<RenderChunkCoordinator>> getRenderChunkCoordinators() {
	//	return Memory::MemberAt<std::unordered_map<int, std::unique_ptr<RenderChunkCoordinator>>>(this, 0x28);
	//}
};