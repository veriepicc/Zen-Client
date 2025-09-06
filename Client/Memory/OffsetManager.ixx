module;
#include <cstdint>

export module OffsetManager;

#define Offset(name, offset) inline constexpr std::uint32_t name = offset;

export namespace Offsets
{
    // Global/client offsets
    Offset(ClientInstance_guiData, 0x5B8) // 1.21.8X+ latest known

    // MinecraftUIRenderContext offsets (set latest values when available)
    Offset(MinecraftUIRenderContext_clientInstance, 0x08) // 1.20.3X+ latest known
    Offset(MinecraftUIRenderContext_screenContext, 0x10)  // 1.20.3X+ latest known
    Offset(MinecraftUIRenderContext_textures, 0x48)       // 1.20.3X+ latest known

    // ScreenContext offsets
    Offset(ScreenContext_colorHolder, 0x30) // 1.20.3X+ latest known
    Offset(ScreenContext_tessellator, 0xC8)  // 1.21.4X

    // GuiData offsets (1.21.6X+ known working; adjust via signatures if needed)
    Offset(GuiData_ScreenSize, 0x40)
    Offset(GuiData_ScreenSizeScaled, 0x50)
    Offset(GuiData_GuiScale, 0x5C)

    Offset(TextureGroup_base, 0x18)             // 1.20.3X
    Offset(TextureGroup_loadedTextures, 0x178)  // 1.20.3X

    Offset(ClientInstance_getLevelRenderer_idx, 218)
    Offset(LevelRenderer_LevelRendererPlayer, 0x3F0)
    Offset(LevelRendererPlayer_cameraPos, 0x664)
}