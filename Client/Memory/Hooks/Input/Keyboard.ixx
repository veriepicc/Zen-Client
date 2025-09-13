module;

export module KeyboardHook;

import HookManager;
import SigManager;
import imgui_impl_bigrat;
import Module;

namespace Hooks::Input::Keyboard
{
    using KeyboardFeedFn = void(*)(int, bool);

    struct State
    {
        static inline KeyboardFeedFn original = nullptr;
    };

    static void __stdcall Detour(int key, bool state)
    {
        // Hook disabled - this function is never called
    }

export namespace Hooks::Input::Keyboard
{
    inline bool Install()
    {
        // Completely disabled - no keyboard hook installed
        return false;
        /*
        void* target = SigManager::Keyboard_feed;
        if (!target)
        {
            return false;
        }
        auto& hm = GetHookManager();
        return hm.hook<KeyboardFeedFn>(target, Detour, &State::original);
        */
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Input::Keyboard::Install };
}