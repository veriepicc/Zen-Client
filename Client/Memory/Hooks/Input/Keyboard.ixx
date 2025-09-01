module;
#include <iostream>

export module KeyboardHook;

import HookManager;
import SigManager;

namespace Hooks::Input::Keyboard
{
    using KeyboardFeedFn = void(*)(int, bool);

    struct State
    {
        static inline KeyboardFeedFn original = nullptr;
    };

    static void __stdcall Detour(int key, bool state)
    {
        std::cout << "[Keyboard] key=" << key << " state=" << (state ? 1 : 0) << std::endl;
        if (State::original) State::original(key, state);
    }
}

export namespace Hooks::Input::Keyboard
{
    inline bool Install()
    {
        void* target = SigManager::Keyboard_feed;
        if (!target)
        {
            std::cout << "[KeyboardHook] signature not resolved" << std::endl;
            return false;
        }
        auto& hm = GetHookManager();
        return hm.hook<KeyboardFeedFn>(target, Detour, &State::original);
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Input::Keyboard::Install };


