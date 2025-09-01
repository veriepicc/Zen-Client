module;

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
            return false;
        }
        auto& hm = GetHookManager();
        return hm.hook<KeyboardFeedFn>(target, Detour, &State::original);
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Input::Keyboard::Install };


