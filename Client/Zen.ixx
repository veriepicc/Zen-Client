module;
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <chrono>

export module Zen;

import ModuleManager;
import HookManager;
import SigManager;
import OffsetManager;
import Memory;
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

        // Timed initialization for sigs and offsets
        auto tStart = std::chrono::high_resolution_clock::now();

        // Initialize signature manager (one-liners)
        try {
            SigManager::initialize();
            std::cout << "[Zen] SigManager initialized" << std::endl;

            const auto& sigs = SigManager::getSigs();
            std::cout << "[Zen] Sigs count: " << sigs.size() << std::endl;
            for (const auto& kv : sigs)
            {
                std::cout << "  - " << kv.first << " = 0x" << std::hex << kv.second << std::dec << std::endl;
            }

        } catch (...) {
            std::cout << "[Zen] SigManager init failed" << std::endl;
        }

        auto tMid = std::chrono::high_resolution_clock::now();

        try {
            OffsetManager::initialize();
            std::cout << "[Zen] OffsetManager initialized" << std::endl;
            const auto& offsets = OffsetManager::getOffsets();
            std::cout << "[Zen] Offsets count: " << offsets.size() << std::endl;
        } catch (...) {
            std::cout << "[Zen] OffsetManager init failed" << std::endl;
        }

       

        auto tEnd = std::chrono::high_resolution_clock::now();
        auto sigMs = std::chrono::duration_cast<std::chrono::milliseconds>(tMid - tStart).count();
        auto offMs = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tMid).count();
        auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart).count();

        std::cout << "[Zen] init times: sigs=" << sigMs << "ms, offsets=" << offMs << "ms, total=" << totalMs << "ms" << std::endl;

        // Activate any registered hooks
        auto& hm = GetHookManager();
        hm.enableAll();

        // Force-create modules so constructors run
        (void)Modules::All();
    }
}


