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
        // Feed to ImGui: assume key codes map 1:1 for A-Z and digits; user can refine later
        if (key >= 0 && key <= 512)
            ImGui_ImplBigRat::AddKeyEvent(key, state);
        Modules::HandleKeyEvent(key, state);
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


