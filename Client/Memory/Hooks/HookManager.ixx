module;
#include <cstdint>
#include <vector>
#include <string>
#include <utility>
#include <memory>
#include <iostream>

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
        MH_Initialize();
    }

    ~HookManager()
    {
        disableAll();
        MH_Uninitialize();
    }

    template <typename T>
    bool create(void* target, T* detour, T** original)
    {
        if (MH_CreateHook(target, reinterpret_cast<LPVOID>(detour), reinterpret_cast<LPVOID*>(original)) != MH_OK)
        {
            std::cout << "[Hook] CreateHook failed target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(target) << std::dec << std::endl;
            return false;
        }
        targets.emplace_back(target);
        std::cout << "[Hook] Created target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(target) << std::dec << std::endl;
        return true;
    }

    bool enableAll()
    {
        const bool ok = (MH_EnableHook(MH_ALL_HOOKS) == MH_OK);
        std::cout << "[Hook] EnableAll " << (ok ? "ok" : "failed") << std::endl;
        return ok;
    }

    bool disableAll()
    {
        const bool ok = (MH_DisableHook(MH_ALL_HOOKS) == MH_OK);
        std::cout << "[Hook] DisableAll " << (ok ? "ok" : "failed") << std::endl;
        return ok;
    }

    bool removeAll()
    {
        bool ok = true;
        for (auto* h : targets)
            ok = ok && (MH_RemoveHook(h) == MH_OK);
        targets.clear();
        std::cout << "[Hook] RemoveAll " << (ok ? "ok" : "failed") << std::endl;
        return ok;
    }

    // Convenience: create + enable in one call
    template <typename T>
    bool hook(void* target, T* detour, T** original)
    {
        const bool created = create(target, detour, original);
        const bool enabled = created && (MH_EnableHook(target) == MH_OK);
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
    std::vector<std::unique_ptr<FuncHook>> ownedHooks;
    std::vector<void*> targets;
};

// Global accessor for convenience
export inline HookManager& GetHookManager()
{
    static HookManager instance;
    return instance;
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
        if (MH_Initialize() != MH_OK) return false;
        for (const auto& h : HookInternal::g_hooks)
        {
            if (MH_CreateHook(h.target, h.detour, h.original) != MH_OK)
            {
                std::cout << "[HookAPI] CreateHook failed target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(h.target) << std::dec << std::endl;
                return false;
            }
            if (MH_EnableHook(h.target) != MH_OK)
            {
                std::cout << "[HookAPI] EnableHook failed target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(h.target) << std::dec << std::endl;
                return false;
            }
            std::cout << "[HookAPI] Hooked target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(h.target) << std::dec << std::endl;
        }
        return true;
    }

    inline void ShutdownHooks()
    {
        for (const auto& h : HookInternal::g_hooks)
        {
            MH_DisableHook(h.target);
            MH_RemoveHook(h.target);
        }
        MH_Uninitialize();
        HookInternal::g_hooks.clear();
        std::cout << "[HookAPI] Shutdown complete" << std::endl;
    }
}


