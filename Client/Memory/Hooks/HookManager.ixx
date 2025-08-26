module;
#include <cstdint>
#include <vector>
#include <string>
#include <utility>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <minhook/MinHook.h>

export module HookManager;

export class HookManager
{
public:
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
            return false;
        hooks.emplace_back(target);
        return true;
    }

    bool enableAll()
    {
        return MH_EnableHook(MH_ALL_HOOKS) == MH_OK;
    }

    bool disableAll()
    {
        return MH_DisableHook(MH_ALL_HOOKS) == MH_OK;
    }

    bool removeAll()
    {
        bool ok = true;
        for (auto* h : hooks)
            ok = ok && (MH_RemoveHook(h) == MH_OK);
        hooks.clear();
        return ok;
    }

    // Convenience: create + enable in one call
    template <typename T>
    bool hook(void* target, T* detour, T** original)
    {
        return create(target, detour, original) && MH_EnableHook(target) == MH_OK;
    }

private:
    std::vector<void*> hooks;
};


