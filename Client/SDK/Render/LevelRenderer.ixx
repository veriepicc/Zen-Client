export module LevelRenderer;

import OffsetManager;
import Memory;
import Math;

export class LevelRendererPlayer {
public:
	Math::Vec3<float> getCameraPos() {
		return Memory::MemberAt<Math::Vec3<float>>(this, Offsets::LevelRendererPlayer_cameraPos);
	}
};

export class LevelRenderer {
public:
	LevelRendererPlayer* getLevelRendererPlayer() {
		return Memory::MemberAt<LevelRendererPlayer*>(this, Offsets::LevelRenderer_LevelRendererPlayer);
	}
};