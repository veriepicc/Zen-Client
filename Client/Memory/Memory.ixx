module;
#include <windows.h>

#include <Jonathan.hpp>
#include <cstdint>
#include <utility>
#include <vector>
#include <iostream>
#include <string_view>
#include <string>
#include <functional>
#include <unordered_map>
#include <span>
#include <algorithm>
#include <execution>

#include <libhat/libhat.hpp>

export module Memory;

// Internal-only implementation details (not exported)
namespace Memory::detail
{
    enum class SignatureKind { Direct, Reference };

    struct QueuedSignatureRequest
    {
        std::string name;
        std::vector<hat::signature_element> pattern;
        int relativeOffset;
        void** outPtr;
        SignatureKind kind;
        std::string section;
    };

    inline std::vector<QueuedSignatureRequest> queuedSignatureRequests;

    inline void enqueueSignature(std::string name,
                                 std::vector<hat::signature_element> pattern,
                                 int relativeOffset,
                                 void** outPtr,
                                 SignatureKind kind,
                                 std::string section)
    {
        queuedSignatureRequests.push_back(QueuedSignatureRequest{
            std::move(name),
            std::move(pattern),
            relativeOffset,
            outPtr,
            kind,
            std::move(section)
        });
    }
}

export namespace Memory
{
    using detail::SignatureKind;

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
            initialized = (Jonathan::init() == Jonathan::status::ok);
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
                if (Jonathan::create_hook(h.target, h.detour, h.original) != Jonathan::status::ok)
                {
                    std::cout << "[Memory] CreateHook failed target=0x" << std::hex << reinterpret_cast<std::uintptr_t>(h.target) << std::dec << std::endl;
                    return false;
                }
                if (Jonathan::enable_hook(h.target) != Jonathan::status::ok)
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
                (void)Jonathan::disable_hook(h.target);
                (void)Jonathan::remove_hook(h.target);
            }
            hooks.clear();
            if (initialized) (void)Jonathan::shutdown();
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

    export inline std::uintptr_t ScanSignature(std::span<const hat::signature_element> sig,
                                               int relativeOffset = 0,
                                               std::string_view section = ".text")
    {
        const auto mod = hat::process::get_process_module();

        auto res = hat::find_pattern<hat::scan_alignment::X16>(sig, section, mod);
        if (!res.has_result())
            res = hat::find_pattern<hat::scan_alignment::X1>(sig, section, mod);
        if (!res.has_result()) return 0;
        return relativeOffset == 0
            ? reinterpret_cast<std::uintptr_t>(res.get())
            : reinterpret_cast<std::uintptr_t>(res.rel(static_cast<size_t>(relativeOffset)));
    }

    
    inline void EnqueueSignature(std::string name,
                                 std::vector<hat::signature_element> pattern,
                                 int relativeOffset,
                                 void** outPtr,
                                 SignatureKind kind,
                                 std::string section)
    {
        detail::enqueueSignature(std::move(name), std::move(pattern), relativeOffset, outPtr, kind, std::move(section));
    }

    
    export inline void ResolveSignatureQueue(std::unordered_map<std::string, std::uintptr_t>& store)
    {
        if (detail::queuedSignatureRequests.empty()) return;

        
        std::unordered_map<std::string, std::span<std::byte>> sectionCache;
        const auto mod = hat::process::get_process_module();
        for (const auto& q : detail::queuedSignatureRequests)
        {
            if (sectionCache.find(q.section) == sectionCache.end())
            {
                sectionCache.emplace(q.section, hat::process::get_section_data(mod, q.section));
            }
        }


        const std::size_t count = detail::queuedSignatureRequests.size();
        std::vector<std::size_t> indices(count);
        for (std::size_t i = 0; i < count; ++i) indices[i] = i;
        std::vector<std::uintptr_t> results(count, 0);

        
        std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&](std::size_t i)
        {
            const auto& q = detail::queuedSignatureRequests[i];
            const auto it = sectionCache.find(q.section);
            if (it == sectionCache.end()) return;
            const auto sec = it->second;
            if (sec.empty()) return;

            auto res = hat::find_pattern<hat::scan_alignment::X16>(
                sec.begin(),
                sec.end(),
                std::span<const hat::signature_element>{q.pattern},
                hat::scan_hint::x86_64
            );

            if (!res.has_result())
            {
           
                res = hat::find_pattern<hat::scan_alignment::X1>(
                    sec.begin(),
                    sec.end(),
                    std::span<const hat::signature_element>{q.pattern},
                    hat::scan_hint::x86_64
                );
                if (!res.has_result())
                {
                    results[i] = 0;
                    return;
                }
            }

            if (q.kind == SignatureKind::Direct)
            {
                if (q.relativeOffset == 0)
                {
                    results[i] = reinterpret_cast<std::uintptr_t>(res.get());
                }
                else
                {
                    results[i] = reinterpret_cast<std::uintptr_t>(res.rel(static_cast<size_t>(q.relativeOffset)));
                }
            }
            else
            {
                const auto base = reinterpret_cast<std::uintptr_t>(res.get());
                const auto ptrLocation = base + static_cast<std::uintptr_t>(q.relativeOffset);
                results[i] = *reinterpret_cast<const std::uintptr_t*>(ptrLocation);
            }
        });


        for (std::size_t i = 0; i < count; ++i)
        {
            const auto& q = detail::queuedSignatureRequests[i];
            const auto addr = results[i];
            *q.outPtr = addr ? reinterpret_cast<void*>(addr) : nullptr;
            store[q.name] = addr;
        }

        detail::queuedSignatureRequests.clear();
    }

    inline std::function<void()> MakeSigInitializer(const char* name,
                                                          std::span<const hat::signature_element> sig,
                                                          int relativeOffset,
                                                          void** outPtr,
                                                          std::unordered_map<std::string, std::uintptr_t>& store,
                                                          std::string_view section = ".text")
    { 
        std::string nameCopy(name);
        std::vector<hat::signature_element> pattern(sig.begin(), sig.end());
        return [nameCopy = std::move(nameCopy), pattern = std::move(pattern), relativeOffset, outPtr, &store, section]() mutable {
            (void)store; 
            EnqueueSignature(std::move(nameCopy), std::move(pattern), relativeOffset, outPtr, SignatureKind::Direct, std::string(section));
        };
    }

   
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
            (void)store; 
            EnqueueSignature(std::move(nameCopy), std::move(pattern), relativeOffset, outPtr, SignatureKind::Direct, std::string(section));
        };
    }

   
    inline std::function<void()> MakeRefSigInitializer(const char* name,
                                                       std::span<const hat::signature_element> sig,
                                                       int relativeOffset,
                                                       void** outPtr,
                                                       std::unordered_map<std::string, std::uintptr_t>& store,
                                                       std::string_view section = ".text")
    {
        std::string nameCopy(name);
        std::vector<hat::signature_element> pattern(sig.begin(), sig.end());
        return [nameCopy = std::move(nameCopy), pattern = std::move(pattern), relativeOffset, outPtr, &store, section]() mutable {
            (void)store; 
            EnqueueSignature(std::move(nameCopy), std::move(pattern), relativeOffset, outPtr, SignatureKind::Reference, std::string(section));
        };
    }

   
    template <typename FixedSig>
    inline std::function<void()> MakeRefSigInitializer(const char* name,
                                                       const FixedSig& fixed,
                                                       int relativeOffset,
                                                       void** outPtr,
                                                       std::unordered_map<std::string, std::uintptr_t>& store,
                                                       std::string_view section = ".text")
    {
        std::string nameCopy(name);
        std::vector<hat::signature_element> pattern(fixed.begin(), fixed.end());
        return [nameCopy = std::move(nameCopy), pattern = std::move(pattern), relativeOffset, outPtr, &store, section]() mutable {
            (void)store; 
            EnqueueSignature(std::move(nameCopy), std::move(pattern), relativeOffset, outPtr, SignatureKind::Reference, std::string(section));
        };
    }

    
    template<typename MemberType, typename Base>
    inline auto& MemberAt(Base* ptr, std::size_t offset)
    {
        return hat::member_at<MemberType>(ptr, offset);
    }
}


