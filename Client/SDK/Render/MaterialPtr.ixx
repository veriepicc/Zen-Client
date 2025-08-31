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
};