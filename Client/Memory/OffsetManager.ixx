module;
#include <cstdint>

export module OffsetManager;

// Minimal, constexpr-style offsets to avoid initializer/linker and IntelliSense issues
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

    // TextureGroup offsets (placeholders; set to correct values for your version)
    inline constexpr std::uint32_t TextureGroup_base = 0x18;          // 1.20.3X
    inline constexpr std::uint32_t TextureGroup_loadedTextures = 0x178;// 1.20.3X
}