export module MeshHelpers;

import Tesselator;
import ScreenContext;
import SigManager;
import MaterialPtr;
import Memory;

export namespace MeshHelpers {
	void renderMeshImmediately(ScreenContext* sc, Tessellator* tess, MaterialPtr* mat) {
		char pad[0x58]{};
		auto func = SigManager::MeshHelper_renderMeshImmediately;
		Memory::CallFunc<void, ScreenContext*, Tessellator*, MaterialPtr*, char*>(func, sc, tess, mat, pad);
	}
}