module;
#include <cstdint>

export module OffsetManager;

export namespace Offsets
{
    // Global/client offsets
    inline constexpr std::uint32_t ClientInstance_guiData = 0x5B8; // 1.21.8X+ latest known

    // MinecraftUIRenderContext offsets (set latest values when available)
    inline constexpr std::uint32_t MinecraftUIRenderContext_clientInstance = 0x08; // 1.20.3X+ latest known
    inline constexpr std::uint32_t MinecraftUIRenderContext_screenContext = 0x10;  // 1.20.3X+ latest known
    inline constexpr std::uint32_t MinecraftUIRenderContext_textures = 0x48;       // 1.20.3X+ latest known

    // ScreenContext offsets
    inline constexpr std::uint32_t ScreenContext_colorHolder = 0x30; // 1.20.3X+ latest known
    inline constexpr std::uint32_t ScreenContext_tessellator = 0xC8;  // 1.21.4X

    // GuiData offsets (1.21.6X+ known working; adjust via signatures if needed)
    inline constexpr std::uint32_t GuiData_ScreenSize = 0x40;
    inline constexpr std::uint32_t GuiData_ScreenSizeScaled = 0x50;
    inline constexpr std::uint32_t GuiData_GuiScale = 0x5C;

    inline constexpr std::uint32_t TextureGroup_base = 0x18;          // 1.20.3X
    inline constexpr std::uint32_t TextureGroup_loadedTextures = 0x178;// 1.20.3X
}