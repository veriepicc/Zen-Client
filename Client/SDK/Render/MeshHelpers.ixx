module;
#include <iostream>
export module MeshHelpers;

import Memory;
import SigManager;
import ScreenContext;
import Tesselator;
import MaterialPtr;
import BedrockTextureData;

export namespace MeshHelpers {
	void renderMeshImmediately(ScreenContext* sc, Tessellator* tess, MaterialPtr* mat) {
		char pad[0x58]{};
		auto func = SigManager::MeshHelper_renderMeshImmediately;
		Memory::CallFunc<void, ScreenContext*, Tessellator*, MaterialPtr*, char*>(func, sc, tess, mat, pad);
	}
	void renderMeshImmediately2(ScreenContext* sc, Tessellator* tess, MaterialPtr* mat, BedrockTextureData& texture) {
		char pad[0x58]{};
		auto func = SigManager::MeshHelper_renderMeshImmediately2;
		if (!func) {
			std::cout << "[MeshHelpers] renderMeshImmediately2 signature unresolved" << std::endl;
			return;
		}
		Memory::CallFunc<void, ScreenContext*, Tessellator*, MaterialPtr*, BedrockTextureData&, char*>(func, sc, tess, mat, texture, pad);
	}
}