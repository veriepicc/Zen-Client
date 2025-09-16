module;
#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <span>
#include <unordered_map>
#include <functional>
#include <iostream>

#include <libhat/libhat.hpp>

export module SigManager;

import Memory;

export enum class SigType { Sig, ReferenceSig };

#define REGISTER_SIG(Name, Pattern, Type, Offset) \
public: \
static inline void* Name; \
private: \
static inline std::function<void()> Name##Reg = ( \
    initializers.emplace_back( \
        (Type == SigType::Sig) \
            ? Memory::MakeSigInitializer(#Name, hat::compile_signature<Pattern>(), Offset, &Name, sigs, ".text") \
            : Memory::MakeRefSigInitializer(#Name, hat::compile_signature<Pattern>(), Offset, &Name, sigs, ".text") \
    ), \
    std::function<void()>() \
); \
public:

export class SigManager {
private:
    static inline std::vector<std::function<void()>> initializers;
    static inline std::unordered_map<std::string, std::uintptr_t> sigs;

public:
    static void initialize()
    {
        for (const auto& init : initializers) init();
        Memory::ResolveSignatureQueue(sigs);
    }

    static void deinitialize()
    {
        initializers.clear();
        sigs.clear();
    }

    static const std::unordered_map<std::string, std::uintptr_t>& getSigs()
    {
        return sigs;
    }

    static void logStatus()
    {
        std::cout << "[Sig] entries=" << sigs.size() << std::endl;
        for (const auto& kv : sigs)
        {
            std::cout << "  - " << kv.first << " = 0x" << std::hex << kv.second << std::dec << std::endl;
        }
    }

    REGISTER_SIG(ContainerScreenController_onContainerSlotHovered,
                 "48 89 ? ? ? 48 89 ? ? ? 41 ? 41 ? 41 ? 48 83 EC ? 45 33 ? 45 8B",
                 SigType::Sig,
                 0)
    REGISTER_SIG(BlockTypeRegistry_getDefaultBlockState,
                 "48 89 5C 24 ? 55 56 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 48 8B DA 48 8B F1 33 FF",
                 SigType::Sig,
                 0)
    REGISTER_SIG(MinecraftGame_update,
                 "48 8B ? 48 89 ? ? 48 89 ? ? 48 89 ? ? 55 41 ? 41 ? 41 ? 41 ? 48 8D ? ? ? ? ? 48 81 EC ? ? ? ? 0F 29 ? ? 0F 29 ? ? 48 8B ? ? ? ? ? 48 33 ? 48 89 ? ? ? ? ? 4C 8B ? C7 44 24 50",
                 SigType::Sig,
                 0)
    REGISTER_SIG(Setupandrender,
                 "48 8B C4 48 89 ?? ?? 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 0F 29 70 ?? 0F 29 78 ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 85 ?? ?? ?? ?? 4C 8B FA 48 89 55",
                 SigType::Sig,
                 0)
    REGISTER_SIG(LevelRenderer_renderLevel,
                 "48 8B ? 48 89 ? ? 55 56 57 41 ? 41 ? 41 ? 41 ? 48 8D ? ? ? ? ? 48 81 EC ? ? ? ? 0F 29 ? ? 0F 29 ? ? 44 0F ? ? ? 44 0F ? ? ? 48 8B ? ? ? ? ? 48 33 ? 48 89 ? ? ? ? ? 4D 8B ? 4C 8B ? 4C 8B ? 33 D2",
                 SigType::Sig,
                 0)
    REGISTER_SIG(BaseLightTextureImageBuilder_createBaseLightTextureData,
                 "48 89 5C 24 ? 48 89 54 24 ? 55 56 57 41 56 41 57 48 83 EC ? 4D 8B F1 49 8B F8",
                 SigType::Sig,
                 0)
    REGISTER_SIG(Gamemode,
                 "48 8B 89 ? ? ? ? 48 8B 01 48 8B 40 18 FF 15 ? ? ? ? 8B 43 24 89 43 20 48 83 C4 20 5B",
                 SigType::Sig,
                 0)

    // Tessellator signatures
    REGISTER_SIG(Tessellator_begin_a,
                 "40 53 55 48 83 EC 28 80 B9",
                 SigType::Sig,
                 0)
    REGISTER_SIG(Tessellator_vertexUV_a,
                 "48 83 EC ? 80 B9 ? ? ? ? ? 0F 57 E4",
                 SigType::Sig,
                 0)
    REGISTER_SIG(Tessellator_begin_b,
                 "48 89 5C 24 ? 56 48 83 EC ? 80 B9 ? ? ? ? ? 45 0F B6",
                 SigType::Sig,
                 0)
    REGISTER_SIG(Tessellator_vertex,
                 "40 57 48 81 EC ? ? ? ? 0F 29 7C ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 8B 81",
                 SigType::Sig,
                 0)
    REGISTER_SIG(Tessellator_vertexUV_b,
                 "48 83 EC ? 48 8B 94",
                 SigType::Sig,
                 0)
    REGISTER_SIG(Tessellator_colorF,
                 "80 B9 ? ? ? ? ? 4C 8B C1 75",
                 SigType::Sig,
                 0)
    REGISTER_SIG(Tessellator_setRotation,
                 "48 89 5C 24 ? 57 48 81 EC ? ? ? ? F3 41 0F 10",
                 SigType::Sig,
                 0)
    REGISTER_SIG(Tessellator_resetTransform,
                 "40 53 48 81 EC ? ? ? ? C6 81 ? ? ? ? ? 48 8B D9 84 D2",
                 SigType::Sig,
                 0)

    REGISTER_SIG(MeshHelper_renderMeshImmediately,
                "40 ? 55 56 57 41 ? 48 81 EC ? ? ? ? 49 8B ? 4D 8B ? 48 8B ? 48 8B ? 80 BA 0D 02 00 00 ? 0F 85 ? ? ? ? 48 8D ? ? ? ? ? 48 89 ? ? ? 48 C7 44 24 58 ? ? ? ? 0F 28 ? ? ? 66 0F ? ? ? ? C7 44 24 20 ? ? ? ? 4C 8D ? ? ? 41 B8 ? ? ? ? 48 8D ? ? ? ? ? ? 48 8B ? E8 ? ? ? ? 48 8B ? C6 84 24 A0 00 00 00 ? 33 D2",
                SigType::Sig,
                0)

    // MeshHelpers::renderMeshImmediately2 (textured) - latest known cross-version signature
    REGISTER_SIG(MeshHelper_renderMeshImmediately2,
                "40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 98 FC FF FF 48 81 EC 68 04 00 00 49",
                SigType::Sig,
                0)

    REGISTER_SIG(createMaterial,
                "48 8B 05 ? ? ? ? 48 8D 55 90 48 8D 0D ? ? ? ? 48 8B 40 08 FF 15 ? ? ? ? 48 8B D8",
                SigType::Sig,
                0)
    REGISTER_SIG(BlockTessellatorCache_getBlock,
                "48 89 ? ? ? 57 48 83 EC ? 48 8B ? 48 8B ? 48 8B ? 48 85 ? 75 ? 48 8D",
                SigType::Sig,
                0)
    REGISTER_SIG(RenderChunkCoordinator_dirtyAll,
                "48 89 ? ? ? 48 89 ? ? ? 48 89 ? ? ? 57 41 ? 41 ? 41 ? 41 ? 48 81 EC ? ? ? ? 48 8B ? ? ? ? ? 48 33 ? 48 89 ? ? ? ? ? ? 44 88",
                SigType::Sig,
                0)
    // RenderMaterialGroup::ui
    REGISTER_SIG(RenderMaterialGroup_ui,
                 "48 8B 05 ? ? ? ? 48 8D 55 90 48 8D 0D ? ? ? ? 48 8B 40 08 FF 15 ? ? ? ? 48 8B D8",
                 SigType::Sig,
                 0)
    // TextureGroup::uploadTexture(resource, cg::ImageBuffer) - version dependent
    REGISTER_SIG(TextureGroup_uploadTexture_imageBuffer,
                 "48 89 ? ? ? 55 56 57 41 ? 41 ? 48 8D ? ? ? 48 81 EC ? ? ? ? 48 8B ? ? ? ? ? 48 33 ? 48 89 ? ? 49 8B ? 48 8B ? 48 8B ? 4C 89 ? ? 48 8D",
                 SigType::Sig,
                 0)
    REGISTER_SIG(TextureGroup_unloadAllTextures,
                 "48 89 ? ? ? 48 89 ? ? ? 48 89 ? ? ? 57 41 ? 41 ? 48 83 EC ? 4C 8B ? 48 8B ? ? ? ? ? 48 8B ? 45 33",
                 SigType::Sig,
                 0)

    // Input feed hooks
    REGISTER_SIG(Keyboard_feed,
                 "? ? ? ? ? ? ? 4C 8D 05 ? ? ? ? 89 54 24 20 88",
                 SigType::Sig,
                 0)
    REGISTER_SIG(MouseDevice_feed,
                 "? ? ? ? ? ? ? ? 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 44 0F B7 BC 24 ? ? ? ? 48 8B D9",
                 SigType::Sig,
                 0)
};

