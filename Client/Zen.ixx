module;
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <chrono>

export module Zen;

import HookManager;
import SigManager;
import SetupAndRender;
import Module;

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
    inline void Initialize()
    {
        Detail::AllocateAndBindConsole();
        SetConsoleTitleW(L"Zen Client");
        std::cout.setf(std::ios::unitbuf);
        std::cerr.setf(std::ios::unitbuf);
        std::cout << "Zen Client has been initialized!" << std::endl;

        SigManager::initialize();
        {
            const auto& sigs = SigManager::getSigs();
            std::size_t resolved = 0, missing = 0;
            for (const auto& kv : sigs) {
                if (kv.second != 0) ++resolved; else ++missing;
            }
            std::cout << "[Zen] Signatures: total=" << sigs.size()
                      << " resolved=" << resolved
                      << " missing=" << missing << std::endl;
            SigManager::logStatus();
            if (missing > 0) {
                std::cout << "[Zen] Missing signatures:" << std::endl;
                for (const auto& kv : sigs) {
                    if (kv.second == 0) std::cout << "  - " << kv.first << std::endl;
                }
            }
        }

        HookRegistry::InitializeAll();
        auto& hm = GetHookManager();
        hm.enableAll();
    }
}
