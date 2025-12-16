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
#include <cstring>
#include <thread>
#include <array>

export module Memory;

import Zenova;

// Internal-only implementation details (not exported)
namespace Memory::detail
{
    enum class SignatureKind { Direct, Reference };

    struct QueuedSignatureRequest
    {
        std::string name;
        std::vector<Zenova::SigByte> pattern;
        std::size_t firstNonWildcard;
        int relativeOffset;
        void** outPtr;
        SignatureKind kind;
        std::string section;
    };

    inline std::vector<QueuedSignatureRequest> queuedSignatureRequests;

    inline void enqueueSignature(std::string name,
                                 std::vector<Zenova::SigByte> pattern,
                                 std::size_t firstNonWildcard,
                                 int relativeOffset,
                                 void** outPtr,
                                 SignatureKind kind,
                                 std::string section)
    {
        queuedSignatureRequests.push_back(QueuedSignatureRequest{
            std::move(name),
            std::move(pattern),
            firstNonWildcard,
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

    inline HookRegistry& Hooks()
    {
        static HookRegistry instance;
        return instance;
    }

    inline bool RegisterHook(void** original, void* detour, void* target)
    {
        return Hooks().registerHook(original, detour, target);
    }

    inline bool InitializeHooking()
    {
        return Hooks().applyAll();
    }

    inline void ShutdownHooking()
    {
        Hooks().shutdown();
    }

    // Scan signature using Zenova (single scan, not batched)
    template<std::size_t N>
    inline std::uintptr_t ScanSignature(const Zenova::Signature<N>& sig,
                                        int relativeOffset = 0,
                                        std::string_view section = ".text")
    {
        const auto mod = Zenova::process::getProcessModule();
        if (!mod.valid()) return 0;

        auto sec = Zenova::process::getSection(mod, section);
        if (!sec.valid()) return 0;

        auto res = Zenova::scan(sec.data(), sig);
        if (!res.valid()) return 0;

        return relativeOffset == 0
            ? res.addr()
            : reinterpret_cast<std::uintptr_t>(res.rel(static_cast<size_t>(relativeOffset)));
    }

    inline void EnqueueSignature(std::string name,
                                 std::vector<Zenova::SigByte> pattern,
                                 std::size_t firstNonWildcard,
                                 int relativeOffset,
                                 void** outPtr,
                                 SignatureKind kind,
                                 std::string section)
    {
        detail::enqueueSignature(std::move(name), std::move(pattern), firstNonWildcard, relativeOffset, outPtr, kind, std::move(section));
    }

    // Process all queued signature requests with parallel AVX2 scanning + rarity-based anchors 🔥
    inline void ResolveSignatureQueue(std::unordered_map<std::string, std::uintptr_t>& store)
    {
        if (detail::queuedSignatureRequests.empty()) return;

        // Cache sections
        std::unordered_map<std::string, Zenova::process::Section> secCache;
        const auto mod = Zenova::process::getProcessModule();
        
        for (const auto& q : detail::queuedSignatureRequests)
        {
            if (secCache.find(q.section) == secCache.end())
            {
                secCache.emplace(q.section, Zenova::process::getSection(mod, q.section));
            }
        }

        const std::size_t count = detail::queuedSignatureRequests.size();
        std::vector<std::size_t> indices(count);
        for (std::size_t i = 0; i < count; ++i) indices[i] = i;
        std::vector<std::uintptr_t> results(count, 0);

        // Parallel signature scanning - each signature uses AVX2 with rarity-based anchor!
        std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&secCache, &results](std::size_t i)
        {
            const auto& q = detail::queuedSignatureRequests[i];
            const auto it = secCache.find(q.section);
            if (it == secCache.end()) return;
            const auto& sec = it->second;
            if (!sec.valid()) return;

            const std::size_t sigLen = q.pattern.size();
            if (sigLen == 0) return;

            // Use Zenova's AVX2 scanner with rarity-based anchor selection! 🔥
            const std::byte* found = Zenova::scanRuntime(
                sec.data(),
                std::span<const Zenova::SigByte>{q.pattern}
            );

            if (!found) {
                results[i] = 0;
                return;
            }

            if (q.kind == SignatureKind::Direct)
            {
                if (q.relativeOffset == 0)
                {
                    results[i] = reinterpret_cast<std::uintptr_t>(found);
                }
                else
                {
                    // Resolve RIP-relative
                    const auto rip = found + q.relativeOffset + 4;
                    std::int32_t rel32;
                    std::memcpy(&rel32, found + q.relativeOffset, sizeof(rel32));
                    results[i] = reinterpret_cast<std::uintptr_t>(rip + rel32);
                }
            }
            else
            {
                const auto base = reinterpret_cast<std::uintptr_t>(found);
                const auto ptrLocation = base + static_cast<std::uintptr_t>(q.relativeOffset);
                results[i] = *reinterpret_cast<const std::uintptr_t*>(ptrLocation);
            }
        });

        // Store results
        for (std::size_t i = 0; i < count; ++i)
        {
            const auto& q = detail::queuedSignatureRequests[i];
            const auto addr = results[i];
            *q.outPtr = addr ? reinterpret_cast<void*>(addr) : nullptr;
            store[q.name] = addr;
        }

        detail::queuedSignatureRequests.clear();
    }

    // Create initializer from Zenova signature (compile-time)
    template <std::size_t N>
    inline std::function<void()> MakeSigInitializer(const char* name,
                                                    const Zenova::Signature<N>& sig,
                                                    int relativeOffset,
                                                    void** outPtr,
                                                    std::unordered_map<std::string, std::uintptr_t>& store,
                                                    std::string_view section = ".text")
    {
        std::string nameCopy(name);
        std::vector<Zenova::SigByte> pattern(sig.begin(), sig.end());
        std::size_t firstNonWildcard = sig.firstNonWildcard;
        return [nameCopy = std::move(nameCopy), pattern = std::move(pattern), firstNonWildcard, relativeOffset, outPtr, &store, section]() mutable {
            (void)store;
            EnqueueSignature(std::move(nameCopy), std::move(pattern), firstNonWildcard, relativeOffset, outPtr, SignatureKind::Direct, std::string(section));
        };
    }

    // Reference signature initializer (compile-time)
    template <std::size_t N>
    inline std::function<void()> MakeRefSigInitializer(const char* name,
                                                       const Zenova::Signature<N>& sig,
                                                       int relativeOffset,
                                                       void** outPtr,
                                                       std::unordered_map<std::string, std::uintptr_t>& store,
                                                       std::string_view section = ".text")
    {
        std::string nameCopy(name);
        std::vector<Zenova::SigByte> pattern(sig.begin(), sig.end());
        std::size_t firstNonWildcard = sig.firstNonWildcard;
        return [nameCopy = std::move(nameCopy), pattern = std::move(pattern), firstNonWildcard, relativeOffset, outPtr, &store, section]() mutable {
            (void)store;
            EnqueueSignature(std::move(nameCopy), std::move(pattern), firstNonWildcard, relativeOffset, outPtr, SignatureKind::Reference, std::string(section));
        };
    }

    // Member access helper (replaces hat::member_at)
    template<typename MemberType, typename Base>
    inline auto& MemberAt(Base* ptr, std::size_t offset)
    {
        return *reinterpret_cast<MemberType*>(reinterpret_cast<std::byte*>(ptr) + offset);
    }
}
