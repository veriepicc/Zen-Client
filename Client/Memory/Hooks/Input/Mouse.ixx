module;
#include <iostream>

export module MouseHook;

import HookManager;
import SigManager;

namespace Hooks::Input::Mouse
{
    using MouseFeedFn = void(*)(void*, char, char, short, short, short, short, char);

    struct State
    {
        static inline MouseFeedFn original = nullptr;
    };

    static void __stdcall Detour(void* mouseDevice, char button, char action, short mouseX, short mouseY,
                                 short movementX, short movementY, char a8)
    {
        std::cout << "[Mouse] btn=" << (int)button << " act=" << (int)action
                  << " x=" << mouseX << " y=" << mouseY << " dx=" << movementX << " dy=" << movementY << std::endl;
        if (State::original) State::original(mouseDevice, button, action, mouseX, mouseY, movementX, movementY, a8);
    }
}

export namespace Hooks::Input::Mouse
{
    inline bool Install()
    {
        void* target = SigManager::MouseDevice_feed;
        if (!target)
        {
            std::cout << "[MouseHook] signature not resolved" << std::endl;
            return false;
        }
        auto& hm = GetHookManager();
        return hm.hook<MouseFeedFn>(target, Detour, &State::original);
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Input::Mouse::Install };


