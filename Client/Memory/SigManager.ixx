module;
#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <span>
#include <unordered_map>
#include <functional>

#include <libhat/libhat.hpp>

export module SigManager;

import Memory;

export enum class SigType { Sig, ReferenceSig };

#define REGISTER_SIG(Name, Pattern, Type, Offset) \
public: \
static inline void* Name; \
private: \
static inline std::function<void()> Name##Reg = ( \
    initializers.emplace_back( \
        Memory::MakeSigInitializer(#Name, hat::compile_signature<Pattern>(), Offset, &Name, sigs, ".text") \
    ), \
    std::function<void()>() \
); \
public:

export class SigManager {
private:
    static inline std::vector<std::function<void()>> initializers;
    static inline std::unordered_map<std::string, std::uintptr_t> sigs;

public:
    static void initialize()
    {
        for (const auto& init : initializers) init();
    }

    static void deinitialize()
    {
        initializers.clear();
        sigs.clear();
    }

    static const std::unordered_map<std::string, std::uintptr_t>& getSigs()
    {
        return sigs;
    }

    REGISTER_SIG(ContainerScreenController_onContainerSlotHovered,
                 "48 89 ? ? ? 48 89 ? ? ? 41 ? 41 ? 41 ? 48 83 EC ? 45 33 ? 45 8B",
                 SigType::Sig,
                 0)
        REGISTER_SIG(Setupandrender,
            "48 8B C4 48 89 ?? ?? 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 0F 29 70 ?? 0F 29 78 ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 85 ?? ?? ?? ?? 4C 8B FA 48 89 55",
            SigType::Sig,
            0)
        REGISTER_SIG(Gamemode,
            "48 8B 89 ? ? ? ? 48 8B 01 48 8B 40 18 FF 15 ? ? ? ? 8B 43 24 89 43 20 48 83 C4 20 5B",
            SigType::Sig,
            0)
};

