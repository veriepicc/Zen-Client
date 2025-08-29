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

#include <minhook/MinHook.h>

export module HookManager;

export class HookManager
{
public:
    struct FuncHook { virtual void Initialize() = 0; virtual ~FuncHook() = default; };

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
        MH_Uninitialize();
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

        const auto status = MH_CreateHook(target, reinterpret_cast<LPVOID>(detour), reinterpret_cast<LPVOID*>(original));
        if (status != MH_OK)
        {
            std::cout << "[Hook] CreateHook failed target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(target) << std::dec << std::endl;
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
        const auto status = MH_CreateHook(target, detour, original);
        if (status != MH_OK)
        {
            std::cout << "[Hook] CreateHook failed target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(target) << std::dec << std::endl;
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
        const bool ok = (MH_EnableHook(MH_ALL_HOOKS) == MH_OK);
        std::cout << "[Hook] EnableAll " << (ok ? "ok" : "failed") << std::endl;
        return ok;
    }

    bool disableAll()
    {
        std::scoped_lock lock(mutex);
        if (!initialized) return true;
        const bool ok = (MH_DisableHook(MH_ALL_HOOKS) == MH_OK);
        std::cout << "[Hook] DisableAll " << (ok ? "ok" : "failed") << std::endl;
        return ok;
    }

    bool enable(void* target)
    {
        std::scoped_lock lock(mutex);
        if (!ensureInitialized()) return false;
        const bool ok = (MH_EnableHook(target) == MH_OK);
        std::cout << "[Hook] Enable target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(target) << std::dec << (ok ? " ok" : " failed") << std::endl;
        return ok;
    }

    bool disable(void* target)
    {
        std::scoped_lock lock(mutex);
        if (!initialized) return true;
        const bool ok = (MH_DisableHook(target) == MH_OK);
        std::cout << "[Hook] Disable target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(target) << std::dec << (ok ? " ok" : " failed") << std::endl;
        return ok;
    }

    bool remove(void* target)
    {
        std::scoped_lock lock(mutex);
        if (!initialized) return true;
        const bool ok = (MH_RemoveHook(target) == MH_OK);
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
            ok = ok && (MH_RemoveHook(h) == MH_OK);
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
        if (MH_Initialize() == MH_OK)
        {
            initialized = true;
            std::cout << "[Hook] MinHook initialized" << std::endl;
        }
        else
        {
            std::cout << "[Hook] MinHook initialization failed" << std::endl;
        }
        return initialized;
    }

    std::vector<std::unique_ptr<FuncHook>> ownedHooks;
    std::unordered_set<void*> targets;
    bool initialized { false };
    mutable std::mutex mutex;
};

// Global accessor for convenience
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

// Simple C-style registry API similar to the reference
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


