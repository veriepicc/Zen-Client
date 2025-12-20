module;
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>
#include <filesystem>

export module Zen;

import HookManager;
import SigManager;
import Module;
import SoundPlayer;
import Utils;
import EntityListDebug;

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

        // Enable ANSI escape codes
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
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
            FreeAndUnbindConsole();
            ExitThread(0);
            return 0;
        }
    }

    inline void Initialize(HMODULE hModule)
    {
        auto tStart = std::chrono::steady_clock::now();
        Module = hModule;
        Detail::AllocateAndBindConsole();
        SetConsoleTitleW(L"Zen Client");
        
        std::cout.setf(std::ios::unitbuf);
        std::cerr.setf(std::ios::unitbuf);

        // Zen ASCII Logo
        std::cout << "\033[38;5;141m" << R"(
  ______              
 |___  /              
    / / ___ _ __      
   / / / _ \ '_ \     
  / /_|  __/ | | |    
 /_____\___|_| |_|    
)" << "\033[0m" << std::endl;

        Utils::Log::Info("Initializing Zen architecture...");

        SigManager::initialize();
        
        const auto& sigs = SigManager::getSigs();
        std::size_t missing = 0;
        for (const auto& kv : sigs) {
            if (kv.second == 0) ++missing;
        }

        if (missing > 0) {
            Utils::Log::Error("Signature mismatch detected. Some features may be disabled.");
            for (const auto& kv : sigs) {
                if (kv.second == 0) std::cout << "  \033[38;5;197m-\033[0m " << kv.first << std::endl;
            }
        } else {
            Utils::Log::Success("Signatures resolved successfully.");
        }

        HookRegistry::InitializeAll();
        auto& hm = GetHookManager();
        hm.enableAll();
        
        GetSoundPlayer().initialize();

        auto tEnd = std::chrono::steady_clock::now();
        auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart).count();

        Utils::Log::Success("Zen Client is ready. Welcome back. (" + std::to_string(totalMs) + "ms)");
        std::cout << std::endl;
    }

    inline void Eject()
    {
        HANDLE h = CreateThread(nullptr, 0, Detail::EjectWorker, nullptr, 0, nullptr);
        if (h) CloseHandle(h);
    }
}
