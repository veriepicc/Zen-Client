module;
#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>

export module SDK;

export namespace SDK
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

    export inline void RegisterCore(std::string_view name)
    {
        registry().add(name, EntryKind::Core);
    }

    // Helper for one-line static registration in a unit
    export inline bool RegisterCoreOnce(const char* name)
    {
        RegisterCore(name);
        return true;
    }

    export inline void RegisterComponent(std::string_view name)
    {
        registry().add(name, EntryKind::Component);
    }

    export inline const std::vector<std::string>& AllCore()
    {
        return registry().core();
    }

    export inline const std::vector<std::string>& AllComponents()
    {
        return registry().component();
    }

    // Registration helpers for clean one-line use in SDK units
    export struct CoreRegistration
    {
        explicit CoreRegistration(const char* name) { RegisterCore(name); }
    };
    export struct ComponentRegistration
    {
        explicit ComponentRegistration(const char* name) { RegisterComponent(name); }
    };
}

// Intentionally no macro export across modules; use one-line static bool below in each unit
