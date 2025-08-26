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

import Memory; // for Memory::ScanSignature and MakeSigInitializer

export enum class SigType { Sig, ReferenceSig };

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

    // Signatures (declare without macros; implementation delegated to Memory)
public:
    static inline void* ContainerScreenController__onContainerSlotHovered;

private:
    static inline std::function<void()> ContainerScreenController__onContainerSlotHovered_reg = (
        initializers_.emplace_back(
            Memory::MakeSigInitializer(
                "ContainerScreenController__onContainerSlotHovered",
                hat::compile_signature<"48 89 ? ? ? 48 89 ? ? ? 41 ? 41 ? 41 ? 48 83 EC ? 45 33 ? 45 8B">(),
                0,
                &ContainerScreenController__onContainerSlotHovered,
                sigs_,
                ".text"
            )
        ),
        std::function<void()>()
    );
};

