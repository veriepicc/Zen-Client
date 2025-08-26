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
    struct Entry { std::string name; std::uint32_t value; };

    void set(std::string name, std::uint32_t value)
    {
        offsets.emplace(std::move(name), value);
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
    void addStatic(std::string name, std::uint32_t value)
    {
        set(std::move(name), value);
    }

    bool addFromSig(std::string name, std::string_view idaPattern, std::string_view section = {})
    {
        return setBySignature(std::move(name), idaPattern, section);
    }

    // Try to resolve and set an offset by signature (IDA-style string)
    bool setBySignature(std::string name, std::string_view idaPattern, std::string_view section = {})
    {
        const auto parsed = hat::parse_signature(idaPattern);
        if (!parsed.has_value()) return false;

        hat::scan_result res;
        if (section.empty()) res = hat::find_pattern(parsed.value());
        else                 res = hat::find_pattern(parsed.value(), section);
        if (!res.has_result()) return false;

        set(std::move(name), static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(res.get())));
        return true;
    }

private:
    std::unordered_map<std::string, std::uint32_t> offsets;
};


