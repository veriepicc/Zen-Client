module;
#include <cstdint>

export module OffsetManager;

// Minimal, constexpr-style offsets to avoid initializer/linker and IntelliSense issues
export namespace Offsets
{
    // Global/client offsets
    inline constexpr std::uint32_t ClientInstance_guiData = 0x5B0;

    // MinecraftUIRenderContext offsets (set latest values when available)
    inline constexpr std::uint32_t MinecraftUIRenderContext_clientInstance = 0x08; // latest known
    inline constexpr std::uint32_t MinecraftUIRenderContext_screenContext = 0x10;  // latest known
    inline constexpr std::uint32_t MinecraftUIRenderContext_textures = 0x48;       // latest known

    // ScreenContext offsets
    inline constexpr std::uint32_t ScreenContext_colorHolder = 0x30;
    inline constexpr std::uint32_t ScreenContext_tessellator = 0xC8;
}