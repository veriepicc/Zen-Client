module;
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <filesystem>

export module Zen;

import HookManager;
import SigManager;
import Module;
import SoundPlayer;
import Utils;

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
        freopen_s(&fptr, "CONIN$", "r", stdin);
    }

    inline void FreeAndUnbindConsole()
    {
        FILE* fptr = nullptr;
        freopen_s(&fptr, "NUL", "w", stdout);
        freopen_s(&fptr, "NUL", "w", stderr);
        freopen_s(&fptr, "NUL", "r", stdin);
        FreeConsole();
    }
}

export namespace Zen
{
    HMODULE Module;

    namespace Detail
    {
        inline DWORD WINAPI EjectWorker(LPVOID)
        {
            Modules::DisableAll();

            auto& hm = GetHookManager();
            hm.disableAll();
            Sleep(100);
            hm.removeAll();

            GetSoundPlayer().shutdown();
            SigManager::deinitialize();
            Detail::FreeAndUnbindConsole();
            ExitThread(0);
            return 0;
        }
    }

    inline void Initialize(HMODULE hModule)
    {
        Module = hModule;
        Detail::AllocateAndBindConsole();
        SetConsoleTitleW(L"Zen Client");
        std::cout.setf(std::ios::unitbuf);
        std::cerr.setf(std::ios::unitbuf);
        std::cout << "Zen Client has been initialized!" << std::endl;
        auto tStart = std::chrono::high_resolution_clock::now();
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
        auto tAfterSigs = std::chrono::high_resolution_clock::now();

        HookRegistry::InitializeAll();
        auto tAfterHooks = std::chrono::high_resolution_clock::now();
        auto& hm = GetHookManager();
        hm.enableAll();
        GetSoundPlayer().initialize();

        auto tAfterEnable = std::chrono::high_resolution_clock::now();

        auto sigMs = std::chrono::duration_cast<std::chrono::milliseconds>(tAfterSigs - tStart).count();
        auto hooksMs = std::chrono::duration_cast<std::chrono::milliseconds>(tAfterHooks - tAfterSigs).count();
        auto enableMs = std::chrono::duration_cast<std::chrono::milliseconds>(tAfterEnable - tAfterHooks).count();
        auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(tAfterEnable - tStart).count();
        std::cout << "[Zen] init times: sigs=" << sigMs
            << "ms, hooks=" << hooksMs
            << "ms, enable=" << enableMs
            << "ms, total=" << totalMs << "ms" << std::endl;
    }

    inline void Eject()
    {
        HANDLE h = CreateThread(nullptr, 0, Detail::EjectWorker, nullptr, 0, nullptr);
        if (h) CloseHandle(h);
    }
}
