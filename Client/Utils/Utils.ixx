module;
#include <cstdint>
#include <string>
#include <filesystem>
#include <wincodec.h>
#pragma comment(lib, "urlmon.lib")

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

export namespace Utils {
    std::string GetRoamingPath() {
        std::string path;
        wchar_t* env;
        size_t size;
        if (!_wdupenv_s(&env, &size, L"localappdata") && env) {
            std::wstring localAppData = env;
            delete env;
            
            std::wstring packagesPath = localAppData + L"\\Packages";
            std::wstring educationPath = packagesPath + L"\\Microsoft.MinecraftEducationEdition_8wekyb3d8bbwe\\RoamingState";
            
            if (std::filesystem::exists(educationPath)) {
                path = std::filesystem::path(educationPath).string();
            } else {
                std::wstring bedrockPath = packagesPath + L"\\Microsoft.MinecraftUWP_8wekyb3d8bbwe\\RoamingState";
                if (std::filesystem::exists(bedrockPath)) {
                    path = std::filesystem::path(bedrockPath).string();
                } else {
                    std::wstring fallbackPath = localAppData + L"\\RoamingState";
                    path = std::filesystem::path(fallbackPath).string();
                }
            }
        }
        return path;
    }
    void DownloadFile(std::string url, std::string path) {
        URLDownloadToFileA(nullptr, url.c_str(), path.c_str(), 0, NULL);
    }
    __int64 getBase() {
        return reinterpret_cast<__int64>(GetModuleHandleA(NULL));
	}
}