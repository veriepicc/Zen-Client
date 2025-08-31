#pragma once

#include "Memory/Memory.hpp"
#include "Memory/SigManager.hpp"
#include "../SDK.hpp"

#include "MaterialPtr.hpp"
#include "HashedString.hpp"
#include "../Tessellator/ScreenContext.hpp"

namespace MeshHelper {
	static void renderMeshImmediately(ScreenContext* sc, Tessellator* tess, MaterialPtr* mat) {
		char pad[0x58]{};
		auto func = SigManager::MeshHelper_renderMeshImmediately;
		Memory::call_func_fastcall<void, ScreenContext*, Tessellator*, MaterialPtr*, char*>(func, sc, tess, mat, pad);
	}
}