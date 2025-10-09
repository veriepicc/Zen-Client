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
import Utils;

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
            return true;
        }

        const auto st_create = Jonathan::create_hook(target, reinterpret_cast<void*>(detour), reinterpret_cast<void**>(original));
        if (st_create != Jonathan::status::ok)
        {
            return false;
        }
        targets.insert(target);
        return true;
    }

    // Raw variant used by C-style API where function type is erased
    bool createRaw(void* target, void* detour, void** original)
    {
        std::scoped_lock lock(mutex);
        if (!ensureInitialized()) return false;
        if (targets.count(target) != 0)
        {
            return true;
        }
        const auto st_create = Jonathan::create_hook(target, detour, original);
        if (st_create != Jonathan::status::ok)
        {
            return false;
        }
        targets.insert(target);
        return true;
    }

    bool enableAll()
    {
        std::scoped_lock lock(mutex);
        if (!ensureInitialized()) return false;
        const auto st = Jonathan::enable_hook(nullptr);
        const bool ok = (st == Jonathan::status::ok);
        if (ok) std::cout << "[Hooks] All enabled" << std::endl;
        return ok;
    }

    bool disableAll()
    {
        std::scoped_lock lock(mutex);
        if (!initialized) return true;
        const bool ok = (Jonathan::disable_hook(nullptr) == Jonathan::status::ok);
        if (ok) std::cout << "[Hooks] All disabled" << std::endl;
        return ok;
    }

    bool enable(void* target)
    {
        std::scoped_lock lock(mutex);
        if (!ensureInitialized()) return false;
        const auto st = Jonathan::enable_hook(target);
        const bool ok = (st == Jonathan::status::ok);
        return ok;
    }

    bool disable(void* target)
    {
        std::scoped_lock lock(mutex);
        if (!initialized) return true;
        const bool ok = (Jonathan::disable_hook(target) == Jonathan::status::ok);
        return ok;
    }

    bool remove(void* target)
    {
        std::scoped_lock lock(mutex);
        if (!initialized) return true;
        const bool ok = (Jonathan::remove_hook(target) == Jonathan::status::ok);
        if (ok) targets.erase(target);
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
        if (ok) std::cout << "[Hooks] All removed" << std::endl;
        return ok;
    }

    // Convenience: create + enable in one call
    template <typename T>
    bool hook(void* target, T detour, T* original)
    {
        const bool created = create(target, detour, original);
        const bool enabled = created && enable(target);
        return created && enabled;
    }

    void registerHook(std::unique_ptr<FuncHook> funcHook)
    {
        ownedHooks.emplace_back(std::move(funcHook));
    }

    void activateHooks()
    {
        for (const auto& hookObj : ownedHooks)
        {
            hookObj->Initialize();
        }
        // Self-registering hooks will install themselves via HookRegistry
    }

    std::size_t ownedCount() const { return ownedHooks.size(); }
    std::size_t targetCount() const { return targets.size(); }

    void logStatus() const
    {
        std::cout << "[Hooks] Status: " << targets.size() << " active hooks" << std::endl;
    }

private:
    bool ensureInitialized()
    {
        if (initialized) return true;
        if (Jonathan::init() == Jonathan::status::ok)
        {
            initialized = true;
            std::cout << "[Hooks] Initialized" << std::endl;
        }
        else
        {
            std::cout << "[Hooks] Initialization failed" << std::endl;
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
        FILE* logFile = nullptr;
        std::string logPath = Utils::GetRoamingPath() + "\\zen_log.txt";
        fopen_s(&logFile, logPath.c_str(), "a");
        
        if (logFile) {
            fprintf(logFile, "[HookRegistry] Initializing %zu hooks...\n", HookRegistryInternal::List().size());
            fflush(logFile);
        }
        
        bool ok = true;
        for (auto fn : HookRegistryInternal::List())
            ok = fn() && ok;
            
        if (logFile) {
            fprintf(logFile, "[HookRegistry] Hook initialization %s\n", (ok ? "succeeded" : "failed"));
            fflush(logFile);
        }
        
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
        std::cout << "[Hooks] Shutdown complete" << std::endl;
    }
}


