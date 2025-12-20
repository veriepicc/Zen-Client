module;
#include <cstdint>

export module OffsetManager;

#define Offset(name, offset) inline constexpr std::uint32_t name = offset;

export namespace Offsets
{
    Offset(ClientInstance_guiData, 0x5B8)

    Offset(MinecraftUIRenderContext_clientInstance, 0x08)
    Offset(MinecraftUIRenderContext_screenContext, 0x10)
    Offset(MinecraftUIRenderContext_textures, 0x48)

    Offset(ScreenContext_colorHolder, 0x30)
    Offset(ScreenContext_tessellator, 0xC8)

    Offset(GuiData_ScreenSize, 0x40)
    Offset(GuiData_ScreenSizeScaled, 0x50)
    Offset(GuiData_GuiScale, 0x5C)

    Offset(TextureGroup_base, 0x18)
    Offset(TextureGroup_loadedTextures, 0x178)

    Offset(ClientInstance_getLevelRenderer_idx, 218)

    Offset(LevelRenderer_LevelRendererPlayer, 0x3F0)
    Offset(LevelRendererPlayer_cameraPos, 0x664)

    Offset(BlockTypeRegistry_mBlockLookupMap_global, 0x8D57F38)
}