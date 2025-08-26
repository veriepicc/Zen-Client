module;
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <optional>

export module OffsetManager;

export class OffsetManager
{
public:
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

private:
    std::unordered_map<std::string, std::uint32_t> offsets;
};


