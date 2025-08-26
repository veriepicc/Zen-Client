module;
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <iostream>
#include <iomanip>

export module Zen;

import ModuleManager;
import HookManager;
import SigManager;
// removed demo imports

namespace Zen::Detail
{
    inline void AllocateAndBindConsole()
    {
        if (!AttachConsole(ATTACH_PARENT_PROCESS))
        {
            AllocConsole();
        }

        FILE* fptr = nullptr;
        freopen_s(&fptr, "CONOUT$", "w", stdout);
        freopen_s(&fptr, "CONOUT$", "w", stderr);
        freopen_s(&fptr, "CONIN$",  "r", stdin);
    }
}

export namespace Zen
{
    inline void Console()
    {
        std::cout.setf(std::ios::unitbuf);
        std::cerr.setf(std::ios::unitbuf);
        std::cout << "Zen Client has been initialized!" << std::endl;
    }

    inline void Initialize()
    {
        Detail::AllocateAndBindConsole();
        SetConsoleTitleW(L"Zen Client");
        Console();

        // Initialize signature manager (DEFINE_SIG-based)
        try {
            SigManager::initialize();
            std::cout << "[Zen] SigManager initialized" << std::endl;

            const auto& sigs = SigManager::getSigs();
            std::cout << "[Zen] Sigs count: " << sigs.size() << std::endl;
            for (const auto& kv : sigs)
            {
                std::cout << "  - " << kv.first << " = 0x" << std::hex << kv.second << std::dec << std::endl;
            }

            std::cout << "[Zen] ContainerScreenController::_onContainerSlotHovered = 0x"
                      << std::hex << reinterpret_cast<std::uintptr_t>(SigManager::ContainerScreenController__onContainerSlotHovered)
                      << std::dec << std::endl;
        } catch (...) {
            std::cout << "[Zen] SigManager init failed" << std::endl;
        }

        // Demonstrate hook manager lifecycle (no-op until you register hooks)
        auto& hm = GetHookManager();
        hm.enableAll();
    }
}


