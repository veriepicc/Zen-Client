module;
#include <cstdint>
#include <vector>
#include <string>
#include <utility>
#include <memory>
#include <iostream>
#include <mutex>
#include <unordered_set>
#include <type_traits>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define JONATHAN_CFG_FREEZE_THREADS 0
#include <Jonathan.hpp>

export module HookManager;
import SigManager;

export class HookManager
{
public:
    struct FuncHook { virtual void Initialize() = 0; virtual ~FuncHook() = default; };

    // Input hooks are implemented as self-registering modules; no auto-install here

    HookManager()
    {
        ensureInitialized();
    }

    ~HookManager()
    {
        std::scoped_lock lock(mutex);
        if (!initialized) return;
        disableAll();
        removeAll();
        (void)Jonathan::shutdown();
        initialized = false;
    }

    template <typename T>
    bool create(void* target, T detour, T* original)
    {
        static_assert(std::is_pointer_v<T> && std::is_function_v<std::remove_pointer_t<T>>, "T must be a function pointer type");
        std::scoped_lock lock(mutex);
        if (!ensureInitialized()) return false;

        if (targets.count(target) != 0)
        {
            std::cout << "[Hook] Create skipped (already created) target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(target) << std::dec << std::endl;
            return true;
        }

        const auto st_create = Jonathan::create_hook(target, reinterpret_cast<void*>(detour), reinterpret_cast<void**>(original));
        if (st_create != Jonathan::status::ok)
        {
            std::cout << "[Hook] CreateHook failed target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(target) << std::dec
                      << " reason=" << Jonathan::status_to_string(st_create) << std::endl;
            return false;
        }
        targets.insert(target);
        std::cout << "[Hook] Created target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(target) << std::dec << std::endl;
        return true;
    }

    // Raw variant used by C-style API where function type is erased
    bool createRaw(void* target, void* detour, void** original)
    {
        std::scoped_lock lock(mutex);
        if (!ensureInitialized()) return false;
        if (targets.count(target) != 0)
        {
            std::cout << "[Hook] Create skipped (already created) target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(target) << std::dec << std::endl;
            return true;
        }
        const auto st_create = Jonathan::create_hook(target, detour, original);
        if (st_create != Jonathan::status::ok)
        {
            std::cout << "[Hook] CreateHook failed target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(target) << std::dec
                      << " reason=" << Jonathan::status_to_string(st_create) << std::endl;
            return false;
        }
        targets.insert(target);
        std::cout << "[Hook] Created target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(target) << std::dec << std::endl;
        return true;
    }

    bool enableAll()
    {
        std::scoped_lock lock(mutex);
        if (!ensureInitialized()) return false;
        const auto st = Jonathan::enable_hook(nullptr);
        const bool ok = (st == Jonathan::status::ok);
        std::cout << "[Hook] EnableAll " << (ok ? "ok" : "failed")
                  << (ok ? "" : std::string(" reason=") + Jonathan::status_to_string(st)) << std::endl;
        return ok;
    }

    bool disableAll()
    {
        std::scoped_lock lock(mutex);
        if (!initialized) return true;
        const bool ok = (Jonathan::disable_hook(nullptr) == Jonathan::status::ok);
        std::cout << "[Hook] DisableAll " << (ok ? "ok" : "failed") << std::endl;
        return ok;
    }

    bool enable(void* target)
    {
        std::scoped_lock lock(mutex);
        if (!ensureInitialized()) return false;
        const auto st = Jonathan::enable_hook(target);
        const bool ok = (st == Jonathan::status::ok);
        std::cout << "[Hook] Enable target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(target) << std::dec
                  << (ok ? " ok" : std::string(" failed reason=") + Jonathan::status_to_string(st)) << std::endl;
        return ok;
    }

    bool disable(void* target)
    {
        std::scoped_lock lock(mutex);
        if (!initialized) return true;
        const bool ok = (Jonathan::disable_hook(target) == Jonathan::status::ok);
        std::cout << "[Hook] Disable target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(target) << std::dec << (ok ? " ok" : " failed") << std::endl;
        return ok;
    }

    bool remove(void* target)
    {
        std::scoped_lock lock(mutex);
        if (!initialized) return true;
        const bool ok = (Jonathan::remove_hook(target) == Jonathan::status::ok);
        if (ok) targets.erase(target);
        std::cout << "[Hook] Remove target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(target) << std::dec << (ok ? " ok" : " failed") << std::endl;
        return ok;
    }

    bool removeAll()
    {
        std::scoped_lock lock(mutex);
        if (!initialized) return true;
        bool ok = true;
        for (auto* h : targets)
            ok = ok && (Jonathan::remove_hook(h) == Jonathan::status::ok);
        targets.clear();
        std::cout << "[Hook] RemoveAll " << (ok ? "ok" : "failed") << std::endl;
        return ok;
    }

    // Convenience: create + enable in one call
    template <typename T>
    bool hook(void* target, T detour, T* original)
    {
        const bool created = create(target, detour, original);
        const bool enabled = created && enable(target);
        std::cout << "[Hook] Hook target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(target) << std::dec
                  << " created=" << (created ? "1" : "0") << " enabled=" << (enabled ? "1" : "0") << std::endl;
        return created && enabled;
    }

    void registerHook(std::unique_ptr<FuncHook> funcHook)
    {
        ownedHooks.emplace_back(std::move(funcHook));
        std::cout << "[Hook] Registered hook object" << std::endl;
    }

    void activateHooks()
    {
        for (const auto& hookObj : ownedHooks)
        {
            std::cout << "[Hook] Initializing hook object" << std::endl;
            hookObj->Initialize();
        }
        // Self-registering hooks will install themselves via HookRegistry
    }

    std::size_t ownedCount() const { return ownedHooks.size(); }
    std::size_t targetCount() const { return targets.size(); }

    void logStatus() const
    {
        std::cout << "[Hook] status: ownedHooks=" << ownedHooks.size()
                  << " targets=" << targets.size() << std::endl;
    }

private:
    bool ensureInitialized()
    {
        if (initialized) return true;
        if (Jonathan::init() == Jonathan::status::ok)
        {
            initialized = true;
            std::cout << "[Hook] Jonathan initialized" << std::endl;
        }
        else
        {
            std::cout << "[Hook] Jonathan initialization failed" << std::endl;
        }
        return initialized;
    }


    std::vector<std::unique_ptr<FuncHook>> ownedHooks;
    std::unordered_set<void*> targets;
    bool initialized { false };
    mutable std::mutex mutex;
};

export inline HookManager& GetHookManager()
{
    static HookManager instance;
    return instance;
}

namespace HookRegistryInternal
{
    inline std::vector<bool (*)()>& List()
    {
        static std::vector<bool (*)()> fns;
        return fns;
    }
}

export namespace HookRegistry
{
    using RegisterFn = bool (*)();

    struct Registration
    {
        explicit Registration(RegisterFn fn)
        {
            HookRegistryInternal::List().push_back(fn);
        }
    };

    inline bool InitializeAll()
    {
        bool ok = true;
        for (auto fn : HookRegistryInternal::List())
            ok = fn() && ok;
        return ok;
    }

    inline void Clear()
    {
        HookRegistryInternal::List().clear();
    }
}

export struct HookInfo { void** original; void* detour; void* target; };

namespace HookInternal
{
    static std::vector<HookInfo> g_hooks;
}

export namespace HookManagerAPI
{
    inline void Register(void** original, void* detour, void* target)
    {
        HookInternal::g_hooks.push_back(HookInfo{ original, detour, target });
        std::cout << "[HookAPI] Registered target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(target) << std::dec << std::endl;
    }

    inline bool InitializeHooks()
    {
        auto& hm = GetHookManager();
        bool ok = true;
        for (const auto& h : HookInternal::g_hooks)
        {
            ok = ok && hm.createRaw(h.target, h.detour, h.original);
            ok = ok && hm.enable(h.target);
        }
        return ok;
    }

    inline void ShutdownHooks()
    {
        auto& hm = GetHookManager();
        for (const auto& h : HookInternal::g_hooks)
        {
            hm.disable(h.target);
            hm.remove(h.target);
        }
        HookInternal::g_hooks.clear();
        std::cout << "[HookAPI] Shutdown complete" << std::endl;
    }
}


