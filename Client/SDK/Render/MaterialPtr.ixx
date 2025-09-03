export module MaterialPtr;

import Memory;
import HashedString;
import SigManager;

export class MaterialPtr {
public:
	static MaterialPtr* createMaterial(HashedString const& name) {
		static void* materialCreator = nullptr;

		if (materialCreator == nullptr) {
			__int64 sigOffset = reinterpret_cast<__int64>(SigManager::createMaterial);
			int offset = *reinterpret_cast<int*>(sigOffset + 3);
			materialCreator = reinterpret_cast<void*>(sigOffset + offset + 7);
		}

		return Memory::CallVFunc<MaterialPtr*, HashedString const&>(1, materialCreator, name);
	}

	// Use RenderMaterialGroup::ui path to get UI materials, mirrors reference impl
	static MaterialPtr* createUIMaterial(HashedString const& name) {
		static void* uiGroup = nullptr;
		if (uiGroup == nullptr) {
			__int64 sigOffset = reinterpret_cast<__int64>(SigManager::RenderMaterialGroup_ui);
			if (sigOffset) {
				int offset = *reinterpret_cast<int*>(sigOffset + 3);
				uiGroup = reinterpret_cast<void*>(sigOffset + offset + 7);
			}
		}
		if (!uiGroup) return nullptr;
		return Memory::CallVFunc<MaterialPtr*, HashedString const&>(1, uiGroup, name);
	}
};