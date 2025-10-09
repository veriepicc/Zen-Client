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
        // Force console allocation first
        AllocConsole();
        
        // Try to attach to parent, but don't fail if it doesn't work
        AttachConsole(ATTACH_PARENT_PROCESS);

        // Set console properties
        SetConsoleTitleA("Zen Client - Education Edition Debug Console");
        
        // Get console handle and set properties
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(hConsole, &csbi);
            COORD newSize = { 120, 30 };
            SetConsoleScreenBufferSize(hConsole, newSize);
        }

        FILE* fptr = nullptr;
        freopen_s(&fptr, "CONOUT$", "w", stdout);
        freopen_s(&fptr, "CONOUT$", "w", stderr);
        freopen_s(&fptr, "CONIN$", "r", stdin);
        
        // Force flush and test output immediately
        std::cout << "[Console] Console initialized successfully!" << std::endl;
        std::cout << "[Console] This should be visible in the console window!" << std::endl;
        std::cout.flush();
        fflush(stdout);
        
        // Force console window to front
        HWND hwnd = GetConsoleWindow();
        if (hwnd) {
            ShowWindow(hwnd, SW_SHOW);
            SetForegroundWindow(hwnd);
        }
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
        
        FILE* debugFile = nullptr;
        std::string debugPath = Utils::GetRoamingPath() + "\\zen_debug.txt";
        fopen_s(&debugFile, debugPath.c_str(), "w");
        if (debugFile) {
            fprintf(debugFile, "[Zen] DLL Initialize called\n");
            fflush(debugFile);
        }
        
        // Immediate debug output before console setup
        OutputDebugStringA("[Zen] DLL Initialize called\n");
        
        Detail::AllocateAndBindConsole();
        SetConsoleTitleW(L"Zen Client - Education Edition");
        std::cout.setf(std::ios::unitbuf);
        std::cerr.setf(std::ios::unitbuf);
        
        FILE* logFile = nullptr;
        std::string logPath = Utils::GetRoamingPath() + "\\zen_log.txt";
        fopen_s(&logFile, logPath.c_str(), "w");
        
        if (logFile) {
            fprintf(logFile, "Zen Client has been initialized!\n");
            fprintf(logFile, "Target: Minecraft Education Edition\n");
            fflush(logFile);
        }
        
        MessageBoxA(nullptr, "Zen Client DLL loaded successfully!\nCheck roaming state folder for zen_log.txt", "Zen Client", MB_OK);
        
        if (debugFile) {
            fprintf(debugFile, "[Zen] Console setup complete\n");
            fflush(debugFile);
        }
        
        OutputDebugStringA("[Zen] Console setup complete\n");
        auto tStart = std::chrono::high_resolution_clock::now();
        SigManager::initialize();
        {
            const auto& sigs = SigManager::getSigs();
            std::size_t resolved = 0, missing = 0;
            for (const auto& kv : sigs) {
                if (kv.second != 0) ++resolved; else ++missing;
            }
            
            if (logFile) {
                fprintf(logFile, "[Zen] Signatures: total=%zu resolved=%zu missing=%zu\n", 
                    sigs.size(), resolved, missing);
                fflush(logFile);
            }
            
            // Log all signatures
            if (logFile) {
                fprintf(logFile, "[Sig] entries=%zu\n", sigs.size());
                for (const auto& kv : sigs) {
                    fprintf(logFile, "  - %s = 0x%llx\n", kv.first.c_str(), kv.second);
                }
                fflush(logFile);
            }
            
            if (missing > 0 && logFile) {
                fprintf(logFile, "[Zen] Missing signatures:\n");
                for (const auto& kv : sigs) {
                    if (kv.second == 0) fprintf(logFile, "  - %s\n", kv.first.c_str());
                }
                fflush(logFile);
            }
        }
        auto tAfterSigs = std::chrono::high_resolution_clock::now();

        HookRegistry::InitializeAll();
        auto tAfterHooks = std::chrono::high_resolution_clock::now();
        auto& hm = GetHookManager();
        hm.enableAll();
        
        // Log hook status to file
        if (logFile) {
            fprintf(logFile, "[Hooks] Status: %zu active hooks\n", hm.targetCount());
            fflush(logFile);
        }
        
        GetSoundPlayer().initialize();

        auto tAfterEnable = std::chrono::high_resolution_clock::now();

        auto sigMs = std::chrono::duration_cast<std::chrono::milliseconds>(tAfterSigs - tStart).count();
        auto hooksMs = std::chrono::duration_cast<std::chrono::milliseconds>(tAfterHooks - tAfterSigs).count();
        auto enableMs = std::chrono::duration_cast<std::chrono::milliseconds>(tAfterEnable - tAfterHooks).count();
        auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(tAfterEnable - tStart).count();
        
        if (logFile) {
            fprintf(logFile, "[Zen] init times: sigs=%lldms, hooks=%lldms, enable=%lldms, total=%lldms\n",
                sigMs, hooksMs, enableMs, totalMs);
            fprintf(logFile, "[Zen] Initialization complete\n");
            fclose(logFile);
        }
            
        if (debugFile) {
            fprintf(debugFile, "[Zen] Initialization complete\n");
            fclose(debugFile);
        }
    }

    inline void Eject()
    {
        HANDLE h = CreateThread(nullptr, 0, Detail::EjectWorker, nullptr, 0, nullptr);
        if (h) CloseHandle(h);
    }
}
