module;
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <iostream>

export module Zen;

import ModuleManager;

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
        std::cout << "Zen Client has been initialized!" << std::endl;
    }

    inline void Initialize()
    {
        Detail::AllocateAndBindConsole();
        SetConsoleTitleW(L"Zen Client");
        Console();
    }
}


