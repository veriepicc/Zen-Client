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
    initializers_.emplace_back( \
        Memory::MakeSigInitializer(#Name, hat::compile_signature<Pattern>(), Offset, &Name, sigs_, ".text") \
    ), \
    std::function<void()>() \
); \
public:

export class SigManager {
private:
    static inline std::vector<std::function<void()>> initializers_;
    static inline std::unordered_map<std::string, std::uintptr_t> sigs_;

public:
    static void initialize()
    {
        for (const auto& init : initializers_) init();
    }

    static void deinitialize()
    {
        initializers_.clear();
        sigs_.clear();
    }

    static const std::unordered_map<std::string, std::uintptr_t>& getSigs()
    {
        return sigs_;
    }

    REGISTER_SIG(ContainerScreenController_onContainerSlotHovered,
                 "48 89 ? ? ? 48 89 ? ? ? 41 ? 41 ? 41 ? 48 83 EC ? 45 33 ? 45 8B",
                 SigType::Sig,
                 0)
};

