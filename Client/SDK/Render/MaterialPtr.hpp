#pragma once

#include <Memory/Memory.hpp>
#include <Memory/SigManager.hpp>
#include <Memory/OffsetManager.hpp>
#include "HashedString.hpp"

class MaterialPtr {
public:
	static MaterialPtr* createMaterial(HashedString const& name) {
		static void* materialCreator = nullptr;

		if (materialCreator == nullptr) {
			__int64 sigOffset = reinterpret_cast<__int64>(SigManager::createMaterial);
			int offset = *reinterpret_cast<int*>(sigOffset + 3);
			materialCreator = reinterpret_cast<void*>(sigOffset + offset + 7);
		}

		return Memory::call_virtual_func<MaterialPtr*, HashedString const&>(1, materialCreator, name);
	}
};