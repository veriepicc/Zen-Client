module;
#include <cstdint>
#include <string>
#include <filesystem>
#include <wincodec.h>

export module Utils;

export struct Color
{
    std::uint8_t r { 255 };
    std::uint8_t g { 255 };
    std::uint8_t b { 255 };
    std::uint8_t a { 255 };

    constexpr Color() = default;
    constexpr Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}

    static constexpr Color fromRGBA(std::uint32_t rgba)
    {
        return Color(
            static_cast<std::uint8_t>((rgba >> 24) & 0xFF),
            static_cast<std::uint8_t>((rgba >> 16) & 0xFF),
            static_cast<std::uint8_t>((rgba >> 8) & 0xFF),
            static_cast<std::uint8_t>(rgba & 0xFF));
    }

    constexpr std::uint32_t toRGBA() const
    {
        return (static_cast<std::uint32_t>(r) << 24) |
               (static_cast<std::uint32_t>(g) << 16) |
               (static_cast<std::uint32_t>(b) << 8)  |
               static_cast<std::uint32_t>(a);
    }

    static consteval Color White() { return Color(255, 255, 255, 255); }
    static consteval Color Black() { return Color(0, 0, 0, 255); }
    static consteval Color Red()   { return Color(255, 0, 0, 255); }
    static consteval Color Green() { return Color(0, 255, 0, 255); }
    static consteval Color Blue()  { return Color(0, 0, 255, 255); }
};

export std::string GetRoamingPath() {
    wchar_t* env;
    size_t size;
    if (!_wdupenv_s(&env, &size, L"localappdata") && env) {
        std::wstring wstr = std::wstring(env).substr(0, lstrlenW(env) - 2) + L"RoamingState";
        delete env;
        return std::filesystem::path(wstr).string();
    }
    return {};
}