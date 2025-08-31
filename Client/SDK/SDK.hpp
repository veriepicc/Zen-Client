#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>

namespace SDK
{
    enum class EntryKind { Core, Component };

    class Registry
    {
    public:
        void add(std::string_view name, EntryKind kind)
        {
            if (name.empty()) return;
            std::string key{name};
            if (seen.count(key)) return;
            seen.insert(key);
            if (kind == EntryKind::Core) cores.emplace_back(std::move(key));
            else components.emplace_back(std::move(key));
        }

        const std::vector<std::string>& core() const { return cores; }
        const std::vector<std::string>& component() const { return components; }

    private:
        std::unordered_set<std::string> seen;
        std::vector<std::string> cores;
        std::vector<std::string> components;
    };

    inline Registry& registry()
    {
        static Registry r;
        return r;
    }

    inline void RegisterCore(std::string_view name)
    {
        registry().add(name, EntryKind::Core);
    }

    // Helper for one-line static registration in a unit
    inline bool RegisterCoreOnce(const char* name)
    {
        RegisterCore(name);
        return true;
    }

    inline void RegisterComponent(std::string_view name)
    {
        registry().add(name, EntryKind::Component);
    }

    inline const std::vector<std::string>& AllCore()
    {
        return registry().core();
    }

    inline const std::vector<std::string>& AllComponents()
    {
        return registry().component();
    }

    // Registration helpers for clean one-line use in SDK units
    struct CoreRegistration
    {
        explicit CoreRegistration(const char* name) { RegisterCore(name); }
    };
    struct ComponentRegistration
    {
        explicit ComponentRegistration(const char* name) { RegisterComponent(name); }
    };
}

// Intentionally no macro across modules; use one-line static bool below in each unit
