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
        {
            std::string base = Utils::GetRoamingPath();
            if (!base.empty())
            {
                std::filesystem::path wav = std::filesystem::path(base) / "sound.wav";
                if (GetSoundPlayer().loadWavFile("roam", wav.wstring()))
                {
                    GetSoundPlayer().play("roam", 0.9f, false);
                }
                else
                {
                    std::cout << "[Sound] Failed to load: " << wav.string() << ", using beep" << std::endl;
                    //GetSoundPlayer().playOneShotSine(880.0f, 0.12f, 0.35f);
                }
            }
            else
            {
                //GetSoundPlayer().playOneShotSine(880.0f, 0.12f, 0.35f);
            }
        }
        if (auto* clickGui = Modules::Find("ClickGui")) clickGui->setEnabled(true);
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
}
