module;
#include <windows.h>

#include <minhook/MinHook.h>
#include <cstdint>
#include <utility>
#include <vector>
#include <iostream>
#include <string_view>
#include <string>
#include <functional>
#include <unordered_map>

#include <libhat/libhat.hpp>

export module Memory;

export namespace Memory
{
    template <typename R, typename... Args>
    R CallFunc(void* func, Args... args)
    {
        return (reinterpret_cast<R(*)(Args...)>(func))(args...);
    }

    template <typename R, typename... Args>
    R CallVFunc(std::uint32_t index, void* thisPtr, Args... args)
    {
        auto vtable = *reinterpret_cast<void***>(thisPtr);
        auto fn = reinterpret_cast<R(*)(void*, Args...)>(vtable[index]);
        return fn(thisPtr, std::forward<Args>(args)...);
    }

    struct HookRecord
    {
        void** original;
        void* detour;
        void* target;
    };

    class HookRegistry
    {
    public:
        bool initialize()
        {
            if (initialized) return true;
            initialized = (MH_Initialize() == MH_OK);
            return initialized;
        }

        bool registerHook(void** original, void* detour, void* target)
        {
            hooks.emplace_back(HookRecord{ original, detour, target });
            return true;
        }

        bool applyAll()
        {
            if (!initialize()) return false;
            for (const auto& h : hooks)
            {
                if (MH_CreateHook(h.target, h.detour, h.original) != MH_OK)
                {
                    std::cout << "[Memory] CreateHook failed target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(h.target) << std::dec << std::endl;
                    return false;
                }
                if (MH_EnableHook(h.target) != MH_OK)
                {
                    std::cout << "[Memory] EnableHook failed target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(h.target) << std::dec << std::endl;
                    return false;
                }
            }
            return true;
        }

        void shutdown()
        {
            for (const auto& h : hooks)
            {
                MH_DisableHook(h.target);
                MH_RemoveHook(h.target);
            }
            hooks.clear();
            if (initialized) MH_Uninitialize();
            initialized = false;
        }

        std::size_t count() const { return hooks.size(); }

    private:
        bool initialized { false };
        std::vector<HookRecord> hooks;
    };

    export inline HookRegistry& Hooks()
    {
        static HookRegistry instance;
        return instance;
    }

    export inline bool RegisterHook(void** original, void* detour, void* target)
    {
        return Hooks().registerHook(original, detour, target);
    }

    export inline bool InitializeHooking()
    {
        return Hooks().applyAll();
    }

    export inline void ShutdownHooking()
    {
        Hooks().shutdown();
    }

    // Signature scanning utilities (used by SigManager macros)
    export inline std::uintptr_t ScanSignature(std::span<const hat::signature_element> sig,
                                               int relativeOffset = 0,
                                               std::string_view section = ".text")
    {
        const auto mod = hat::process::get_process_module();
        auto res = hat::find_pattern(sig, section, mod);
        if (!res.has_result()) return 0;
        return relativeOffset == 0
            ? reinterpret_cast<std::uintptr_t>(res.get())
            : reinterpret_cast<std::uintptr_t>(res.rel(static_cast<size_t>(relativeOffset)));
    }

    // Factory that creates an initializer to resolve a signature and store the result
    inline std::function<void()> MakeSigInitializer(const char* name,
                                                          std::span<const hat::signature_element> sig,
                                                          int relativeOffset,
                                                          void** outPtr,
                                                          std::unordered_map<std::string, std::uintptr_t>& store,
                                                          std::string_view section = ".text")
    {
        // Copy values into the closure; store and outPtr kept by reference
        std::string nameCopy(name);
        std::vector<hat::signature_element> pattern(sig.begin(), sig.end());
        return [nameCopy = std::move(nameCopy), pattern = std::move(pattern), relativeOffset, outPtr, &store, section]() mutable {
            auto addr = ScanSignature(std::span{pattern}, relativeOffset, section);
            *outPtr = addr ? reinterpret_cast<void*>(addr) : nullptr;
            store[nameCopy] = addr;
        };
    }

    // Overload that accepts a fixed_signature (std::array) directly
    template <typename FixedSig>
    inline std::function<void()> MakeSigInitializer(const char* name,
                                                           const FixedSig& fixed,
                                                           int relativeOffset,
                                                           void** outPtr,
                                                           std::unordered_map<std::string, std::uintptr_t>& store,
                                                           std::string_view section = ".text")
    {
        std::string nameCopy(name);
        std::vector<hat::signature_element> pattern(fixed.begin(), fixed.end());
        return [nameCopy = std::move(nameCopy), pattern = std::move(pattern), relativeOffset, outPtr, &store, section]() mutable {
            auto addr = ScanSignature(std::span{pattern}, relativeOffset, section);
            *outPtr = addr ? reinterpret_cast<void*>(addr) : nullptr;
            store[nameCopy] = addr;
        };
    }

    // Convenience: safely access a member at byte offset without including libhat in each module
    template<typename MemberType, typename Base>
    inline auto& MemberAt(Base* ptr, std::size_t offset)
    {
        return hat::member_at<MemberType>(ptr, offset);
    }
}


