module;
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <optional>

export module SigManager;

export class SignatureManager
{
public:
    SignatureManager() = default;

    static std::optional<std::uintptr_t> scan(std::uintptr_t startAddress,
                                              std::size_t searchSize,
                                              std::string_view idaPattern) noexcept;
};

inline std::vector<std::optional<std::uint8_t>> patternToBytes(std::string_view pattern)
{
    std::vector<std::optional<std::uint8_t>> bytes;
    bytes.reserve(pattern.size() / 2);

    for (std::size_t i = 0; i < pattern.size();) {
        if (pattern[i] == ' ') { ++i; continue; }
        if (pattern[i] == '?') {
            if (i + 1 < pattern.size() && pattern[i + 1] == '?') ++i;
            ++i;
            bytes.emplace_back(std::nullopt);
            continue;
        }

        auto hexToNibble = [](char c) -> std::uint8_t {
            if (c >= '0' && c <= '9') return static_cast<std::uint8_t>(c - '0');
            if (c >= 'A' && c <= 'F') return static_cast<std::uint8_t>(10 + (c - 'A'));
            if (c >= 'a' && c <= 'f') return static_cast<std::uint8_t>(10 + (c - 'a'));
            return 0;
        };

        if (i + 1 < pattern.size()) {
            std::uint8_t high = hexToNibble(pattern[i]);
            std::uint8_t low  = hexToNibble(pattern[i + 1]);
            bytes.emplace_back(static_cast<std::uint8_t>((high << 4) | low));
            i += 2;
        } else {
            break;
        }
    }
    return bytes;
}

inline std::optional<std::uintptr_t> SignatureManager::scan(std::uintptr_t, std::size_t, std::string_view) noexcept
{
    return std::nullopt;
}


