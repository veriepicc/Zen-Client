module;
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <optional>
#include <span>

#include <libhat/libhat.hpp>

export module OffsetManager;

export class OffsetManager
{
public:
    void set(std::string_view name, std::uint32_t value)
    {
        offsets[std::string(name)] = value;
    }

    std::optional<std::uint32_t> get(std::string_view name) const
    {
        if (auto it = offsets.find(std::string(name)); it != offsets.end()) return it->second;
        return std::nullopt;
    }

    void clear()
    {
        offsets.clear();
    }

    // Bulk setup helpers
    void addStatic(std::string_view name, std::uint32_t value)
    {
        set(name, value);
    }

    bool addFromSig(std::string_view name, std::string_view idaPattern, std::string_view section = {})
    {
        return setBySignature(name, idaPattern, section).has_value();
    }

    // Try to resolve and set an offset by signature (IDA-style string)
    std::optional<std::uint32_t> setBySignature(std::string_view name, std::string_view idaPattern, std::string_view section = {})
    {
        const auto parsed = hat::parse_signature(idaPattern);
        if (!parsed.has_value()) return std::nullopt;

        hat::scan_result res;
        if (section.empty()) res = hat::find_pattern(parsed.value());
        else                 res = hat::find_pattern(parsed.value(), section);
        if (!res.has_result()) return std::nullopt;

        auto value = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(res.get()));
        set(name, value);
        return value;
    }

private:
    std::unordered_map<std::string, std::uint32_t> offsets;
};


