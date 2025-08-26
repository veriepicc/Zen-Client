module;
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <span>
#include <unordered_map>
#include <mutex>

#include <libhat/libhat.hpp>

export module SigManager;

export namespace Sig
{
    // Runtime-parse an IDA-style signature string and scan current module or a given section
    inline std::optional<std::uintptr_t> find(std::string_view idaPattern,
                                              std::string_view section = {})
    {
        const auto parsed = hat::parse_signature(idaPattern);
        if (!parsed.has_value()) return std::nullopt;

        hat::scan_result res = section.empty()
            ? hat::find_pattern(hat::signature_view{ parsed.value() })
            : hat::find_pattern(hat::signature_view{ parsed.value() }, section);
        if (res.has_result()) return reinterpret_cast<std::uintptr_t>(res.get());
        return std::nullopt;
    }

    // Compile-time signature helper: usage -> auto addr = Sig::findCT<"48 8B ?? ??">();
    template<hat::fixed_string SigStr>
    inline std::optional<std::uintptr_t> findCT()
    {
        constexpr auto fixed = hat::compile_signature<SigStr>();
        auto res = hat::find_pattern(std::span{fixed});
        if (res.has_result()) return reinterpret_cast<std::uintptr_t>(res.get());
        return std::nullopt;
    }
}

export namespace Sigs
{
    struct Entry
    {
        std::string name;
        std::string pattern;
        std::string section; // optional; empty means whole module
        std::uintptr_t address { 0 };
        bool resolved { false };
    };

    void add(std::string name, std::string pattern, std::string section = {});
    std::optional<std::uintptr_t> get(std::string_view name);
    bool resolveOne(std::string_view name);
    std::size_t resolveAll();
    void clear();
}

namespace Sigs
{
    namespace detail
    {
        static std::mutex mtx;
        static std::unordered_map<std::string, Entry> entries;
    }

    inline void add(std::string name, std::string pattern, std::string section)
    {
        std::lock_guard<std::mutex> lock(detail::mtx);
        Entry e{ name, std::move(pattern), std::move(section), 0, false };
        detail::entries.emplace(e.name, std::move(e));
    }

    inline std::optional<std::uintptr_t> get(std::string_view name)
    {
        std::lock_guard<std::mutex> lock(detail::mtx);
        if (auto it = detail::entries.find(std::string(name)); it != detail::entries.end())
        {
            if (it->second.resolved) return it->second.address;
        }
        return std::nullopt;
    }

    inline bool resolveOne(std::string_view name)
    {
        std::lock_guard<std::mutex> lock(detail::mtx);
        auto it = detail::entries.find(std::string(name));
        if (it == detail::entries.end()) return false;
        const auto parsed = hat::parse_signature(it->second.pattern);
        if (!parsed.has_value()) return false;
        hat::scan_result res = it->second.section.empty()
            ? hat::find_pattern(hat::signature_view{ parsed.value() })
            : hat::find_pattern(hat::signature_view{ parsed.value() }, it->second.section);
        if (!res.has_result()) return false;
        it->second.address = reinterpret_cast<std::uintptr_t>(res.get());
        it->second.resolved = true;
        return true;
    }

    inline std::size_t resolveAll()
    {
        std::size_t count = 0;
        std::lock_guard<std::mutex> lock(detail::mtx);
        for (auto& [_, e] : detail::entries)
        {
            const auto parsed = hat::parse_signature(e.pattern);
            if (!parsed.has_value()) continue;
            hat::scan_result res = e.section.empty()
                ? hat::find_pattern(hat::signature_view{ parsed.value() })
                : hat::find_pattern(hat::signature_view{ parsed.value() }, e.section);
            if (!res.has_result()) continue;
            e.address = reinterpret_cast<std::uintptr_t>(res.get());
            e.resolved = true;
            ++count;
        }
        return count;
    }

    inline void clear()
    {
        std::lock_guard<std::mutex> lock(detail::mtx);
        detail::entries.clear();
    }
}

